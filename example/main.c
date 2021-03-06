
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
#include "../vt100.h"
#include "../terminal-io.h"
#include "../clarg.h"

static int command( void* p, char** argv, int argc );
static int sum( void* p, char** argv, int argc );
static int mult( void* p, char** argv, int argc );
static int clear( void* p, char** argv, int argc );
static int login( void* p, char** argv, int argc ); 
static void printHistory( struct history const* hist, void* p );

static void client( void* p ) {

    /* Configure the hints: */
    static char const* const names[] = {
        "clear", "help", "exit", "command", "sum", "mult", "login", "history", "echo"
    };
    static struct hints const hints = {
        .str  = names,
        .qty  = sizeof names / sizeof *names
    };

    /* Configure the history: */
    enum {
        linelen  =  80,
        numlines =  32
    };
    struct historycfg const histcfg = {
        .lines    = malloc( numlines * linelen ),
        .linelen  = linelen,
        .numlines = numlines
    };
    struct history hist;
    history_init( &hist, &histcfg );

    /* Configure VT100: */
    char buff[ linelen ];
    struct vt100 const vt100 = {
        .p     = p,
        .max   = sizeof buff,
        .line  = buff,
        .hist  = &hist,
        .hints = &hints,
    };

    enum echo echo = echo_on;

    /* Clear screen: */
    tputs( "\033c\033[2J", p );

    for(;;) {

        /* Print prompt: */
        tputs( "\033[32m \\>\033[0m ", p );

        /* Get line: */
        int len = vt100_getline( &vt100, echo );
        if( 0 == len )
            continue;
        if( 0 > len ) {
            fprintf( stderr, "%s%d\n", "Error", len );
            break;
        }

        /* Parse arguments: */
        enum { maxargc = 10 };
        char* argv[ maxargc ];
        int const argc = clarg( argv, maxargc, vt100.line );
        if( 0 > argc )
            continue;

        /* Print arguments in local terminal: */
        printf( "%s%d\n", "Client: ", clientid( p ) );
        for( int i = 0; i < argc; ++i )
            printf( " [%d] %s\n", i, argv[i] );

        /* Process especial commands: */
        if( 0 == strcmp( "exit", *argv ) )
            break;

        if ( 0 == strcmp( "help", *argv ) ) {
            for( int i = 0; i < hints.qty; ++i )
                tputs( hints.str[i], p ), tputs( "\r\n", p );
            continue;
        }

        if ( 0 == strcmp( "history", *argv ) ) {
            printHistory( &hist, p );
            continue;
        }

        if ( 0 == strcmp( "echo", *argv ) ) {
             if ( 2 != argc )
                 continue;
             if ( 0 == strcmp( "on", argv[1] ) )
                 echo = echo_on;
             else if ( 0 == strcmp( "off", argv[1] ) )
                 echo = echo_off;
             continue;
        }

        /* Process ordinary commands: */
        static struct {
            char const* name;
            int(*func)(void*,char**,int);
        } const cmd [] = {
            { "sum",     sum     },
            { "mult",    mult    },
            { "command", command },
            { "clear",   clear   },
            { "login",   login   }
        };
        for( int i = 0; i < sizeof cmd / sizeof *cmd; ++i ) {
            if( 0 == strcmp( cmd[i].name, *argv ) ) {
                int rslt = cmd[i].func( p, argv, argc );
                printf( "%s%s%d\n", *argv, " return: ", rslt );
                break;
            }
        }
    }

    free( histcfg.lines );
}

int main( void ) {
    return server( client );
}

static int command( void* p, char** argv, int argc ) {
    if( 1 == argc ) {
        tputs( "This command just prints the arguments.\r\n", p );
        return 0;
    }
    for( int i = 1; i < argc; ++i )
        tputs( argv[i], p ), tputs( "\r\n", p );
    return 0;
}

static int sum( void* p, char** argv, int argc ) {
    if( 3 > argc || ( 1 < argc && 0 == strcmp( "help", argv[1] ) ) ) {
        tputs( "Usage: sum <number> <number> [<number> ...]\r\n", p );
        return 0;
    }
    long rslt = 0;
    for( int i = 1; i < argc; ++i ) {
        char* end;
        long const a = strtol( argv[i], &end, 10 );
        if( end == argv[i] || '\0' != *end ) {
            tputs( "Error: Bad argument\a\r\n", p );
            return -1;
        }
        rslt += a;
    }
    char buff[16];
    sprintf( buff, "%ld\r\n", rslt );
    tputs( buff, p );
    return 0;
}

static int mult( void* p, char** argv, int argc ) {
    if( 3 > argc || ( 1 < argc && 0 == strcmp( "help", argv[1] ) ) ) {
        tputs( "Usage: sum <number> <number> [<number> ...]\r\n", p );
        return 0;
    }
    long rslt = 1;
    for( int i = 1; i < argc; ++i ) {
        char* end;
        long const a = strtol( argv[i], &end, 10 );
        if( end == argv[i] || '\0' != *end ) {
            tputs( "Error: Bad argument\a\r\n", p );
            return -1;
        }
        rslt *= a;
    }
    char buff[16];
    sprintf( buff, "%ld\r\n", rslt );
    tputs( buff, p );
    return 0;
}

static int clear( void* p, char** argv, int argc ) {
    tputs( "\033c\033[2J", p );
    return 0;
}

static int login( void* p, char** argv, int argc ) {
    char buff[32];
    struct vt100 const vt100 = {
        .p     = p,
        .max   = sizeof buff,
        .line  = buff,
        .hist  = NULL,
        .hints = NULL,
    };
    static struct { char const* filed; enum echo echo; } const lut [] = {
        { "Name:       ", echo_on   },
        { "Password 1: ", echo_pass },
        { "Password 2: ", echo_off  }
    };
    for( int i = 0; i < sizeof lut / sizeof *lut; ++i ) {
        for(;;) {
            tputs( lut[i].filed, p );
            int len = vt100_getline( &vt100, lut[i].echo );
            if( 0 > len ) {
                printf( "%s%d\n", "Error: ", len );
                return len;
            }
            if( 4 < len )
                break;
            tputs( "It is too short\r\n", p );
        }
        printf( "%s%s\n", lut[i].filed, buff );        
    }   
    return 0;
}


static void printHistory( struct history const* hist, void* p ) {
    typedef char(*array_t)[hist->cfg->numlines][hist->cfg->linelen];
    array_t const lines = (array_t)hist->cfg->lines;
    if ( -1 == hist->newest )
        return;
    for( int i = hist->oldest;; i = hist->cfg->numlines - 1 == i ? 0 : i + 1 ) {
        puts( (*lines)[i] );
        tputs( (*lines)[i], p );
        tputs( "\r\n", p );
        if ( hist->newest == i )
            return;        
    }
}
