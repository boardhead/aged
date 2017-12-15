#ifndef __TextSpec_h__
#define __TextSpec_h__

#include <X11/Xlib.h>

struct TextSpec {
    XFontStruct   * font;
#ifdef ANTI_ALIAS
    XftFont       * xftFont;
#endif
    char          * string;
};

#endif // __TextSpec_h__
