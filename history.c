
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

#include <string.h>
#include "history.h"

/* Initialize an instance of a history. */
void history_init( struct history* hist, struct historycfg const* cfg ) {
    hist->cfg = cfg;
    history_erase( hist );
    typedef char(*array_t)[cfg->numlines][cfg->linelen];
    array_t const lines = (array_t)cfg->lines;
    for( int i = 0; i < cfg->numlines; ++i )
        (*lines)[i][0] = '\0';
}

/*  Erase history. */
void history_erase( struct history* hist ) {
    typedef char(*array_t)[hist->cfg->numlines][hist->cfg->linelen];
    array_t const lines = (array_t)hist->cfg->lines;
    for( int i = 0; i < hist->cfg->numlines; ++i )
        (*lines)[i][0] = '\0';
    hist->oldest =  0;
    hist->newest = -1;
    hist->pos    = -1;
}

/*  Add a new line to a history. */
void history_line( struct history* hist, char const* line ) {
    typedef char(*array_t)[hist->cfg->numlines][hist->cfg->linelen];
    array_t const lines = (array_t)hist->cfg->lines;
    if( 0 == strcmp( line, (*lines)[hist->pos] ) )
        return;
    int empty = -1 == hist->newest;
    if( empty ) {
        hist->oldest = 0;
        hist->newest = 0;
    }
    else {
        if( ++hist->newest == hist->cfg->numlines )
            hist->newest = 0;
        if( hist->newest == hist->oldest )
            if( ++hist->oldest == hist->cfg->numlines )
                hist->oldest = 0;
    }
    strncpy( (*lines)[ hist->newest ], line, sizeof **lines );
    hist->pos = -1;
}

/** Get the previous history entry.
  * @param hist A valid history handle.
  * @return The previous history entry or null it the limit is got. */
static char const* backward( struct history* hist ) {
    if( -1 == hist->newest )
        return NULL;
    typedef char(*array_t)[hist->cfg->numlines][hist->cfg->linelen];
    array_t const lines = (array_t)hist->cfg->lines;
    if( 0 > hist->pos )
        hist->pos = hist->newest;
    else if( hist->pos != hist->oldest )
        if( --hist->pos < 0 )
            hist->pos = hist->cfg->numlines - 1;
    return (*lines)[hist->pos];
}

/* It searches in a history the previous matching entry. */
char const* history_backward( struct history* hist, char const* text, int len ) {
    for(;;) {
        int const limit = hist->pos == hist->oldest;
        char const* rslt = backward( hist );
        if ( NULL == rslt )
            return NULL;
        if ( 0 == memcmp( text, rslt, len ) )
            return rslt;
        if ( limit )
            return NULL;
    }
}

/** Get the next history entry.
  * @param hist A valid history handle.
  * @return The next history entry or null it the limit is got. */
static char const* forward( struct history* hist ) {
    if( -1 == hist->newest )
        return NULL;
    if( hist->pos == hist->newest )
        return NULL;
    typedef char(*array_t)[hist->cfg->numlines][hist->cfg->linelen];
    array_t const lines = (array_t)hist->cfg->lines;
    if( 0 > hist->pos )
        hist->pos = hist->newest;
    else if( ++hist->pos == hist->cfg->numlines )
        hist->pos = 0;
    return (*lines)[hist->pos];
}

/* It searches in a history the next matching entry. */
char const* history_forward( struct history* hist, char const* text, int len ) {
    for(;;) {
        char const* rslt = forward( hist );
        if ( NULL == rslt )
            return NULL;
        if ( 0 == memcmp( text, rslt, len ) )
            return rslt;
    }
}
