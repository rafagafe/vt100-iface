
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

#define WINVER       0x0A00
#define _WIN32_WINNT 0x0A00

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <ctype.h>
#include "server.h"
#include "../vt100.h"

#define DEFAULT_PORT "2277"

struct client {
    SOCKET socket;
    void(*clientask)(void*);
    int id;
};

int tputc( int c, void* p ) {
    struct client* client = (struct client*)p;
    char const data = c;
    int txlen = send( client->socket, &data, sizeof data, 0 );
    if ( SOCKET_ERROR == txlen ) {
        fprintf( stderr, "%s%d\n", "send failed with error: ", WSAGetLastError() );
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
    int const rxlen = recv( client->socket, (char*)&data, sizeof data, 0 );
    if( 0 == rxlen )
        return -1;
    if( 0 > rxlen ) {
        fprintf( stderr, "%s%d\n", "recv failed with error: ", WSAGetLastError() );
        closesocket( client->socket );
        return -1;
    }
    int const verbose = 0;
    if( verbose ) {
        if( isprint( data ) ) printf( "%c\n", data );
        else  printf( "<%3d>\n", (int)data );
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
    closesocket( client->socket );
    free( client );
    return NULL;
}

int server( void(*clientask)(void*) ) {

    // Initialize Winsock
    WSADATA wsaData;
    int err = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    if ( err ) {
        fprintf( stderr, "%s%d\n", "WSAStartup failed with error: ", err );
        return 1;
    }

    struct addrinfo hints;
    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = AI_PASSIVE;

    // Resolve the server address and port
    struct addrinfo* addrinfo = NULL;
    err = getaddrinfo( NULL, DEFAULT_PORT, &hints, &addrinfo );
    if ( err ) {
        fprintf( stderr, "%s%d\n", "getaddrinfo failed with error: ", err );
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    SOCKET ListenSocket = socket( addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol );
    if ( INVALID_SOCKET == ListenSocket ) {
        fprintf( stderr, "%s%d\n", "socket failed with error: ", WSAGetLastError() );
        freeaddrinfo( addrinfo );
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    err = bind( ListenSocket, addrinfo->ai_addr, (int)addrinfo->ai_addrlen);
    if ( SOCKET_ERROR == err  ) {
        fprintf( stderr, "%s%d\n", "bind failed with error: ", WSAGetLastError() );
        freeaddrinfo( addrinfo );
        closesocket( ListenSocket );
        WSACleanup();
        return 1;
    }

    freeaddrinfo( addrinfo );

    err = listen( ListenSocket, SOMAXCONN );
    if ( SOCKET_ERROR == err  ) {
        fprintf( stderr, "%s%d\n", "listen failed with error: ", WSAGetLastError() );
        closesocket( ListenSocket );
        WSACleanup();
        return 1;
    }

    for( int id = 0; id < 100; ++id ) {
        // Accept a client socket
        SOCKET ClientSocket = accept( ListenSocket, NULL, NULL );
        if ( INVALID_SOCKET == ClientSocket  ) {
            fprintf( stderr, "%s%d\n", "accept failed with error: ", WSAGetLastError() );
            closesocket( ListenSocket );
            WSACleanup();
            return 1;
        }
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
    closesocket( ListenSocket );

    WSACleanup();
    return 0;
}
