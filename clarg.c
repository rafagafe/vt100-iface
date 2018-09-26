
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

#include <ctype.h>
#include <stdio.h>

/**  Search the next non-space or null character.
  * @param p Pointer to first character.
  * @return  Pointer to the found character. */
static char* skipspace( char* p ) {
    for( ; '\0' != *p && isspace( *p ); ++p );
    return p;
}

/** Search the next space or null character.
  * @param p Pointer to first character.
  * @return  Pointer to the found character. */
static char* skipnonspace( char* p ) {
    for( ; '\0' != *p && !isspace( *p ); ++p );
    return p;
}

static int getesc( int ch ) {
    static struct { char ch; char code; } const lut [] = {
        { '\"', '\"' }, { '\\', '\\' }, { '/',  '/'  }, { 'b',  '\b' },
        { 'f',  '\f' }, { 'n',  '\n' }, { 'r',  '\r' }, { 't',  '\t' },
        { 'a',  '\a' }, { 'e', '\033' }
    };
    for( int i = 0; i < sizeof lut / sizeof *lut; ++i )
        if ( lut[i].ch == ch )
            return lut[i].code;
    return '\0';
}

/** Parse an argument that is enclosed in quotation marks
  * @param str First character.
  * @return Last character.*/
static char* parsearg( char* str ) {
    char* head = str;
    char* tail = str;
    for( ; (unsigned)' ' <= *head && '\"' != *head; ++head, ++tail ) {
        if ( '\\' != *head )
            *tail = *head;
        else {
            int const esc = getesc( *++head );
            if ( '\0' != esc )
                *tail = esc;
            else {
                *tail = '\\';
                *++tail = *head;
            }
        }
    }
    head += '\"' == *head;
    *tail = '\0';
    return head;
}

/* Parse Command Line ARGuments */
int clarg( char** argv, int max, char* line ) {
    for( int i = 0; i < max; ++i ) {
        line = skipspace( line );
        if( '\0' == *line )
            return i;
        if( '\"' == *line ) {
            argv[i] = ++line;
            line = parsearg( line );
            if( '\0' == *line )
                 return i + 1;
        }
        else {
            argv[i] = line;
            line = skipnonspace( line );
            if( '\0' == *line )
                return i + 1;
            *line = '\0';
            ++line;
        }
    }
    return -1;
}
