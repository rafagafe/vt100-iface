# vt100-iface

[![Build Status](https://travis-ci.org/rafagafe/vt100-iface.svg?branch=master)](https://travis-ci.org/rafagafe/vt100-iface)

It is a C library to read text lines from a VT100 terminal.

You can edit the command line navigate it with the cursor using:
* The left and right arrow keys
* The home and end keys
* With control + arrow left or right the cursor moves jumping from word to word
* Delete characters with the backspace and delete keys
* Delete full words with shift + backspace
* With Tab and shift + tab you can autocomplete by looking in the hints
* With the up and down arrows you can autocomplete by looking in the history

# Interface with user project

The user must define at least two functions. vt100-iface uses these two functions to print on the terminal, one to print one character and another for strings.

```C
/** It prints a character to a terminal.
  * @param c Character to be sent.
  * @param p A valid instance of a terminal.
  * @return  On success, a non-negative value is returned.
  *          On error, -1 is returned  */
int tputc( int c, void* p );

/** It prints a null-terminated string to a terminal.
  * @param c Character to be sent.
  * @param p A valid instance of a terminal.
  * @return  On success, a non-negative value is returned.
  *          On error, -1 is returned  */
int tputs( char const* str, void* p );
```

Currently vt100-iface does not evaluate the return value of these functions. The parameter p is given by the user in the configuration of each instance of vt100-iface.

Optionally a third function can be defined to read characters from a terminal. It is useful in systems where it does not matter that the reading in the terminal is blocking, usually in multi-threaded systems. If you do not need this feature, exclude the file vt100-tgetc.c from the compilation.

```C
/** Get blocked until get a character from a terminal.
  * @param p A valid instance of a terminal.
  * @retval On success, the character read is returned (promoted to an int value).
  * @retval On error, a negative value. */
int tgetc( void* p );
```

In embedded systems these functions can interact with peripheral drivers such as UART or USB. In the example application in this repo they transfer the text by TCP sockets. In the unit tests they write and read in arrays that are then checked.

  