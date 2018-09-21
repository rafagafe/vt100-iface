
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

#ifndef HISTORY_H
#define	HISTORY_H

#ifdef	__cplusplus
extern "C" {
#endif

/** History configuration. */
struct historycfg {
    void* lines;    /**< Memory block for history.  */
    short linelen;  /**< Length of lines in history. */
    short numlines; /**< Lines capacity in history.  */
};

/** It handles a history. */
struct history {
    struct historycfg const* cfg;
    short oldest; /**< Index of the oldest entry.         */
    short newest; /**< Index if the newest entry.         */
    short pos;    /**< Index of the last consulted entry. */
};

/** Initialize an instance of a history.
  * @param hist A valid history handle.
  * @param cfg History configuration. */
void history_init( struct history* hist, struct historycfg const* cfg );

/** Erase history.
  * @param hist A valid history handle. */
void history_erase( struct history* hist );

/** Add a new line to a history.
  * @param hist A valid history handle.
  * @param line Null-terminated string with the new line to be added. */
void history_line( struct history* hist, char const* line );

/** It searches in a history the previous matching entry.
  * @param hist A valid history handle.
  * @param text The key string for searching.
  * @param len  The lengthy of key string.
  * @return  The next matching entry or null if not found. */
char const* history_backward( struct history* hist, char const* text, int len );

/** It searches in a history the next matching entry.
  * @param hist A valid history handle.
  * @param text The key string for searching.
  * @param len  The lengthy of key string.
  * @return  The next matching entry or null if not found. */
char const* history_forward( struct history* hist, char const* text, int len );


/* Example:
 *
 *   enum {
 *       numlines = 64,
 *       linelen  = 80
 *   };
 *
 *   char memory[numlines][linelen];
 *
 *   static struct historycfg const cfg = {
 *       .lines    = memory,
 *       .linelen  = linelen,
 *       .numlines = numlines
 *   };
 *
 *   struct history his;
 *
 *   history_init( &his, cfg );
 *
 */

#ifdef	__cplusplus
}
#endif

#endif	/* HISTORY_H */

