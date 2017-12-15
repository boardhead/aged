//==============================================================================
// File:        PEventControlWindow.h
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#ifndef __PEventControlWindow_h__
#define __PEventControlWindow_h__

#include <Xm/Xm.h>
#include "PWindow.h"
#include "PListener.h"
#include "PMenu.h"
#include "ImageData.h"

enum TriggerFlag {
    TRIGGER_OFF,                        /* trigger is off */
    TRIGGER_SINGLE,                     /* capture next event only */
    TRIGGER_CONTINUOUS                  /* run continuously */
};


class PEventControlWindow : public PWindow, public PListener, public PMenuHandler {
public:
    PEventControlWindow(ImageData *data);
    
    virtual void    Listen(int message, void *dataPt);
    virtual void    Show();
    
    void            UpdateTriggerText();
    void            UpdateEventNumber();
    void            UpdateHistoryLabel(int isHistory);
    int             GetTriggerFlag()    { return mTriggerFlag; }
    
    static void     UpdateTriggerText(ImageData *data);
    static void     UpdateEventNumber(ImageData *data);
    static void     SetEventFilter(ImageData *data);

    static void     GotoProc(Widget w,PEventControlWindow *pe_win, caddr_t call_data);
    static void     WriteProc(Widget w,PEventControlWindow *pe_win, caddr_t call_data);
    
private:
    void            SetNhitText();
    void            SetTriggerMaskText();
    void            SetPmtNcdLogicText();
    void            SetGotoType(int type);
    void            CreateTriggerRadio(int num);
    
    int             mTriggerFlag;   // flag for radio button activated
    Widget          trigger_radio[3];   /* trigger types */
    Widget          thresh_text, trigger_text;
    Widget          trigger_label, history_label;
};

#endif // __PEventControlWindow_h__
