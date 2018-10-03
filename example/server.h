
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

#ifndef SERVER_H
#define SERVER_H

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * This module implements the functions tgetc(), tputc() and tputs()
 * that are needed for vt100.
 */

/** Create a TCP server at port 2277.
  * For client create a thread.
  * @param client Callback function for each client.
  *               The parameter passed has to be used with tgetc(), tputc() and tputs().
  *               With this functions data can be transfered using the socket.
  * @return  On success, zero.
  *          On error, non-zero*/
int server( void(*client)(void*) );

/** Get the client ID. */
int clientid( void* p );

#ifdef	__cplusplus
}
#endif

#endif	/* SERVER_H */

