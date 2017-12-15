//==============================================================================
// File:        AgedWindow.h
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#ifndef __AgedWindow_h__
#define __AgedWindow_h__

#include <Xm/Xm.h>
#include "PImageWindow.h"
#include "PListener.h"
#include "PSpeaker.h"
#include "PMenu.h"
#include "TextSpec.h"

const short kLabelSize      = 1024;
const short kMaxLabelLines  = 64;

enum ELabelFlags {
    kLabelRun       = 0x00000001,   // rn
    kLabelEvID      = 0x00000002,   // ev
    kLabelTime      = 0x00000004,   // ti
    kLabelDate      = 0x00000008,   // da
    kLabelNhit      = 0x00000010,   // nh
};


class AgedWindow : public PImageWindow,
                   public PListener, 
                   public PSpeaker,
                   public PMenuHandler
{
public:
                    AgedWindow(int load_settings=1);
    virtual         ~AgedWindow();

    virtual void    UpdateSelf();
    virtual void    DoMenuCommand(int anID);
    virtual int     CheckMenuCommand(int anID, int flags);
    virtual void    Listen(int message, void *dataPt);
    virtual void    SetTitle(char *str=NULL);
    
    void            CreateWindow(int anID);
    void            ShowWindow(int id);
    MenuList    *   GetPopupMenuItem(int id);
    void            SaveResources(int force_save=0);
    
    void            LabelFormatChanged();
    TextSpec      * GetLabelText();
    int             GetLabelHeight()        { return mLabelHeight;  }
    long            GetLabelFlags()         { return mLabelFlags;   }
    
    void            AboutAged();
    
    static long     BuildLabelString(ImageData *data, TextSpec *aTextOut,
                                     char *aLabelFormat, char *aBuffer);

private:
    void            SetHitMaskMenuToggles();
    void            MakeLabel();
    void            SetLabelDirty();
    void            WarnQuit();
    
    static void     SetupSum(ImageData *data);
    static int      GetPrecision(char *fmt, int def_prec);
    static void     DestroyDialogProc(Widget w, Widget **dialogPtr, caddr_t call_data);
    static void     CancelProc(Widget w, Widget aShell, caddr_t call_data);
    static void     WarnCancelProc(Widget w, AgedWindow *win, caddr_t call_data);
    static void     WarnOKProc(Widget w, AgedWindow *win, caddr_t call_data);
    static void     WarnDestroyProc(Widget w, AgedWindow *win, caddr_t call_data);

    char            mLabelString[kLabelSize];// label for this event
    TextSpec        mLabelText[kMaxLabelLines];
    int             mLabelHeight;       // pixel height of label
    long            mLabelFlags;        // label flags
    int             mLabelDirty;        // non-zero if label needs remaking
    int             mPrintType;
    
    Widget          disp_text;
    Widget          aboutbox;           // about box
    Widget          mWarnDialog;        // warning dialog
};


#endif // __AgedWindow_h__
