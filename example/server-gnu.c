
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

#include <unistd.h> // ssize_t
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <pthread.h>
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include "server.h"
#include "../vt100.h"

#define DEFAULT_PORT 2277

struct client {
    int socket;
    void(*clientask)(void*);
    int id;
};

int tputc( int c, void* p ) {
    struct client* client = (struct client*)p;
    char const data = c;
    ssize_t txlen = write( client->socket, &data, sizeof data );
    if ( sizeof data != txlen ) {
        fprintf( stderr, "%s%zd\n", "send failed with error: ", txlen );
        return -1;
    }
    return 0;
}

int tputs( char const* str, void* p ) {
    while( '\0' != *str )
        tputc( *str++, p );
    return 0;
}

int tgetc( void* p ) {
    struct client* client = (struct client*)p;
    unsigned char data;
    ssize_t const rxlen = recv( client->socket, &data, sizeof data, 0 );
    if( 0 == rxlen )
        return -1;
    if( 0 > rxlen ) {
        fprintf( stderr, "%s%zd\n", "recv failed with error: ", rxlen );
        close( client->socket );
        return -1;
    }
    int const verbose = 0;
    if( verbose ) {
        if( isprint( data ) ) printf( "%c\n", data );
        else printf( "<%3d>\n", (int)data );
    }
    return data;
}

int clientid( void* p ) {
    struct client* client = (struct client*)p;
    return client->id;
}

static void* threadforclient( void* param ) {
    struct client* client = (struct client*)param;
    client->clientask( param );
    close( client->socket );
    free( client );
    return NULL;
}


int server( void(*clientask)(void*) ) {
    //Create socket
    int socket_desc = socket( AF_INET, SOCK_STREAM, 0 );
    if ( -1 == socket_desc ) {
        fputs( "Could not create socket", stderr );
        return -1;
    }
    puts( "Socket created" );

    //Prepare the sockaddr_in structure
    struct sockaddr_in server = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port        = htons( DEFAULT_PORT ),
    };

    //Bind
    int bindrslt = bind( socket_desc, (struct sockaddr*)&server, sizeof server );
    if( 0 > bindrslt ) {
        fprintf( stderr, "%s%d\n", "Bind failed. Error: ", bindrslt );
        return -1;
    }
    puts( "Bind done" );

    //Listen
    listen( socket_desc, 3 );

    for( int id = 0; id < 100; ++id ) {
    
        //Accept and incoming connection
        puts( "Waiting for incoming connections..." );
        int c = sizeof( struct sockaddr_in );

        // Accept connection from an incoming client
        struct sockaddr_in clientaddr;
        int ClientSocket = accept( socket_desc, (struct sockaddr*)&clientaddr, (socklen_t*)&c);
        if ( 0 > ClientSocket ) {
            fprintf( stderr, "%s%d\n", "Accept failed. Error: ", ClientSocket );
            return -1;
        }
        puts( "Connection accepted" );
        
        struct client* client = malloc( sizeof( struct client ) );
        *client = (struct client){ .socket = ClientSocket, .clientask = clientask, .id = id };
        pthread_t thread;
        int err = pthread_create( &thread, NULL, threadforclient, client );
        if( 0 != err ) {
            fprintf( stderr, "%s%d\n", "pthread_create failed with error: ", err );
            return 1;
        }
        fprintf( stdout, "%s %d\n", "New client", id );
    }

    // No longer need server socket
    close( socket_desc );

    return 0;
    
}


