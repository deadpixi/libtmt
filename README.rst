
============================================
libtmt - a simple terminal emulation library
============================================

libtmt is the Tiny Mock Terminal Library.  It provides emulation of a classic
smart text terminal, by maintaining an in-memory screen image.  Sending text
and command sequences to libtmt causes it to update this in-memory image,
which can then be examined and rendered however the user sees fit.

libtmt is similar in purpose to `libtsm`_, but considerably smaller (500
lines versus 6500 lines). libtmt is also, in this author's humble opinion,
considerably easier to use.

.. _`libtsm`: https://www.freedesktop.org/wiki/Software/kmscon/libtsm/

Major Features and Advantages
=============================

Works Out-of-the-Box
    libtmt emulates a well-known terminal type (`mach` and/or `mach-color`),
    the definitions of which have been in the terminfo database
    since 1998.  There's no need to install a custom terminfo entry.
    There's no claiming to be an xterm but only emulating a small subset
    of its features. Any program using terminfo works automatically:
    this includes vim, emacs, mc, cmus, nano, nethack, ...

Portable
    Written in pure C99.
    Optionally, the POSIX-mandated `wcwidth` function can be used, which
    provides minimal support for combining characters.

Small
    Less than 500 lines of C, including comments and whitespace.

Free
    Released under a BSD-style license, free for commercial and
    non-commerical use, with no restrictions on source code release or
    redistribution.

Simple
    Only 10 functions to learn, and really you can get by with 6!

International
    libtmt internally uses wide characters exclusively, and uses your C
    library's multibyte encoding functions.
    This means that the library automatically supports any encoding that
    your operating system does.

How to Use libtmt
=================

libtmt is a single C file and a single header.  Just include these files
in your project and you should be good to go.

By default, libtmt uses only ISO standard C99 features,
but see `Compile-Time Options`_ below.

Example Code
------------

Here is a simple program fragment giving the flavor of libtmt:


.. code:: c


    #include <stdio.h>
    #include <stdlib.h>
    #include "tmt.h"

    /* Forward declaration of a callback.
     * libtmt will call this function when the terminal's state changes.
     */
    void callback(tmt_msg_t m, TMT *vt, const void *a, void *p);

    int
    main(void)
    {
        /* Open a virtual terminal with 2 lines and 10 columns.
         * The final NULL is just a pointer that will be provided to the
         * callback; it can be anything.
         */
        TMT *vt = tmt_open(2, 10, callback, NULL);
        if (!vt)
            return perror("could not allocate terminal"), EXIT_FAILURE;

        /* Write some text to the terminal, using escape sequences to
         * use a bold rendition. The "mb" is to indicate that we're
         * passing in a string using the operating system's default
         * multibyte encoding; we could also pass wide characters in
         * directly.
         *
         * The final argument is the length of the input; 0 means that
         * libtmt will determine the length dynamically using strlen.
         */
        tmt_writemb(vt, "\033[1mhello, world (in bold!)\033[0m", 0);

        /* Writing input to the virtual terminal can (and in this case, did)
         * call the callback letting us know the screen was updated. See the
         * callback below to see how that works.
         */
        tmt_close(vt);
        return EXIT_SUCCESS;
    }

    void
    callback(tmt_msg_t m, TMT *vt, const void *a, void *p)
    {
        /* grab a pointer to the virtual screen */
        const TMTSCREEN *s = tmt_screen(vt);
        const TMTPOINT *c = tmt_cursor(vt);

        switch (m){
            case TMT_MSG_BELL:
                /* the terminal is requesting that we ring the bell/flash the
                 * screen/do whatever ^G is supposed to do; a is NULL
                 */
                printf("bing!\n");
                break;

            case TMT_MSG_UPDATE:
                /* the screen image changed; a is a pointer to the TMTSCREEN */
                for (size_t r = 0; r < s->nline; r++){
                    if (s->lines[r]->dirty){
                        for (size_t c = 0; c < s->ncol; c++){
                            printf("contents of %zd,%zd: %lc (%s bold)\n", r, c,
                                   s->lines[r]->chars[c].c,
                                   s->lines[r]->chars[c].a.bold? "is" : "is not");
                        }
                    }
                }

                /* let tmt know we've redrawn the screen */
                tmt_clean(vt);
                break;

            case TMT_MSG_MOVED:
                /* the cursor moved; a is a pointer to the cursor's TMTPOINT */
                printf("cursor is now at %zd,%zd\n", c->r, c->c);
                break;
        }
    }

Data Types and Enumerations
---------------------------

.. code:: c

    /* an opaque structure */
    typedef struct TMT TMT;

    /* possible messages sent to the callback */
    typedef enum{
        TMT_MSG_MOVED,  /* the cursor changed position */
        TMT_MSG_UPDATE, /* the screen image changed    */
        TMT_MSG_BELL    /* the terminal bell was rung  */
    } tmt_msg_T;

    /* a callback for the library
     * m is one of the message constants above
     * vt is a pointer to the vt structure
     * r is NULL for TMT_MSG_BELL
     *   a pointer to the cursor's TMTPOINT for TMT_MSG_MOVED
     *   a pointer to the terminal's TMTSCREEN for TMT_MSG_UPDATE
     * p is whatever was passed to tmt_open (see below).
     */
    typedef void (*TMTCALLBACK)(tmt_msg_t m, struct TMT *vt,
                                const void *r, void *p);

    /* color definitions */
    typedef enum{
        TMT_COLOR_BLACK,
        TMT_COLOR_RED,
        TMT_COLOR_GREEN,
        TMT_COLOR_YELLOW,
        TMT_COLOR_BLUE,
        TMT_COLOR_MAGENTA,
        TMT_COLOR_CYAN,
        TMT_COLOR_WHITE
    } tmt_color_t;

    /* graphical rendition */
    typedef struct TMTATTRS TMTATTRS;
    struct TMTATTRS{
        bool bold;      /* character is bold             */
        bool dim;       /* character is half-bright      */
        bool underline; /* character is underlined       */
        bool blink;     /* character is blinking         */
        bool reverse;   /* character is in reverse video */
        bool invisible; /* character is invisible        */
        tmt_color_t fg; /* character foreground color    */
        tmt_color_t bg; /* character background color    */
    };

    /* characters */
    typedef struct TMTCHAR TMTCHAR;
    struct TMTCHAR{
        wchar_t  c; /* the character */
        TMTATTRS a; /* its rendition */
    };

    /* a position on the screen; upper left corner is 0,0 */
    typedef struct TMTPOINT TMTPOINT;
    struct TMTPOINT{
        size_t r; /* row    */
        size_t c; /* column */
    };

    /* a line of characters on the screen;
     * every line is always as wide as the screen
     */
    typedef struct TMTLINE TMTLINE;
    struct TMTLINE{
        bool dirty;     /* line has changed since it was last drawn */
        TMTCHAR chars;  /* the contents of the line                 */
    };

    /* a virtual terminal screen image */
    typedef struct TMTSCREEN TMTSCREEN;
    struct TMTSCREEN{
        size_t nline;    /* number of rows          */
        size_t ncol;     /* number of columns       */
        TMTLINE **lines; /* the lines on the screen */
    };

Functions
---------

`TMT *tmt_open(size_t nrows, size_t ncols, TMTCALLBACK cb, VOID *p);`
    Creates a new virtual terminal, with `nrows` rows and `ncols` columns.
    The callback `cb` will be called on updates, and passed `p` as a final
    argument. See the definition of `tmt_msg_t` above for possible values
    of each argument to the callback.

    Note that the callback must be ready to be called immediately, as it
    will be called after initialization of the terminal is done, but before
    the call to `tmt_open` returns.

`void tmt_close(TMT *vt)`
    Close and free all resources associated with `vt`.

`bool tmt_dirty(const TMT *vt)`
    Returns true if `vt` has been modified since it was last drawn.
    It is not usually necessary to call this function, as the callback
    provided to `tmt_open` will be called with `TMT_MSG_UPDATE` whenever
    `vt` is updated.

`bool tmt_resize(TMT *vt, size_t nrows, size_t ncols)`
    Resize the virtual terminal to have `nrows` rows and `ncols` columns.
    The contents of the area in common between the two sizes will be preserved.

    If this function returns false, the resize failed (only possible in
    out-of-memory conditions). If this happens, the terminal is trashed and
    the only valid operation is the close the terminal (and, optionally,
    open a new one).

`void tmt_write(TMT *vt, const wchar_t *w, size_t n);`
    Write the wide-character string to the terminal, interpreting any escape
    sequences contained threin, and update the screen image.  The last
    argument is the length of the input in wide characters, if set to 0,
    the length is determined using `wcslen`.

    The terminal's callback function may be invoked one or more times before
    calls to this function return.

void tmt_writemb(TMT *vt, const char *s, size_t n);`
    Write the provided string to the terminal, interpreting any escape
    sequences contained threin, and update the screen image. The last
    argument is the length of the input in wide characters, if set to 0,
    the length is determined using `strlen`.

    The terminal's callback function may be invoked one or more times before
    calls to this function return.

    The string is converted internally to a wide-character string using the
    system's current multibyte encoding. Each terminal maintains a private
    multibyte decoding state, and correctly handles mulitbyte characters that
    span multiple calls to this function (that is, the final byte(s) of `s`
    may be a partial mulitbyte character to be completed on the next call).

`const TMTSCREEN *tmt_screen(const TMT *vt);`
    Returns a pointer to the terminal's screen image.

`const TMTPOINT *tmt_cursor(cosnt TMT *vt);`
    Returns a pointer to the terminal's cursor position.

`void tmt_clean(TMT *vt);`
    Call this after receiving a `TMT_MSG_UPDATE` or `TMT_MSG_MOVED` callback
    to let the library know that the program has handled all reported changes
    to the screen image.

`void tmt_reset(TMT *vt);`
    Resets the virtual terminal to its default state (colors, multibyte
    decoding state, rendition, etc).

Special Keys
------------

To send special keys to a program that is using libtmt for its display,
write one of the `TMT_KEY_*` strings to that program's standard input
(*not* to libtmt; it makes no sense to send any of these constants to
libtmt itself).

The following macros are defined, and are all constant strings:

- TMT_KEY_UP
- TMT_KEY_DOWN
- TMT_KEY_RIGHT
- TMT_KEY_LEFT
- TMT_KEY_HOME
- TMT_KEY_END
- TMT_KEY_BACKSPACE
- TMT_KEY_ESCAPE
- TMT_KEY_PAGE_UP
- TMT_KEY_PAGE_DOWN
- TMT_KEY_F1 through TMT_KEY_F10

Compile-Time Options
--------------------

There are two preprocessor macros that affect libtmt:

`TMT_INVALID_CHAR`
    Define this to a wide-character. This character will be added to
    the virtual display when an invalid multibyte character sequence
    is encountered.

    By default (if you don't define it as something else before compiling),
    this is `((wchar_t)0xfffd)`, which is the codepoint for the Unicode
    'REPLACEMENT CHARACTER'. Note that your system might not use Unicode,
    and its wide-character type might not be able to store a constant as
    large as `0xfffd`, in which case you'll want to use an alternative.

`TMT_HAS_WCWIDTH`
    By default, libtmt uses only standard C99 features.  If you define
    TMT_HAS_WCWIDTH before compiling, libtmt will use the POSIX `wcwidth`
    function to detect combining characters.

    Note that combining characters are still not handled particularly
    well, regardless of whether this was defined. Also note that what
    your C library's `wcwidth` considers a combining character and what
    the written language in question considers one could be different.

Supported Input and Escape Sequences
====================================

Internally libtmt uses your C library's/compiler's idea of a wide character for
all characters, so you should be able to use whatever characters you want when
writing to the virtual terminal.

The following escape sequences are recognized and will be processed specially:

+-------------+------------------------------------------------------------------------+
| Sequence    |   Meaning                                                              |
+=============+========================================================================+
| `ESC c`     | Reset the terminal to its default state and clear the screen.          |
+-------------+------------------------------------------------------------------------+
| `ESC # A`   | Move the cursor up # rows.                                             |
+-------------+------------------------------------------------------------------------+
| `ESC # B`   | Move the cursor down # rows.                                           |
+-------------+------------------------------------------------------------------------+
| `ESC # C`   | Move the cursor right # columns.                                       |
+-------------+------------------------------------------------------------------------+
| `ESC # D`   | Move the cursor left # columns.                                        |
+-------------+------------------------------------------------------------------------+
| `ESC # E`   | Move the cursor to the beginning of the #th next row down.             |
+-------------+------------------------------------------------------------------------+
| `ESC # F`   | Move the cursor to the beginning of the #th previous row up.           |
+-------------+------------------------------------------------------------------------+
| `ESC # G`   | Move the cursor to the #th column.                                     |
+-------------+------------------------------------------------------------------------+
| `ESC #;# H` | Move the cursor to the row and column specified.                       |
+-------------+------------------------------------------------------------------------+
| `ESC # J`   | - # = 0: clear from cursor to end of screen                            |
|             | - # = 1: clear from beginning of screen to cursor                      |
|             | - # = 2: clear entire screen                                           |
+-------------+------------------------------------------------------------------------+
| `ESC # K`   | - # = 0: clear from cursor to end of line                              |
|             | - # = 1: clear from beginning of line to cursor                        |
|             | - # = 2: clear entire line                                             |
+-------------+------------------------------------------------------------------------+
| `ESC # L`   | Insert # lines before the current line, scrolling lower lines down.    |
+-------------+------------------------------------------------------------------------+
| `ESC # M`   | Delete # lines (including the current line), scrolling lower lines up. |
+-------------+------------------------------------------------------------------------+
| `ESC # P`   | Delete # characters, scrolling later characters left.                  |
+-------------+------------------------------------------------------------------------+
| `ESC # S`   | Scroll the screen up by # lines.                                       |
+-------------+------------------------------------------------------------------------+
| `ESC # T`   | Scroll the screen down by # lines.                                     |
+-------------+------------------------------------------------------------------------+
| `ESC # X`   | Overwrite # characters with spaces.                                    |
+-------------+------------------------------------------------------------------------+
| `ESC #;...m`| Change the graphical rendition properties according to the table below.|
|             | Up to eight properties may be set in one command.                      |
+-------------+------------------------------------------------------------------------+
| `ESC # @`   | Insert # blank spaces, moving later characters right.                  |
+-------------+------------------------------------------------------------------------+

==============   ==================
Rendition Code   Meaning
==============   ==================
0                Normal text
1                Bold
2                Dim (half bright)
4                Underline
5                Blink
7                Reverse video
8                Invisible
24               Underline off
27               Reverse video off
30               Forground black
31               Forground red
32               Forground green
33               Forground yellow
34               Forground blue
35               Forground magenta
36               Forground cyan
37               Forground white
40               Background black
41               Background red
42               Background green
43               Background yellow
44               Background blue
45               Background magenta
46               Background cyan
47               Background white
==============   ==================

For those escape sequences that take arguments, the default for an empty or
missing argument is the smallest meaningful number (which is 0 for `SGR`, `ED`,
and `EL`, and 1 for all others).

For the cursor movement commands, the cursor is constrained to the bounds of
the screen and the contents of the screen are not scrolled.

Characters and lines moved off the side or bottom of screen are lost.

Note that most users find blinking text annoying, and it can be dangerous for
those who suffer from epilepsy and other conditions.

Known Issues
============

- Combining characters are "handled" by ignoring them
  (when compiled with `TMT_HAS_WCWIDTH`) or by printing them separately.
- The documentation and error messages are available only in English.

Frequently Asked Questions
==========================

Why does libtmt emulate mach terminals? Why not xterm/screen/rxvt/ANSI?
-----------------------------------------------------------------------

For several reasons, really.

I like to multiplex my terminal windows, a la tmux or screen, but I don't
like using tmux or screen.  (Note that this is not a dig at either of those
absolutely fantastic programs; I just prefer minimalist implementations.)

I used `dvtm`_ for a long time, and it is also an excellent piece of
software.  However, it suffers from a few issues that I wanted to work
around: it crashes or fails to start up correctly sometimes, it's getting
a little feature-bloated for my taste, and its terminal definition is not
universally deployed.

.. _`dvtm`: http://www.brain-dump.org/projects/dvtm/

The final issue is the real sticking point.  I SSH into a lot of old
machines for my job, and it's not always feasible to get the dvtm terminfo
entry onto them.

So I wanted to find a terminal that had universal support in terminfo,
even on older machines.  There were plenty to choose from, including
such venerable options as xterm, but they were all relatively complex.
I needed something simple enough that I could implement it myself and be
reasonably sure that I got it right.

For example, xterm defines 488 escape sequences, with multiple and varied
syntaxes, modified by dozens of modes. ECMA-48 is of similar complexity.
Essentially no terminals completely implement ECMA-48 and of the dozens
of emulators that claim to be an xterm, only xterm actually implements
all of the features.

This leads to the sad state of affairs where if a terminal claims to be
an xterm or to implement ECMA-48, you can't actually rely on it doing so,
and have to stick to some ill-defined "common subset" of features that
isn't really written down anywhere and the contents of which vary from
person to person.

I originally targeted the classic ANSI.SYS emulation from the days
of MS-DOS.  That was a very simple terminal to emulate, but more modern
systems use the same terminfo entry ("pcansi") to refer to more modern
systems and relegate the classic definition to names like "ansi.sys-old".
This latter terminal definition isn't always deployed.

I then ended up targeting the `Minix`_ console, which was incredibly simple
(only 16 escape sequences).  Sadly, one of the requirements of libtmt was
to work transparently with multibyte characters in any multibyte encoding
supported the operating sytem.  The common terminfo entry for minix maps
box-drawing characters to a fixed set of codes with the high-bit set,
which breaks many multibyte encoding schemes.  If libtmt stuck with Minix
emulation, it would never support box drawing (and, what's worse, would
corrupt the display if boxes were drawn).

.. _`Minix`: http://www.minix3.org

This finally led to my picking the `Mach`_ console to emulate.  It was
almost as small as Minix's (only 19 escape sequences, no modes), meaning
it was small enough that I could write an emulation by myself in a short
amount of time.  It has been in the common terminfo database since 1998,
and unmodified since 2001.  Its definition was present on every machine I
could check, so I knew that an emulator based on that standard would work
out-of-the-box essentially everywhere.

.. _`Mach`: http://www.cs.cmu.edu/afs/cs/project/mach/public/www/mach.html

But shouldn't libtmt emulate a more powerful terminal?
------------------------------------------------------

Why? There are two possibilities for a program doing terminal output:
assume the terminal, or use terminfo/termcap.

In the first case (assuming the terminal), the program just isn't going
to work on some terminals, and not just libtmt-based ones.

In the later case (using terminfo/termcap), the program will work for any
terminal with a terminfo entry.  As established above, mach and mach-color
have widely-deployed and stable terminfo entries.

It's true that libtmt lacks some of the more advanced features provided by,
e.g. xterm, like mouse input tracking and terminal title setting. If you
absolutely need one of those features, libtmt isn't going to work for you
(and neither will some other common terminal types).

Also, it should be pointed out that every escape sequence and feature is a
potential source of bugs and security issues.  Witness a bug that I found
years ago in Mac OS X's Terminal.app in its handling of the xterm resizing
escape sequences that lead to remote code execution.  I wrote a `blog entry`_
about it in a past life.

.. _`blog entry`: http://web.archive.org/web/20090625043244/http://dvlabs.tippingpoint.com/blog/2009/06/05/whats-worse-than-finding-a-bug-in-your-apple

(It was actually a bigger threat than you might think. At the time, Safari
on Mac OS X would automatically open `telnet://` URIs in Terminal.app,
including such URI's in invisible frames in web pages. You could visit
a page in Safari which would open Terminal.app and have it telnet to a
malicious host that you controlled that would send a bad escape sequence
and execute arbitrary code. It was pretty interesting...)

What programs work with libtmt?
-------------------------------

Pretty much all of them.  As addressed in the previous question, if a
program hardcodes expectations about what terminal it's running on, it's
going to fail sometimes, and not just on libtmt.

I've tested quite a few applications in libtmt and they've worked flawlessly:
vim, GNU emacs, nano, cmus, mc (Midnight Commander), and others just work
with no changes.

What programs don't work with libtmt?
-------------------------------------

Breakage with libtmt is of two kinds: breakage due to assuming a terminal
type, and reduced functionality.

In all my testing, I only found one program that didn't work correctly by
default with libtmt: recent versions of Debian's `apt`_ assume a terminal
with definable scrolling regions to draw a facing progress bar during
package installation.  Using apt in its default configuration in libtmt will
result in a corrupted display (that can be fixed by clearing the screen).

.. _`apt`: https://wiki.debian.org/Apt

In my honest opinion, this is a bug in apt: it shouldn't assume the type
of terminal it's running in.

The second kind of breakage is when not all of a program's features are
available.  The biggest missing feature here is mouse support: libtmt
doesn't, and probably never will, support mouse tracking.  I know of many
programs that *can* use mouse tracking in a terminal, but I don't know
of any that *require* it.  Most (if not all?) programs of this kind would
still be completely usable in libtmt.

License
-------

Copyright (c) 2017 Rob King
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
- Neither the name of the copyright holder nor the
  names of contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS,
COPYRIGHT HOLDERS, OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
