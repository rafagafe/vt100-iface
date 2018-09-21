
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
#include <stdint.h>
#include <ctype.h>
#include "../vt100.h"
#include "../vt100-tgetc.h"


// ----------------------------------------------------- Test "framework": ---

#define done() return 0
#define fail() return __LINE__
static int checkqty = 0;
#define check( x ) do { ++checkqty; if (!(x)) fail(); } while ( 0 )

struct test {
    int(*func)(void);
    char const* name;
};

static int test_suit( struct test const* tests, int numtests ) {
    printf( "%s", "\n\nTests:\n" );
    int failed = 0;
    for( int i = 0; i < numtests; ++i ) {
        printf( " %02d%s%-25s ", i, ": ", tests[i].name );
        int linerr = tests[i].func();
        if ( 0 == linerr )
            printf( "%s", "OK\n" );
        else {
            printf( "%s%d\n", "Failed, line: ", linerr );
            ++failed;
        }
    }
    printf( "\n%s%d\n", "Total checks: ", checkqty );
    printf( "%s[ %d / %d ]\r\n\n\n", "Tests PASS: ", numtests - failed, numtests );
    return failed;
}


// ----------------------------------------------------- Helper functions: ---

static void printchar( int c ) {
    printf( isprint( c ) ? "%c" : "<%02x>", c );
}

static void printstr( char const* str ) {
    while( *str )
        printchar( *str++ );
}


// ----------------------------------------------------------------- Mock: ---

struct stream {
    char output[ 1024 ];
    int iout;
    char const* input;
    int iin;
};

int tputc( int c, void* p ) {
    struct stream* stream = (struct stream*)p;
    if( stream->iout == sizeof stream->output - 2 )
        return -1;
    stream->output[ stream->iout++ ] = c;
    stream->output[ stream->iout ]   = '\0';
    int const verbose = 0;
    if ( verbose )
        printchar( c );
    return 0;
}

int tputs( char const* str, void* p ) {
    while( '\0' != *str )
        tputc( *str++, p );
    return 0;
}

int tgetc( void* p ) {
    struct stream* stream = (struct stream*)p;
    int const rslt = stream->input[ stream->iin ];
    if( '\0' == rslt )
        return -1;
    ++stream->iin;
    return rslt;
}


// ----------------------------------------------------- Helper functions: ---

void pstream( struct stream const* stream ) {
    printf( "\n%s", "Input: " );
    printstr( stream->input );
    printf( "\n%s", "Output: " );
    printstr( stream->output );
    printf( "\n" );
}

static int processline( char const* input, char* line, int sizeline ) {
    struct stream stream;
    memset( &stream, 0, sizeof stream );
    stream.input = input;
    struct vt100 const vt100 = {
        .p     = &stream,
        .line  = line,
        .max   = sizeline,
        .hist  = NULL,
        .hints = NULL,
    };
    return vt100_getline( &vt100 );
}

#define HOME      "\033[1~" // Home key
#define END       "\033[4~" // End key
#define DEL       "\033[3~" // Delete hey
#define BS        "\177"    // Backspace key


// ----------------------------------------------------------- Unit tests: ---

static int basicInput( void ) {
    static char const input[] = "One Two Three\r\n";
    static char const expected[] = "One Two Three";
    char line[ 128 ];
    int const len = processline( input, line, sizeof line );
    check( len == sizeof expected - 1 );
    check( 0 == strcmp( line, expected ) );
    done();
}

static int arrows( void ) {
    static char const input[] = "One Three\033[100D\033[50C\033[6D\033[CTwo \n";
    static char const expected[] = "One Two Three";
    char line[ 128 ];
    int const len = processline( input, line, sizeof line );
    check( len == sizeof expected - 1 );
    check( 0 == strcmp( line, expected ) );
    done();
}

static int arrows2( void ) {
    static char const input[] = "One Three\033OD\033OD\033OCTwo \n";
    static char const expected[] = "One Two Three";
    char line[ 128 ];
    int const len = processline( input, line, sizeof line );
    check( len == sizeof expected - 1 );
    check( 0 == strcmp( line, expected ) );
    done();
}

static int backSpaceEnd( void ) {
    static char const input[] = "One Four" BS BS BS BS "Two Three\n";
    static char const expected[] = "One Two Three";
    char line[ 128 ];
    int const len = processline( input, line, sizeof line );
    check( len == sizeof expected - 1 );
    check( 0 == strcmp( line, expected ) );
    done();
}

static int backSpaceMiddle( void ) {
    static char const input[] = "One Two Four Three\033OD" BS BS BS BS BS "\n";
    static char const expected[] = "One Two Three";
    char line[ 128 ];
    int const len = processline( input, line, sizeof line );
    check( len == sizeof expected - 1 );
    check( 0 == strcmp( line, expected ) );
    done();
}

static int shiftBackSpaceEnd( void ) {
    static char const input[] = "One Four\010Two Three\n";
    static char const expected[] = "One Two Three";
    char line[ 128 ];
    int const len = processline( input, line, sizeof line );
    check( len == sizeof expected - 1 );
    check( 0 == strcmp( line, expected ) );
    done();
}

static int shiftBackSpaceMiddle( void ) {
    static char const input[] = "One Four Two Three\033[12D\010\n";
    static char const expected[] = "One Two Three";
    char line[ 128 ];
    int const len = processline( input, line, sizeof line );
    check( len == sizeof expected - 1 );
    check( 0 == strcmp( line, expected ) );
    done();
}

static int delete( void ) {
    static char const input[] = "One Four Three\033[10D" DEL DEL DEL DEL "Two\n";
    static char const expected[] = "One Two Three";
    char line[ 128 ];
    int const len = processline( input, line, sizeof line );
    check( len == sizeof expected - 1 );
    check( 0 == strcmp( line, expected ) );
    done();
}

static int home( void ) {
    static char const input[]    = "Two Three" HOME "One \n";
    static char const expected[] = "One Two Three";
    char line[ 128 ];
    int const len = processline( input, line, sizeof line );
    check( len == sizeof expected - 1 );
    check( 0 == strcmp( line, expected ) );
    done();
}

static int end( void ) {
    static char const input[]    = "Two" HOME "One " END " Three\n";
    static char const expected[] = "One Two Three";
    char line[ 128 ];
    int const len = processline( input, line, sizeof line );
    check( len == sizeof expected - 1 );
    check( 0 == strcmp( line, expected ) );
    done();
}

// --------------------------------------------------------- Execute tests: ---

int main( void ) {
    static struct test const tests[] = {
        { basicInput,           "Basic input"              },
        { arrows,               "Arrows char by char"      },
        { arrows2,              "Arrows word by word"      },
        { backSpaceEnd,         "Backspace at the end"     },
        { backSpaceMiddle,      "Backspace in the middle"  },
        { shiftBackSpaceEnd,    "Erase the last word"      },
        { shiftBackSpaceMiddle, "Erase a middle word"      },
        { delete,               "Delete key"               },
        { home,                 "Home key"                 },
        { end,                  "End key"                  },
    };
    return test_suit( tests, sizeof tests / sizeof *tests );
}