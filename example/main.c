
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
#include "server.h"
#include <stdlib.h>
#include <time.h>
#include "../vt100-tgetc.h"

void prompt( void* p ) {
    tputs( "\033[32m", p );
    time_t now = time( NULL );
    struct tm *mytime = localtime( &now );
    char buffer[32];
    strftime( buffer, sizeof buffer, "%X", mytime );
    tputs( buffer, p );
    tputs( " >\033[0m ", p );
}

static void client( void* p ) {

    /* Configure the hints: */
    static char const* const names[] = {
        "clear"   ,
        "help"    ,
        "history" ,
        "exit"    ,
    };

    static struct hints const hints = {
        .str  = names,
        .qty  = sizeof names / sizeof *names
    };

    /* Configure the history: */
    enum {
        linelen  =  80,
        numlines = 200
    };
    char lines[ numlines ][ linelen ];
    struct historycfg const cfg = {
        .lines    = lines,
        .linelen  = linelen,
        .numlines = numlines
    };
    struct history his;
    history_init( &his, &cfg );

    char buff[ linelen ];
    struct vt100 const vt100 = {
        .p     = p,
        .max   = sizeof buff,
        .line  = buff,
        .hist   = &his,
        .hints = &hints,
    };

    tputs( "\ec\e[2J", p );

    for(;;) {

        prompt( p );

        int len = vt100_getline( &vt100 );
        if( 0 == len )
            continue;
        if( 0 > len ) {
            fprintf( stderr, "%s%d\n", "Error", len );
            break;
        }
        printf( "%3d) %s\n", clientid( p ), vt100.line );

        if( 0 == strcmp( "exit", vt100.line ) )
            break;

        if ( 0 == strcmp( "clear", vt100.line ) )
            tputs( "\ec\e[2J", p );

        else if ( 0 == strcmp( "help", vt100.line ) )
            for( int i = 0; i < hints.qty; ++i )
                tputs( hints.str[i], p ), tputs( "\r\n", p );

        else if ( 0 == strcmp( "history", vt100.line ) )
            history_init( &his, &cfg );
    }
}

int main( void ) {
    server( client );
    return 0;
}