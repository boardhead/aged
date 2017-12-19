//==============================================================================
// File:        PScrollingWindow.h
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#ifndef __PScrollingWindow_h__
#define __PScrollingWindow_h__

#include "PWindow.h"
#include "PScrollBar.h"
#include <Xm/Xm.h>


/* PScrollingWindow class definition */
class PScrollingWindow : public PWindow, public PScrollHandler {
public:
    PScrollingWindow(ImageData *data);
    virtual ~PScrollingWindow();
    
    /* scrollbar functions */
    virtual void    SetScrolls() { }
    void            SetScrollHandler(PScrollHandler *hand);
    void            NewScrollBar(EScrollBar bar, char *name, Arg *wargs, int n);
    PScrollBar    * GetScrollBar(EScrollBar bar)    { return mScrollBar[bar];              }
    Widget          GetScroll(EScrollBar bar)       { return mScrollBar[bar]->GetWidget(); }
    virtual void    SetScrollValue(EScrollBar bar, int value, int do_callback=0);
    virtual int     GetScrollValue(EScrollBar bar);
    
private:
    PScrollBar    * mScrollBar[kNumScrollBars];
};

#endif // __PScrollingWindow_h__
