#ifndef TMT_H
#define TMT_H

#include <stdbool.h>
#include <wchar.h>

/**** INVALID WIDE CHARACTER INVALID CHAR */
#ifndef TMT_INVALID_CHAR
#define TMT_INVALID_CHAR ((wchar_t)0xfffd)
#endif

/**** INPUT SEQUENCES */
#define TMT_KEY_UP             "\033[A"
#define TMT_KEY_DOWN           "\033[B"
#define TMT_KEY_RIGHT          "\033[C"
#define TMT_KEY_LEFT           "\033[D"
#define TMT_KEY_HOME           "\033[H"
#define TMT_KEY_END            "\033[Y"
#define TMT_KEY_CTL_UP         "\033[1;5A"
#define TMT_KEY_CTL_DOWN       "\033[1;5B"
#define TMT_KEY_CTL_RIGHT      "\033[1;5C"
#define TMT_KEY_CTL_LEFT       "\033[1;5D"
#define TMT_KEY_BACKSPACE      "\x7f"
#define TMT_KEY_PAUSE          "\x1a"
#define TMT_KEY_ESCAPE         "\x1b"
#define TMT_KEY_INSERT         "\033[@"
#define TMT_KEY_DELETE         "\177"
#define TMT_KEY_PAGE_UP        "\033[V"
#define TMT_KEY_PAGE_DOWN      "\033[U"
#define TMT_KEY_F1             "\033OP"
#define TMT_KEY_F2             "\033OQ"
#define TMT_KEY_F3             "\033OR"
#define TMT_KEY_F4             "\033OS"
#define TMT_KEY_F5             "\033OT"
#define TMT_KEY_F6             "\033OU"
#define TMT_KEY_F7             "\033OV"
#define TMT_KEY_F8             "\033OW"
#define TMT_KEY_F9             "\033OX"
#define TMT_KEY_F10            "\033OY"

/**** BASIC DATA STRUCTURES */
typedef struct TMT TMT;

typedef enum{
    TMT_COLOR_BLACK,
    TMT_COLOR_RED,
    TMT_COLOR_GREEN,
    TMT_COLOR_YELLOW,
    TMT_COLOR_BLUE,
    TMT_COLOR_MAGENTA,
    TMT_COLOR_CYAN,
    TMT_COLOR_WHITE,

    TMT_COLOR_MAX
} tmt_color_t;

typedef struct TMTATTRS TMTATTRS;
struct TMTATTRS{
    bool bold;
    bool dim;
    bool underline;
    bool blink;
    bool reverse;
    bool invisible;
    tmt_color_t fg;
    tmt_color_t bg;
};

typedef struct TMTCHAR TMTCHAR;
struct TMTCHAR{
    wchar_t c;
    TMTATTRS a;
};

typedef struct TMTPOINT TMTPOINT;
struct TMTPOINT{
    size_t r;
    size_t c;
};

typedef struct TMTLINE TMTLINE;
struct TMTLINE{
    bool dirty;
    TMTCHAR chars[];
};

typedef struct TMTSCREEN TMTSCREEN;
struct TMTSCREEN{
    size_t nline;
    size_t ncol;

    TMTLINE **lines;
};

/**** CALLBACK SUPPORT */
typedef enum{
    TMT_MSG_MOVED,
    TMT_MSG_UPDATE,
    TMT_MSG_BELL
} tmt_msg_t;

typedef void (*TMTCALLBACK)(tmt_msg_t m, struct TMT *v, const void *r, void *p);

/**** PUBLIC FUNCTIONS */
TMT *tmt_open(size_t nline, size_t ncol, TMTCALLBACK cb, void *p);
void tmt_close(TMT *vt);
bool tmt_resize(TMT *vt, size_t nline, size_t ncol);
void tmt_write(TMT *vt, const wchar_t *w, size_t n);
void tmt_writemb(TMT *vt, const char *s, size_t n);
const TMTSCREEN *tmt_screen(const TMT *vt);
const TMTPOINT *tmt_cursor(const TMT *vt);
void tmt_clean(TMT *vt);
void tmt_reset(TMT *vt);

#endif
