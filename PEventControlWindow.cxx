//==============================================================================
// File:        PEventControlWindow.cxx
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <X11/IntrinsicP.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include "PEventControlWindow.h"
#include "PResourceManager.h"
#include "PSpeaker.h"
#include "PUtils.h"
#include "menu.h"

#define WINDOW_WIDTH        350
#define WINDOW_HEIGHT       113
#define WINDOW_MIN_HEIGHT   109


//-----------------------------------------------------------------------------
// callbacks
//
static void fwdProc(Widget w, ImageData *data, caddr_t call_data)
{
    aged_next(data,1);
}

static void backProc(Widget w, ImageData *data, caddr_t call_data)
{
    aged_next(data,-1);
}

static void ffwdProc(Widget w, ImageData *data, caddr_t call_data)
{
    aged_next(data,10);
}

static void fbackProc(Widget w, ImageData *data, caddr_t call_data)
{
    aged_next(data,-10);
}

static void filterProc(Widget w, ImageData *data, caddr_t call_data)
{
    PEventControlWindow::SetEventFilter(data);
    PEventControlWindow::UpdateTriggerText(data);
}

static void offProc(Widget w, ImageData *data, caddr_t call_data)
{
    PEventControlWindow *pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
    if (pe_win->GetTriggerFlag() != TRIGGER_OFF) {
        setTriggerFlag(data,TRIGGER_OFF);
    }
}

static void singleProc(Widget w, ImageData *data, caddr_t call_data)
{
    PEventControlWindow *pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
    if (pe_win->GetTriggerFlag() != TRIGGER_SINGLE) {
        PEventControlWindow::SetEventFilter(data);
        setTriggerFlag(data,TRIGGER_SINGLE);
    }
}

static void contProc(Widget w, ImageData *data, caddr_t call_data)
{
    PEventControlWindow *pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
    if (pe_win->GetTriggerFlag() != TRIGGER_CONTINUOUS) {
        PEventControlWindow::SetEventFilter(data);
        setTriggerFlag(data,TRIGGER_CONTINUOUS);
    }
}

static void scaleProc(Widget w, ImageData *data, XmScaleCallbackStruct *call_data)
{
    data->time_interval = call_data->value / 10.0;
    
    if (data->trigger_flag == TRIGGER_CONTINUOUS) {
        PEventControlWindow::UpdateTriggerText(data);
    }
}

//-----------------------------------------------------------------------------------
// PEventControlWindow constructor
//
PEventControlWindow::PEventControlWindow(ImageData *data)
                   : PWindow(data)
{
    int     n;
    Arg     wargs[20];
    Widget  w, but;
    
    data->mSpeaker->AddListener(this);  // listen for trigger changed messages
    PResourceManager::sSpeaker->AddListener(this);
    
    mTriggerFlag = data->trigger_flag;
    
    n = 0;
    XtSetArg(wargs[n], XmNtitle, "Event Control"); ++n;
    XtSetArg(wargs[n], XmNx, 625); ++n;
    XtSetArg(wargs[n], XmNy, 20); ++n;
    XtSetArg(wargs[n], XmNwidth, WINDOW_WIDTH); ++n;
    XtSetArg(wargs[n], XmNheight, WINDOW_HEIGHT); ++n;
    XtSetArg(wargs[n], XmNminWidth, WINDOW_WIDTH); ++n;
    XtSetArg(wargs[n], XmNminHeight, WINDOW_MIN_HEIGHT); ++n;
    SetShell(CreateShell("evtPop",data->toplevel,wargs,n));
    SetMainPane(w = XtCreateManagedWidget("agedForm",xmFormWidgetClass,GetShell(),NULL,0));

    n = 0;
    XtSetArg(wargs[n], XmNx, 16); ++n;
    XtSetArg(wargs[n], XmNy, 8); ++n;
    XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_BEGINNING); ++n;
    trigger_label = XtCreateManagedWidget("trigger_text",xmLabelWidgetClass,w,wargs,n);
    
    n = 0;
    XtSetArg(wargs[n], XmNy, 33); ++n;
    XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
    XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
    XtCreateManagedWidget("sep1",xmSeparatorWidgetClass,w,wargs,n);
    
    CreateTriggerRadio(TRIGGER_CONTINUOUS);
    CreateTriggerRadio(TRIGGER_SINGLE);
    CreateTriggerRadio(TRIGGER_OFF);
        
    n = 0;
    XtSetArg(wargs[n], XmNx, 16); ++n;
    XtSetArg(wargs[n], XmNy, 82); ++n;
    XtCreateManagedWidget("Delay (sec):",xmLabelWidgetClass,w,wargs,n);
    
    n = 0;
    XtSetArg(wargs[n], XmNx, 135); ++n;
    XtSetArg(wargs[n], XmNy, 67); ++n;
    XtSetArg(wargs[n], XmNwidth, 198); ++n;
    XtSetArg(wargs[n], XmNminimum, 1); ++n;
    XtSetArg(wargs[n], XmNmaximum, 100); ++n;
    XtSetArg(wargs[n], XmNdecimalPoints, 1); ++n;
    XtSetArg(wargs[n], XmNshowValue, TRUE); ++n;
    XtSetArg(wargs[n], XmNorientation, XmHORIZONTAL); ++n;
    XtSetArg(wargs[n], XmNvalue, (int)(data->time_interval * 10)); ++n;
    but = XtCreateManagedWidget("Scale",xmScaleWidgetClass,w,wargs,n);
    XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)scaleProc,data);

    n = 0;
    XtSetArg(wargs[n], XmNx, 135); ++n;
    XtSetArg(wargs[n], XmNy, 114); ++n;
    XtSetArg(wargs[n], XmNwidth, 34); ++n;
    but = XtCreateManagedWidget("-1",xmPushButtonWidgetClass,w,wargs,n);
    XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)backProc,data);

    n = 0;
    XtSetArg(wargs[n], XmNx, 171); ++n;
    XtSetArg(wargs[n], XmNy, 114); ++n;
    XtSetArg(wargs[n], XmNwidth, 34); ++n;
    but = XtCreateManagedWidget("+1",xmPushButtonWidgetClass,w,wargs,n);
    XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)fwdProc,data);

    n = 0;
    XtSetArg(wargs[n], XmNx, 207); ++n;
    XtSetArg(wargs[n], XmNy, 114); ++n;
    XtSetArg(wargs[n], XmNwidth, 38); ++n;
    but = XtCreateManagedWidget("-10",xmPushButtonWidgetClass,w,wargs,n);
    XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)fbackProc,data);

    n = 0;
    XtSetArg(wargs[n], XmNx, 247); ++n;
    XtSetArg(wargs[n], XmNy, 114); ++n;
    XtSetArg(wargs[n], XmNwidth, 38); ++n;
    but = XtCreateManagedWidget("+10",xmPushButtonWidgetClass,w,wargs,n);
    XtAddCallback(but,XmNactivateCallback,(XtCallbackProc)ffwdProc,data);

    n = 0;
    XtSetArg(wargs[n], XmNx, 16); ++n;
    XtSetArg(wargs[n], XmNy, 147); ++n;
    XtCreateManagedWidget("NHIT Thresh:", xmLabelWidgetClass,w,wargs,n);

    n = 0;
    XtSetArg(wargs[n], XmNleftOffset, 135); ++n;
    XtSetArg(wargs[n], XmNy, 142); ++n;
    XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
    XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
    XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
    XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
    thresh_text = XtCreateManagedWidget("Threshold",xmTextWidgetClass,w,wargs,n);
    XtAddCallback(thresh_text,XmNactivateCallback,(XtCallbackProc)filterProc,data);
    //SetNhitText();

    n = 0;
    XtSetArg(wargs[n], XmNx, 16); ++n;
    XtSetArg(wargs[n], XmNy, 180); ++n;
    XtCreateManagedWidget("Trigger Mask:", xmLabelWidgetClass,w,wargs,n);

    n = 0;
    XtSetArg(wargs[n], XmNleftOffset, 135); ++n;
    XtSetArg(wargs[n], XmNy, 175); ++n;
    XtSetArg(wargs[n], XmNleftAttachment, XmATTACH_FORM); ++n;
    XtSetArg(wargs[n], XmNrightAttachment, XmATTACH_FORM); ++n;
    XtSetArg(wargs[n], XmNrightOffset, 16); ++n;
    XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
    trigger_text = XtCreateManagedWidget("Triggers",xmTextWidgetClass,w,wargs,n);
    XtAddCallback(trigger_text,XmNactivateCallback,(XtCallbackProc)filterProc,data);
    //SetTriggerMaskText();

    UpdateEventNumber();
    UpdateTriggerText();
}

void PEventControlWindow::CreateTriggerRadio(int num)
{
    int n;
    Arg wargs[24];
    Widget but;
    ImageData *data = GetData();
    
    switch (num) {
        case TRIGGER_CONTINUOUS:
            n = 0;
            XtSetArg(wargs[n], XmNx, 16); ++n;
            XtSetArg(wargs[n], XmNy, 40 + RADIO_OFFSET); ++n;
            XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
            XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
            XtSetArg(wargs[n], XmNset, mTriggerFlag == TRIGGER_CONTINUOUS); ++n;
            but = XtCreateManagedWidget("Continuous",xmToggleButtonWidgetClass,GetMainPane(),wargs,n);
            XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)contProc, data);
            break;
        case TRIGGER_SINGLE:
            n = 0;
            XtSetArg(wargs[n], XmNx, 165); ++n;
            XtSetArg(wargs[n], XmNy, 40 + RADIO_OFFSET); ++n;
            XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
            XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
            XtSetArg(wargs[n], XmNset, mTriggerFlag == TRIGGER_SINGLE); ++n;
            but = XtCreateManagedWidget("Next",xmToggleButtonWidgetClass,GetMainPane(),wargs,n);
            XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)singleProc, data);
            break;
        case TRIGGER_OFF:
            n = 0;
            XtSetArg(wargs[n], XmNx, 265); ++n;
            XtSetArg(wargs[n], XmNy, 40 + RADIO_OFFSET); ++n;
            XtSetArg(wargs[n], XmNmarginHeight, 2); ++n;
            XtSetArg(wargs[n], XmNindicatorType, XmONE_OF_MANY); ++n;
            XtSetArg(wargs[n], XmNset, mTriggerFlag == TRIGGER_OFF); ++n;
            but = XtCreateManagedWidget("Stop",xmToggleButtonWidgetClass,GetMainPane(),wargs,n);
            XtAddCallback(but,XmNvalueChangedCallback,(XtCallbackProc)offProc, data);
            break;
        default:
            return;
    }
    trigger_radio[num] = but;
}

void PEventControlWindow::Listen(int message, void *dataPt)
{
    switch (message) {
    
        case kMessageTriggerChanged: {
            int oldTrigger = mTriggerFlag;
            mTriggerFlag = GetData()->trigger_flag; // save new trigger type
            if (oldTrigger == mTriggerFlag) break;
            
            // unset old radio
            XmToggleButtonSetState(trigger_radio[oldTrigger], 0, FALSE);

            // set new radio
            XmToggleButtonSetState(trigger_radio[mTriggerFlag], 1, FALSE);

            UpdateTriggerText();    // update the text according to the new trigger setting
        }   break;
    }
}

void PEventControlWindow::UpdateTriggerText(ImageData *data)
{
    PEventControlWindow *pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
    if (pe_win) {
        pe_win->UpdateTriggerText();
    }
}

void PEventControlWindow::UpdateTriggerText()
{
    char        buff[1024];
    int         len;
    ImageData   *data = GetData();
    
    if (data->trigger_flag == TRIGGER_CONTINUOUS) {
        len = sprintf(buff,"Continuous  %.1fs",data->time_interval);
    } else if (data->trigger_flag == TRIGGER_SINGLE) {
        len = sprintf(buff,"Single");
    } else {
        len = sprintf(buff,"<stopped>");
    }
    setLabelString(trigger_label, buff);
    XtResizeWidget(trigger_label, 500, 20, 0);
}

/* set trigger logic variables from strings in text widgets */
void PEventControlWindow::SetEventFilter(ImageData *data)
{
    PEventControlWindow *pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
    
    if (pe_win) {
    }
}

void PEventControlWindow::Show()
{
    PWindow::Show();    // let the base class do the work
    
    if (!WasResized()) {
        Resize(WINDOW_WIDTH,WINDOW_HEIGHT);
    }
}

/* Set event number in window */
void PEventControlWindow::UpdateEventNumber(ImageData *data)
{
    PEventControlWindow *pe_win = (PEventControlWindow *)data->mWindow[EVT_NUM_WINDOW];
    if (pe_win) {
        pe_win->UpdateEventNumber();
    }
}

void PEventControlWindow::UpdateEventNumber()
{
}

