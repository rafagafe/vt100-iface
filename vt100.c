
/*
  Licensed under the MIT License <http://opensource.org/licenses/MIT>.
  SPDX-License-Identifier: MIT
  Copyright (c) 2018 Rafa Garcia <rafagarcia77@gmail.com>.
  Permission is hereby  granted, free of charge, to any  person obtaining a copy
  of this software and associated  documentation files (the "Software"), to deal
  in the Software  without restriction, including without  limitation the rights
  to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
  copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
  IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
  FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
  AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
  LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "vt100.h"
#include "terminal-io.h"

/** Look for the next word start in array of characters.
  * @param str Pointer to the first character in the array.
  * @param pos Actual position.
  * @param end Size if the array.
  * @return  The position of the next word start. */
static int nextword( char const* str, int pos, int end ) {
    for( ; pos < end && !isspace( str[pos] ); ++pos );
    for( ; pos < end &&  isspace( str[pos] ); ++pos );
    return pos;
}

/** Look for the previous word start in array of characters.
  * @param str Pointer to the first character in the array.
  * @param pos Actual position.
  * @return  The position of the previous word start. */
static int prevword( char const* str, int pos ) {
    for( ; pos > 0 &&  isspace( str[pos-1] ); --pos );
    for( ; pos > 0 && !isspace( str[pos-1] ); --pos );
    return pos;
}

/** It searches in a hints set the next matching hint.
  * @param hints The hints set.
  * @param str   String to search.
  * @param len   Length of the string.
  * @param h     Hint index where start to search.
  * @return      The index of the hint found or -1 if there are not matches. */
static int nexthint( struct hints const* hints, char const* str, int len, int h ) {
    if( 0 == len )
        return hints->qty == h + 1 ? 0 : h + 1;
    for( int i = h + 1; i < hints->qty; ++i )
        if ( 0 == memcmp( str, hints->str[i], len ) )
            return i;
    for( int i = 0; i < h; ++i )
        if ( 0 == memcmp( str, hints->str[i], len ) )
            return i;
    return -1;
}

/** It searches in a hints set the previous matching hint.
  * @param hints The hints set.
  * @param str   String to search.
  * @param len   Length of the string.
  * @param h     Hint index where start to search.
  * @return      The index of the hint found or -1 if there are not matches. */
static int prevhint( struct hints const* hints, char const* str, int len, int h ) {
    if( 0 == len )
        return 0 == h ? hints->qty - 1 : h - 1;
    for( int i = h - 1; i >= 0; --i )
        if ( 0 == memcmp( str, hints->str[i], len ) )
            return i;
    for( int i = hints->qty - 1; i > h; --i )
        if ( 0 == memcmp( str, hints->str[i], len ) )
            return i;
    return -1;
}

/** Move the cursor of the vt100 terminal n columns to the right or to the left.
  * @param colunms Number of columns. Positive means to the right.
  * @param p Parameter for printing */
static void movecursor( int colunms, void* p  ) {
    switch( colunms ) {
        case 0:
            break;
        case -1:
            tputs( "\033[D", p );
            break;
        case 1:
            tputs( "\033[C", p );
            break;
        default: {
            char buff[8];
            sprintf( buff, "\033[%d%c", abs( colunms ), 0 > colunms ? 'D' : 'C' );
            tputs( buff, p );
            break;
        }
    }
}

/** Erase in the vt100 terminal from the cursor until the end of line.
  * @param p Parameter for printing */
static void eraseend( void* p ) {
    tputs( "\033[K", p );
}

/** Move the cursor n columns forward.
  * In st->param is the number of columns. 
  * @param st State of line capture. */
static void cursorforward( struct vt100state* st ) {
    int const param   = 0 == st->param ? 1 : st->param;
    int const toend   = st->len - st->cur;
    int const columns = toend < param ? toend : param;
    if( echo_off != st->echo )    
        movecursor( columns, st->cfg->p );
    st->cur += columns;
}

/** Move the cursor n columns backward.
  * In st->param is the number of columns.
  * @param st State of line capture. */
static void cursorbackward( struct vt100state* st ) {
    int const param   = 0 == st->param ? 1 : st->param;
    int const tobegin = st->cur;
    int const columns = tobegin < param ? tobegin : param;
    if( echo_off != st->echo )
        movecursor( -columns, st->cfg->p );
    st->cur -= columns;
}

/** Add a character to the line. It is inserted in the position of the cursor.
  * @param st State of line capture.
  * @param c  Character value to be inserted. */
static void addchar( struct vt100state* st, int c ) {
    if( st->len + 2 >= st->cfg->max )
        return;
    if( st->cur < st->len )
        eraseend( st->cfg->p );
    for( int i = st->cur; i <= st->len; ++i ) {
        if( echo_off != st->echo )
            tputc( echo_pass == st->echo ? '*' : c, st->cfg->p );
        int tmp = st->cfg->line[i];
        st->cfg->line[i] = c;
        c = tmp;
    }
    ++st->cur;
    ++st->len;
    if( echo_off != st->echo )
        movecursor( st->cur - st->len, st->cfg->p );
}

/** Remove the character before the cursor
  * @param st State of line capture. */
static void removechar( struct vt100state* st ) {
    if( 0 == st->cur )
        return;
    if( echo_off != st->echo ) {
        tputc( '\b', st->cfg->p );
        eraseend( st->cfg->p );
    }
    for( int i = st->cur; i < st->len; ++i ) {
        st->cfg->line[i-1] = st->cfg->line[i];
        if( echo_off != st->echo )
            tputc( echo_pass == st->echo ? '*' : st->cfg->line[i], st->cfg->p );
    }
    --st->cur;
    --st->len;
    if( echo_off != st->echo )
        movecursor( st->cur - st->len, st->cfg->p );
}

/** Set the cursor to the first character of the line.
  * @param st State of line capture. */
static void home( struct vt100state* st ) {
    if( echo_off != st->echo )
        movecursor( -st->cur, st->cfg->p );
    st->cur = 0;
}

/** Set the cursor to the end of the line.
  * @param st State of line capture. */
static void end( struct vt100state* st ) {
    if( echo_off != st->echo )
        movecursor( st->len - st->cur, st->cfg->p );
    st->cur = st->len;
}

/** Remove the character after the cursor
  * @param st State of line capture. */
static void delete( struct vt100state* st ) {
    if( st->cur == st->len )
        return;
    --st->len;
    if( echo_off != st->echo )
        eraseend( st->cfg->p );
    for( int i = st->cur; i < st->len; ++i ) {
        st->cfg->line[i] = st->cfg->line[i+1];
        if( echo_off != st->echo )
            tputc( echo_pass == st->echo ? '*' : st->cfg->line[i], st->cfg->p );
    }
    if( echo_off != st->echo )
        movecursor( st->cur - st->len, st->cfg->p );
}

/** Move the cursor to the next word start.
  * @param st State of line capture. */
static void movenextword( struct vt100state* st ) {
    int const pos = nextword( st->cfg->line, st->cur, st->len );
    movecursor( pos - st->cur, st->cfg->p );
    st->cur = pos;
}

/** Move the cursor to the previous word start.
  * @param st State of line capture. */
static void moveprevword( struct vt100state* st ) {
    int const pos = prevword( st->cfg->line, st->cur );
    movecursor( pos - st->cur, st->cfg->p );
    st->cur = pos;
}

/** Erase a complete word pointed by the cursor.
  * @param st State of line capture. */
void eraseword( struct vt100state* st ) {
    if( echo_on != st->echo )
        return;
    int const first = prevword( st->cfg->line, st->cur );
    int const end   = nextword( st->cfg->line, first, st->len );
    movecursor( first - st->cur, st->cfg->p );
    int const oldlen = st->len;
    st->len = st->cur = first;
    eraseend( st->cfg->p );
    memmove( st->cfg->line + first, st->cfg->line + end, oldlen - end );
    int const wordlen = end - first;
    int const newlen = oldlen - wordlen;
    for( int i = first; i < newlen; ++i )
        addchar( st, st->cfg->line[i] );
    movecursor( first - newlen, st->cfg->p );
    st->cur = first;
}

/** Erase form the cursor until the end of line.
  * Concatenate a string.
  * Restore the cursor to the the position before call this function.
  * @param st State of line capture.
  * @param str String to be concatenated. */
static void refill( struct vt100state* st, char const* str ) {
    eraseend( st->cfg->p );
    if( NULL == str )
        return;
    int const pos = st->len = st->cur;
    while( '\0' != *str )
        addchar( st, *str++ );
    st->cur = pos;
    movecursor( st->cur - st->len, st->cfg->p );
}

/** Write in the line the next history entry that matches.
  * @param st State of line capture. */
static void nextentry( struct vt100state* st ) {
    if( echo_on != st->echo || NULL == st->cfg->hist )
        return;
    char const* entry = history_forward( st->cfg->hist, st->cfg->line, st->cur );
    refill( st, NULL == entry ? "" : entry + st->cur );
}

/** Write in the line the previous history entry that matches.
  * @param st State of line capture. */
static void preventry( struct vt100state* st ) {
    if( echo_on != st->echo || NULL == st->cfg->hist )
        return;
    char const* entry = history_backward( st->cfg->hist, st->cfg->line, st->cur );
    if( NULL != entry )
        refill( st, entry + st->cur );
}

/** Write in the line the next hints that matches.
  * @param st State of line capture.
  * @param forward non-zero forward or zero backward. */
static void hint( struct vt100state* st, int forward ) {
    if( echo_on != st->echo || NULL == st->cfg->hints )
        return;
    int const h = ( forward ? nexthint : prevhint )( st->cfg->hints, st->cfg->line, st->cur, st->h );
    if( 0 > h )
        return;
    st->h =  h;
    refill( st, st->cfg->hints->str[ h ] + st->cur );
}

/** Process a VT100 command: <ESC>[num~
  * @param st State of line capture. */
static void cursorctrl( struct vt100state* st ) {
    switch( st->param ) {
        case 1: home( st );   break; // Home key
        case 3: delete( st ); break; // Delete key
        case 4: end( st );    break; // End key
    }
}

/** Process a VT100 command: <ESC>[{num}{char}
  * @param st State of line capture.
  * @param c Character ID of VT100 command.
  * @param a Parameter of VT100 command. */
static void escapeSquareBracket( struct vt100state* st, int c ) {
    switch( c ) {
        case '~': cursorctrl( st );     break;
        case 'A': preventry( st );      break; // Arrow Up
        case 'B': nextentry( st );      break; // Arrow Down
        case 'C': cursorforward( st );  break; // Arrow Right
        case 'D': cursorbackward( st ); break; // Arrow Left
        case 'Z': hint( st, 0 );        break; // Shift + Tab
    }
}

/** Process a VT100 command: <ESC>O{char}
  * @param st State of line capture.
  * @param c Character ID of VT100 command.  */
static void escapeBigO( struct vt100state* st, int c ) {
    if( echo_on != st->echo )
        return;
    switch( c ) {
        case 'C': movenextword( st ); break; // Shift + Right Arrow
        case 'D': moveprevword( st ); break; // Shift + Left Arrow
    }
}

/** State codes of line capture. */
enum state {
    CHAR,    /**< Waiting for a print character or escape. */
    ESCAPE,  /**< Escape character is received.            */
    BRACKET, /**< Received <ESC>[                          */
    BIG_O,   /**< Received <ESC>O                          */
};

/*  Initialize a state of line capture. */
void vt100_init( struct vt100state* st, struct vt100 const* vt100, enum echo echo ) {
    *st = ( struct vt100state ) {
        .len   = 0,
        .cur   = 0,
        .h     = 0,
        .state = CHAR,
        .cfg   = vt100,
        .echo  = echo
    };
}

/* Discard all received and star a new line capture. */
void vt100_newline( struct vt100state* st ) {
    st->len = 0;
    st->cur = 0;
    st->h   = 0;
}

/** Control keys codes used. */
enum ctrlkey {
    BS  =   8, /**< Backspace */
    TAB =   9, /**< Tabulate  */
    ESC =  27, /**< Escape    */
    DEL = 127, /**< Delete    */
};

/* Process a received character in a line capture. */
int vt100_char( struct vt100state* st, int c ) {

    if( '\n' == c || '\r' == c ) {
        tputs( "\r\n", st->cfg->p );
        st->cfg->line[st->len] = '\0';
        if( NULL != st->cfg->hist )
            history_line( st->cfg->hist, st->cfg->line );
        int const len = st->len;
        vt100_newline( st );
        return len;
    }

    switch( st->state ) {

        case CHAR: {
            switch( c ) {
                case ESC: st->state = ESCAPE; break; // Escape
                case DEL: removechar( st );   break; // Backspace
                case TAB: hint( st, 1 );      break; // Tab
                case BS:  eraseword( st );    break; // Shift + backspace
                default:                    
                    if ( isprint( c ) )
                        addchar( st, c );
            }
            break;
        }
        
        case ESCAPE: {
            if ( '[' == c ) {
                st->state = BRACKET;
                st->param = 0;
            }
            else if ( 'O' == c )
                st->state = BIG_O;
            else
                st->state = CHAR;
            break;
        }

        case BRACKET: {
            if( ';' == c ) {
                st->param = 0;
                break;
            }
            if( !isdigit( c ) ) {
                escapeSquareBracket( st, c );
                st->state = CHAR;
                break;
            }
            st->param = 10 * st->param + c - '0';
            break;
        }

        case BIG_O: {
            escapeBigO( st, c );
            st->state = CHAR;
            break;
        }

    }
    return -1;
}
