
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

#ifndef VT100_H
#define	VT100_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "history.h"

/** Callback function. It has to be defined by the user.
  * It prints a character to a terminal.
  * @param c Character to be sent.
  * @param p A valid instance of a terminal.
  * @return  On success, a non-negative value is returned.
  *          On error, -1 is returned  */
int tputc( int c, void* p );

/** Callback function. It has to be defined by the user.
  * It prints a null-terminated string to a terminal.
  * @param c Character to be sent.
  * @param p A valid instance of a terminal.
  * @return  On success, a non-negative value is returned.
  *          On error, -1 is returned  */
int tputs( char const* str, void* p );

/** Set of hints for a line capture. */
struct hints {
    /** Pointer to array of null-terminated strings with the hints. */
    char const* const* str;
    /** Number of hints. */
    int qty;
};

/** Configuration to capture a line from a vt100 terminal */
struct vt100 {
    /** A valid instance of a terminal. It will be passed to tputc() and tputs() */
    void* p;
    struct history* hist;       /**< History handle or null if there is not. */
    struct hints const* hints;  /**< Hints handle or null if there is not.   */
    char* line;                 /**< Destination buffer.                     */
    int max;                    /**< Size of line buffer.                    */
};

/** State of line capture. For internal use. */
struct vt100state {
    struct vt100 const* cfg;
    short state; /**< For state machine.           */
    short param; /**< Parameter of vt100 commands. */
    short len;   /**< Actual line len.             */
    short cur;   /**< Actual cursor possition.     */
    short h;     /**< Hint index.                  */
};

/** Initialize a state of line capture.
  * @param st    State of line capture.
  * @param vt100 Configuration of line capture. */
void vt100_init( struct vt100state* st, struct vt100 const* vt100 );

/** Process a received character in a line capture.
  * @param st State of line capture.
  * @param c  The received character.
  * @retval  non-negative: The line is just captured. The value is the length.
  * @retval negative:      Waiting for another character. */
int vt100_char( struct vt100state* st, int c );

/** Discard all received and star a new line capture.
  * @param st State of line capture. */
void vt100_newline( struct vt100state* st );


#ifdef	__cplusplus
}
#endif

#endif	/* VT100_H */

