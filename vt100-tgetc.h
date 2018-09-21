
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

#ifndef VT100_UTIL_H
#define	VT100_UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "vt100.h"

/** Callback function. It has to be defined by the user.
  * Get blocked until get a character from a terminal.
  * @param p A valid instance of a terminal.
  * @retval On success, the character read is returned (promoted to an int value).
  * @retval On error, a negative value. */
int tgetc( void* p );

/** Get blocked until capture a line.
  * @param vt100 A vt100 configure.
  * @retval On success, a non negative with the length of the line captured.
  * @retval On error, a negative value returned by tgetc(). */
int vt100_getline( struct vt100 const* vt100 );

#ifdef	__cplusplus
}
#endif

#endif	/* VT100_UTIL_H */

