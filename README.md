# vt100-iface

[![Build Status](https://travis-ci.org/rafagafe/vt100-iface.svg?branch=master)](https://travis-ci.org/rafagafe/vt100-iface)

vt100-iface is a C library to read text lines from a VT100 terminal.

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

Optionally a third function can be defined to read characters from a terminal. It is useful in systems where it does not matter that the reading in the terminal is blocking, usually in multi-threaded systems. If you do not need this feature, exclude the file vt100-tgetc.c from the build.

```C
/** Get blocked until get a character from a terminal.
  * @param p A valid instance of a terminal.
  * @retval On success, the character read is returned (promoted to an int value).
  * @retval On error, a negative value. */
int tgetc( void* p );
```

In embedded systems these functions can interact with peripheral drivers such as UART or USB. In the example application in this repo they transfer the text by TCP sockets. In the unit tests they write and read in arrays that are then checked.

# Configuration

To configure each instance of vt100-iface we will pass a constant configuration structure. It contains pointers to history, hints, the parameter that is passed to the functions tputs, tputc and tgetc, and a pointer to the destination buffer and its size.

```C
/** Configuration to capture a line from a vt100 terminal */
struct vt100 {
    /** A valid instance of a terminal. It will be passed to tputc() and tputs() */
    void* p;
    struct history* hist;       /**< History handle or null if there is not. */
    struct hints const* hints;  /**< Hints handle or null if there is not.   */
    char* line;                 /**< Destination buffer.                     */
    int max;                    /**< Size of line buffer.                    */
};
```

If you do not use any of these features, its pointer can be left to NULL. The simplest configuration would be like this:

```C
/* Destination buffer */
static char line[80];

/* Configuration */
static struct vt100 const vt100 = {
    .p     = NULL,
    .hist  = NULL,
    .hints = NULL,
    .line  = line,
    .max   = sizeof line
};
```

# History

The files history.c and history.h are a standalone module. You can use for other purposes. To configure each instance of history we will pass a constant configuration structure. It contains the history dimensions and a pointer to the history memory block.

```C
/** History configuration. */
struct historycfg {
    void* lines;    /**< Memory block for history.   */
    short linelen;  /**< Length of lines in history. */
    short numlines; /**< Lines capacity in history.  */
};
```

A configuration of vt100-iface instance that uses a history would be like this:

```C
//...
    /* History dimensions: */
    enum {
        numlines = 16,
        linelen  = 80
    };

    /* Memory for history: */
    static char histlines[numlines][linelen];

    /* A history configuration. */ 
    static struct historycfg const histcfg = {
        .lines    = histlines,
        .linelen  = linelen,
        .numlines = numlines
    };

    /* Initialize a history instance: */
    struct history hist;
    history_init( &hist, &histcfg );
    
    /* Destination buffer */
    static char line[ linelen ];

    /* Configuration */
    static struct vt100 const vt100 = {
        .p     = NULL,
        .hist  = &hist, /* <--<< Add history handle */
        .hints = NULL,
        .line  = line,
        .max   = sizeof line
    };    
//...    
```

# Hints

A hints set is a structure that contains: 1) A reference to an array of pointers to null-terminated strings with each hint. 2) The length of this array.

```C
/** Set of hints for a line capture. */
struct hints {
    /** Pointer to array of null-terminated strings with the hints. */
    char const* const* str;
    /** Number of hints. */
    int qty;
};
```

A configuration of vt100-iface instance that uses a hints set would be like this:

```C
    /* Array with all hints the hints: */
    static char const* const names[] = {
        "clear", "help", "exit", "command", "sum", "mult"
    };
    
    
    static struct hints const hints = {
        .str  = names,
        .qty  = sizeof names / sizeof *names
    };
    
    /* Destination buffer */
    static char line[80];

    /* Configuration */
    static struct vt100 const vt100 = {
        .p     = NULL,
        .hist  = NULL,
        .hints = &hints, /* <--<< Add hints set handle */
        .line  = line,
        .max   = sizeof line
    };  
    
```

# Using with tgetc

Using vt100-iface with tgetc is very easy because the vt100_getline() function is blocking. When the vt100_getline() function returns an error or a line is captured. 

```C
extern void doerror( int );
extern void doline( char const* );

void func() {

    //...
    
    /* Configuration */
    static struct vt100 const vt100 = {
        //...
    };  
    
    for(;;) {    
        int len = vt100_getline( &vt100, echo_on );
        if( 0 > len ) {
            doerror( len );
            continue;
        }
        doline( vt100.line );
     }
}   
```

# Using without tgetc

When using vt100-iface without tgetc one should poll if a character has been received. The received characters are introduced in vt100_char function until it returns a non-negative value, which is the length of the captured line.

```C
extern bool anyCharReceived( void );
extern int getCharReceived( void );
extern void doline( char const* );

/* Periodically invoked */
void statemachine( void ) {
    
    /* state of state machine: */
    static enum {
        INIT,
        WAIT_CHAR
    } state = INIT;
    
    /* Destination buffer */
    static char line[80];

    /* Configuration */
    static struct vt100 const vt100 = {
        .p     = NULL,
        .hist  = NULL,
        .hints = NULL,
        .line  = line,
        .max   = sizeof line
    };
    
    /* vt100-iface state: */
    static struct vt100state st;
    
    switch( state ) {
        
        case INIT:
            vt100_init( &st, &vt100, echo_on );
            state = WAIT_CHAR;
            break;
            
        case WAIT_CHAR: {
            if ( anyCharReceived() ) {
                int ch = getCharReceived();
                int len = vt100_char( st, ch );
                if ( 0 <= len )
                    doline( line );
            }
            break;
        }
        
    }
}

```

# Command line arguments parser

The files clarg.c and clarg.h are a standalone module. You can use for other purposes. 

```C
extern void doerror( int err );
extern int docommand( int argc, char** argv );

int func() {

    enum {
        maxargc = 10
    };

    //...
    
    /* Configuration */
    static struct vt100 const vt100 = {
        //...
    };  
    
    for(;;) {

        /* Print prompt: */
        tputs( " \\> ", p );

        /* Get line: */
        int len = vt100_getline( &vt100 );
        if( 0 == len )
            continue;            
        if( 0 > len ) {
            fprintf( stderr, "%s%d\n", "Error", len );
            return -1;
        }

        /* Parse arguments: */
        char* argv[ maxargc ];
        int argc = clarg( argv, maxargc, vt100.line );
        if( 0 > argc )
            continue;
        
        /* Execute command: */
        int err = docommand( argc, argv );
        if( err )
            fprintf( stderr, "%s%s%s%d\n", "Command ", *argv, " returned the error: ", err );
            
    }
            
}   
```

