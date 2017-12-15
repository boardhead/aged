//==============================================================================
// File:        AgedWindow.cxx
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <Xm/Form.h>
#include <Xm/ScrollBar.h>
#include <Xm/RowColumn.h>
#include <Xm/MessageB.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/FileSB.h>
#include <X11/StringDefs.h>
#include "AgedWindow.h"
#include "AgedImage.h"
#include "Aged.h"
#include "PResourceManager.h"
#include "PHitInfoWindow.h"
#include "PEventInfoWindow.h"
#include "PEventControlWindow.h"
#include "PWaveformWindow.h"
#include "PPrintWindow.h"
#include "PColourWindow.h"
#include "PSettingsWindow.h"
#include "PEventHistogram.h"
#include "PMapImage.h"
#include "PUtils.h"
#include "aged_version.h"
#include "menu.h"

extern char *sFilePath; /* alternate search path for data files */

// --------------------------------------------------------------------------------------
// Main menu definitions
//
static MenuStruct file_menu[] = {
    { "Event Control...",   0,   XK_E,  EVT_NUM_WINDOW,     NULL, 0, 0},
    { "Next Event",         '>', XK_N,  IDM_NEXT_EVENT,     NULL, 0, 0},
    { "Clear Event",        'l', XK_l,  IDM_CLEAR_EVENT,    NULL, 0, 0},
    { NULL,                 0,   0,     0,                  NULL, 0, 0},
    { "Next Space Point",   '+', XK_x,  IDM_NEXT_SPCPT,     NULL, 0, 0},
    { "Prev Space Point",   '-', XK_v,  IDM_PREV_SPCPT,     NULL, 0, 0},
    { NULL,                 0,   0,     0,                  NULL, 0, 0},
    { "Print Image...",     'p', XK_P,  IDM_PRINT_IMAGE,    NULL, 0, 0},
    { "Print Window...",    0,   XK_W,  IDM_PRINT_WINDOW,   NULL, 0, 0},
    { NULL,                 0,   0,     0,                  NULL, 0, 0},
    { "Settings...",        0,   XK_t,  SETTINGS_WINDOW,    NULL, 0, 0},
    { "Colors...",          0,   XK_C,  COLOUR_WINDOW,      NULL, 0, 0},
    { "Save Settings",      's', XK_S,  IDM_SAVE_SETTINGS,  NULL, 0, 0},
    { NULL,                 0,   0,     0,                  NULL, 0, 0},
    { "About Aged...",      0,   XK_A,  IDM_ABOUT,          NULL, 0, 0},
    { NULL,                 0,   0,     0,                  NULL, 0, 0},
    { "Quit",               'q', XK_Q,  IDM_QUIT,           NULL, 0, 0}
};
static MenuStruct sp_menu[] = {
    { "Errors",             'e', XK_E,  IDM_SP_ERRORS,      NULL, 0, MENU_RADIO | MENU_TOGGLE_ON },
    { "Squares",            'r', XK_r,  IDM_SP_SQUARES,     NULL, 0, MENU_RADIO },
    { "Circles",            'c', XK_C,  IDM_SP_CIRCLES,     NULL, 0, MENU_RADIO },
    { "None",               'n', XK_N,  IDM_SP_NONE,        NULL, 0, MENU_RADIO },
};
static MenuStruct display_menu[] = {
    { "Space Points",       0,   XK_S,  0, sp_menu,         XtNumber(sp_menu),  0},
    { "Underscale",         'u', XK_U,  IDM_CUT_UNDERSCALE, NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},
    { "Overscale",          'v', XK_v,  IDM_CUT_OVERSCALE,  NULL, 0, MENU_TOGGLE | MENU_TOGGLE_ON},
    { NULL,                 0,   0,     0,                  NULL, 0, 0},
    { "Detector",           'd', XK_D,  IDM_DETECTOR,       NULL, 0, MENU_TOGGLE},
    { "Fit",                'f', XK_F,  IDM_FIT,            NULL, 0, MENU_TOGGLE},
    { NULL,                 0,   0,     0,                  NULL, 0, 0},
    { "White Backgnd",      'b', XK_B,  IDM_WHITE_BKG,      NULL, 0, MENU_TOGGLE},
    { "Greyscale",          'g', XK_G,  IDM_GREYSCALE,      NULL, 0, MENU_TOGGLE},
};
static MenuStruct move_menu[] = {
    { "To Home",            'h', XK_H,  IDM_MOVE_HOME,      NULL, 0, 0},
    { "To Axis",            'a', XK_A,  IDM_MOVE_TOP,       NULL, 0, 0},
};
static MenuStruct data_menu[] = {
    { "Hit Time",           't', XK_T,  IDM_TIME,           NULL, 0, MENU_RADIO | MENU_TOGGLE_ON},
    { "Pulse Height",       'i', XK_i,  IDM_HEIGHT,         NULL, 0, MENU_RADIO},
    { "XYZ Error",          'x', XK_X,  IDM_ERROR,          NULL, 0, MENU_RADIO},
    { NULL,                 0,   0,     0,                  NULL, 0, 0},
    { "Wire Number",        0,   XK_W,  IDM_DISP_WIRE,      NULL, 0, MENU_RADIO},
    { "Pad Number",         0,   XK_P,  IDM_DISP_PAD,       NULL, 0, MENU_RADIO},
};
static MenuStruct window_menu[] = {
    { "Event Info",         0,   XK_E,  EVT_INFO_WINDOW,    NULL, 0, 0},
    { "Space Point",        0,   XK_S,  HIT_INFO_WINDOW,    NULL, 0, 0},
    { "Histogram",          0,   XK_i,  HIST_WINDOW,        NULL, 0, 0},
    { "Waveforms",          0,   XK_W,  WAVE_WINDOW,        NULL, 0, 0},
    { "Projections",        0,   XK_P,  PROJ_WINDOW,        NULL, 0, 0},
};
static MenuStruct main_menu[] = {
    { "File",               0,   0,     0, file_menu,       XtNumber(file_menu),    0},
    { "Move",               0,   0,     0, move_menu,       XtNumber(move_menu),    0},
    { "Display",            0,   0,     0, display_menu,    XtNumber(display_menu), 0},
    { "Data",               0,   0,     IDM_DATA_MENU, data_menu, XtNumber(data_menu),  0},
    { "Windows",            0,   0,     0, window_menu,     XtNumber(window_menu),  0},
};

//======================================================================================
// AgedWindow constructor
//
AgedWindow::AgedWindow(int load_settings)
            : PImageWindow(NULL)
{
    Arg     wargs[10];
    int     n;
    unsigned int i;
    extern char *g_argv[];
    
    mLabelHeight = 0;
    mLabelDirty = 0;
    mPrintType = kPrintImage;
    mLabelText[0].font = NULL;
#ifdef ANTI_ALIAS
    mLabelText[0].xftFont = NULL;
#endif
    mLabelText[0].string = NULL;
    mWarnDialog = NULL;
    
    /* create new imagedata structure */
    ImageData *data = new ImageData;
    memset(data,0,sizeof(ImageData));
    mData = data;
/*
** Create top level widget containing a form widget
*/
    data->argv = *g_argv;
    
#ifdef NO_MAIN
    int was_app = 1;
#else   
    int was_app = (PResourceManager::sResource.the_app != 0);
#endif
    /* initialize the application */
    PResourceManager::InitApp();
    
    /* create our top level widget */
    n = 0;
    XtSetArg(wargs[n], XmNx, 20); ++n;
    XtSetArg(wargs[n], XmNy, 20); ++n;
    XtSetArg(wargs[n], XmNminWidth, 100); ++n;
    XtSetArg(wargs[n], XmNminHeight, 100); ++n;
    XtSetArg(wargs[n], XmNwidth, 600); ++n;
    XtSetArg(wargs[n], XmNheight, 600); ++n;
    XtSetArg(wargs[n], XmNtitle, "ALPHA-g Event Display"); ++n;

    if (was_app) {
        XtSetArg(wargs[n], XmNx, 100); ++n;
        XtSetArg(wargs[n], XmNy, 100); ++n;
        // create transient shell if this isn't the application window
        // (must be called "Aged" to load resources properly)
        data->toplevel = CreateShell("Aged",NULL,wargs,n);
    } else {
        // create application shell
        data->toplevel = XtAppCreateShell(NULL,"Aged",applicationShellWidgetClass,
                        PResourceManager::sResource.display,wargs,n);
    }

    PResourceManager::InitResources(data->toplevel);
/*
** Initialize our ImageData
*/
    initData(data, load_settings);

    SetShell(data->toplevel);

    /* create main pane */
    SetMainPane(XtCreateManagedWidget("imageForm",xmFormWidgetClass,data->toplevel,NULL,0));
/*
** Create Menubar
*/
    CreateMenu(NULL,main_menu,XtNumber(main_menu),this);
    
    // subsequent main windows do not quit the program, so change the label
    if (was_app) {
        GetMenu()->SetLabel(IDM_QUIT,"Close Display");
    }
    
    SetMainWindow();        // make this a main window

    PImageCanvas *pimage = new AgedImage(this);
    pimage->CreateCanvas("canvas", kScrollAllMask);

    /* save a pointer to this main window */
    data->mMainWindow = this;
    
    /* listen to resource changed messages */
    PResourceManager::sSpeaker->AddListener(this);
    
    /* listen for cursor messages */
    data->mSpeaker->AddListener(this);
    
    /* setup menus from our resources */
    if (data->dataType)   SelectMenuItem(IDM_TIME + data->dataType);
    if (data->spType)     SelectMenuItem(IDM_SP_ERRORS + data->spType);
    
    SetHitMaskMenuToggles();

    GetMenu()->SetToggle(IDM_DETECTOR,       data->show_detector);
    GetMenu()->SetToggle(IDM_FIT,            data->show_fit);
    
    GetMenu()->SetToggle(IDM_WHITE_BKG,     (data->image_col & kWhiteBkg) != 0);
    GetMenu()->SetToggle(IDM_GREYSCALE,     (data->image_col & kGreyscale) != 0);

    if (data->show_label) {
        LabelFormatChanged();       // initialize label string
    }
    
    // call necessary functions required by current menu items
    if (data->show_detector) sendMessage(data, kMessageDetectorChanged);

    /* show the window */
    Show();
    
    /* open any other windows specified by settings */
    if (sMainWindow == this) {
        int open_windows[2];
        int n, bitnum;
        open_windows[0] = data->open_windows;
        open_windows[1] = data->open_windows2;
        if (open_windows[0] > 0 || open_windows[1] > 0) {
            // open windows by menu item number
            for (i=0; i<XtNumber(window_menu); ++i) {
                n = i / 31; // 31 useable bits in each mask
                if (i >= 2) continue;
                bitnum = i - n * 31;
                if (open_windows[n] & (1UL << bitnum)) {
                    ShowWindow(window_menu[i].id);
                }
            }
        } else if (open_windows[0] < 0 || open_windows[1] < 0) {
            // open windows by window ID
            open_windows[0] = -open_windows[0];
            open_windows[1] = -open_windows[1];
            for (i=1; i<NUM_WINDOWS; ++i) {
                n = i / 31; // 31 useable bits in each mask
                bitnum = i - n * 31;
                if (open_windows[n] & (1UL << bitnum)) {
                    ShowWindow(i);
                }
            }
        }
    }
}

AgedWindow::~AgedWindow()
{
    if (sMainWindow == this) {
        // only save resources on close of base main window
        SaveResources();

        // this is the base Aged window -- delete all other main windows
        while (mNextMainWindow) {
            delete mNextMainWindow;
        }
        // reset the base window pointer
        sMainWindow = NULL;
    } else {
        // this is not the base window -- remove from linked list
        PWindow **xwin_pt = &sMainWindow;
        while (*xwin_pt) {
            if ((*xwin_pt)->mNextMainWindow == this) {
                (*xwin_pt)->mNextMainWindow = mNextMainWindow;  // remove this from list
                break;
            }
            xwin_pt = &(*xwin_pt)->mNextMainWindow;
        }
    }
    
    // free all allocated memory in ImageData struct (except this main window)
    freeData(mData);
}

// add run:event numbers to window title
void AgedWindow::SetTitle(char *str)
{
    char buff[256];

    if (!str) {
        ImageData   *data = GetData();
        char        *pt = GetTitle();
        
        strncpy(buff, pt, 128);
        pt = strchr(buff, '[');                     /* find old run:event */
        if (pt > buff+1) --pt;  
        else pt = buff + strlen(buff);              /* pt points to end of title */
        if (data->agEvent) {
            sprintf(pt,data->hex_id ? " [Run %ld:0x%lx]" : " [Run %ld:%ld]",
                data->run_number, data->event_id);
        } else *pt = 0;
        str = buff;
    }
    PWindow::SetTitle(str);     /* set new title */
}

void AgedWindow::Listen(int message, void *dataPt)
{
    ImageData   *data = GetData();
    
    switch (message) {
        case kMessageEventCleared:
            mLabelText[0].font = NULL;
#ifdef ANTI_ALIAS
            mLabelText[0].xftFont = NULL;
#endif
            mLabelText[0].string = NULL;
            mLabelFlags = 0;
            SetTitle();
            break;
        case kMessageNewEvent:
            if (data->show_label) {
                SetLabelDirty();
            }
            SetTitle();
            break;
        case kMessageTimeFormatChanged:
            // make new label if it shows time or date
            if (data->show_label && (mLabelFlags & (kLabelTime | kLabelDate))) {
                SetLabelDirty();
            }
            break;
        case kMessageEvIDFormatChanged:
            // make new label if it shows EvID
            if (data->show_label && (mLabelFlags & kLabelEvID)) {
                SetLabelDirty();
            }
            SetTitle();
            break;
        case kMessageSmoothTextChanged:
            LabelFormatChanged();
            break;
        case kMessageHitsChanged:
            if (data->show_label && (mLabelFlags & kLabelNhit)) {
                SetLabelDirty();
            }
            break;
        case kMessageResourceColoursChanged:
            data->image_col = PResourceManager::sResource.image_col;
            GetMenu()->SetToggle(IDM_WHITE_BKG, (data->image_col & kWhiteBkg) != 0);
            GetMenu()->SetToggle(IDM_GREYSCALE, (data->image_col & kGreyscale) != 0);
            sendMessage(data, kMessageColoursChanged);
            break;
    }
}

// update AgedResource's and save to file
void AgedWindow::SaveResources(int force_save)
{
    int         i;
    ImageData   *data = mData;
    
    // save main window position
    SaveWindowData();
    
    int open_windows = 0;   // initialize open window mask
    int open_windows2 = 0;
    
    // save all other window positions and calculate open_windows bitmask
    for (i=0; i<NUM_WINDOWS; ++i) {
        if (data->mWindow[i]) {
            // save window data for any windows remaining open
            data->mWindow[i]->SaveWindowData();
            // calculate open_windows (indexed to menu entries)
            if (i<31) {
                open_windows |= (1 << i);
            } else {
                open_windows2 |= (1 << (i-31));
            }
        }
    }
        
    // update necessary AgedResource settings before writing to file
    data->version           = AGED_VERSION;
    data->open_windows      = -open_windows;    // negative to indicate IDM_ indexing
    data->open_windows2     = -open_windows2;
    data->dataType          = data->wDataType - IDM_TIME;
    data->spType            = data->wSpStyle - IDM_SP_ERRORS;
    data->projType          = data->wProjType - IDM_PROJ_RECTANGULAR;
    data->shapeOption       = data->wShapeOption - IDM_HIT_SQUARE;
    data->label_format_pt   = data->label_format;
    
    // send message indicating we are about to write our settings
    sendMessage(data, kMessageWillWriteSettings);
    
    // save settings to resource file
    PResourceManager::WriteSettings(data, force_save);
    
    // send message indicating we are done writing settings
    sendMessage(data, kMessageWriteSettingsDone);
}

void AgedWindow::UpdateSelf()
{
    // count number of displayed hits and save to data->num_disp
    ImageData *data = GetData();
    int n, num_disp = 0;
    if ((n = data->hits.num_nodes) != 0) {
        HitInfo *hi = data->hits.hit_info;
        long bit_mask = data->bit_mask;
        for (int i=0; i<n; ++i,++hi) {
            if (!(hi->flags & bit_mask)) ++num_disp;
        }
    }
    data->num_disp = num_disp;

    // this must be done after calculating num_disp,
    // so the labels will update correctly - PH 05/23/00
    PImageWindow::UpdateSelf();
}

// LabelFormatChanged - called whenever the image label format changes
// - calculates the label height in pixels (obtained through GetLabelHeight())
// - sets label dirty, forcing rebuilding of label on next GetLabelText() call
void AgedWindow::LabelFormatChanged()
{
    int     fontNum;
    char    *label_format = mData->label_format;
    
    mLabelHeight = 0;   // initialize label height
    
    // calculate the label height
    if (*label_format) {
        // initialize pointer to next big-font specification
        char *next_big = strstr(label_format,"%+");
        int fontHeight[2];
#ifdef ANTI_ALIAS
        if ((mData->smooth & kSmoothText) && mData->xft_label_font && mData->xft_label_big_font) {
            fontHeight[0] = mData->xft_label_font->ascent + mData->xft_label_font->descent;
            fontHeight[1] = mData->xft_label_big_font->ascent + mData->xft_label_big_font->descent;
        } else {
#endif
            fontHeight[0] = mData->label_font->ascent + mData->label_font->descent;
            fontHeight[1] = mData->label_big_font->ascent + mData->label_big_font->descent;
#ifdef ANTI_ALIAS
        }
#endif
        for (;;) {
            // look for next newline spec
            label_format = strstr(label_format, "%/");
            if (!label_format) {
                // no more newlines -- add height of this line
                if (next_big) fontNum = 1;
                else          fontNum = 0;
                mLabelHeight += fontHeight[fontNum];
                break;
            }
            // skip over newline specification
            label_format += 2;
            
            if (next_big && next_big<label_format) {
                // this line contained a big-font specification
                fontNum = 1;
                // look for next big-font spec
                next_big = strstr(label_format,"%+");
            } else {
                fontNum = 0;
            }
            // add the height of this label line
            mLabelHeight += fontHeight[fontNum];
        }
    } else {
        mLabelFlags = 0;
    }
    SetLabelDirty();    // the label needs to be remade
}

int AgedWindow::GetPrecision(char *fmt, int def_prec)
{
    unsigned    prec;
    char        *pt = strchr(fmt,'.');
    
    if (pt) {
        prec = (unsigned)atoi(pt+1);
        if (prec > 9) prec = 9; // limit to 9 decimal places (must not do more)
    } else {
        prec = def_prec;
    }
    return((int)prec);
}

// SetLabelDirty - Set flag to indicate label needs rebuilding
// - also sends message indicating that the label has changed
void AgedWindow::SetLabelDirty()
{
    mLabelDirty = 1;
    sendMessage(mData, kMessageLabelChanged);
}

// BuildLabelString [static] - make label string for specified event
//
// aTextOut - pointer to TextSpec array of kMaxLabelLines entries (may be NULL)
// aLabelFormat - pointer to label format string
// aBuffer - pointer to buffer to save output string (kLabelSize bytes long)
//
long AgedWindow::BuildLabelString(ImageData *data, TextSpec *aTextOut,
                                    char *aLabelFormat, char *aBuffer)
{
    int             i, len;
    char          * pt = aBuffer;
    char          * last = aBuffer + kLabelSize - 32;
    char          * src = aLabelFormat;
    char          * fmtPt = NULL;
    const short     kFormatSize = 128;
    char            format[kFormatSize];
//  struct tm     * tms = NULL;
    long            labelFlags;
    
    // these label formats are in the same order as the ELabelFlags bits
    static char *labelFormats[] = { "rn","ev","ti","da","nh",NULL,NULL,NULL,
                                    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
                                    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
                                    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };
    
    labelFlags = 0; // initialize label flags
    
    // initialize label text for first line
    int lines = 0;
    if (aTextOut) {
        aTextOut[0].font = PResourceManager::sResource.label_font;
#ifdef ANTI_ALIAS
        aTextOut[0].xftFont = PResourceManager::sResource.xft_label_font;
#endif
        aTextOut[0].string = aBuffer;
    }
    
    for (char ch=*src; ; ch=*(++src)) {
        if (fmtPt) {
            if (!ch) {
                *fmtPt = '\0';
                pt += sprintf(pt,"%s",format);  // add malformed format to string
                break;
            }
            if ((ch>='0' && ch<='9') || ch=='.' || ch=='-') {
                *(fmtPt++) = ch;
                // abort if format is too large
                if (fmtPt - format >= (kFormatSize-5)) {
                    *pt = '\0';
                    break;  // stop now
                }
            } else if (ch == '%') { // '%' character specified by "%%"
                *(pt++) = ch;
                fmtPt = NULL;
            } else if (ch == '/') { // CR specified by "%/"
                // terminate this line since our string is a compound string
                // -- one null-terminated string for each line
                *(pt++) = '\0';
                fmtPt = NULL;
                // check for too many lines
                if (lines >= kMaxLabelLines-2) break;
                // initialize label text for next line
                ++lines;
                if (aTextOut) {
                    aTextOut[lines].font = PResourceManager::sResource.label_font;
#ifdef ANTI_ALIAS
                    aTextOut[lines].xftFont = PResourceManager::sResource.xft_label_font;
#endif
                    aTextOut[lines].string = pt;
                }
            } else if (ch == '+') { // big font specified by "%+"
                if (aTextOut) {
                    aTextOut[lines].font = PResourceManager::sResource.label_big_font;
#ifdef ANTI_ALIAS
                    aTextOut[lines].xftFont = PResourceManager::sResource.xft_label_big_font;
#endif
                }
                fmtPt = NULL;
            } else if (!*(++src)) { // skip second letter of format type
                *(fmtPt++) = ch;
                *fmtPt = '\0';
                pt += sprintf(pt,"%s",format);  // add malformed format to string
                break;
            } else {
                ch = tolower(ch);
                char ch2 = tolower(*src);
                long flag;
                for (i=0; ; ++i) {
                    if (!labelFormats[i]) {
                        flag = 0;
                        break;
                    }
                    if (labelFormats[i][0]==ch && labelFormats[i][1]==ch2) {
                        flag = (1L << i);
                        break;
                    }
                }
                *fmtPt = '\0';  // null terminate format specification
                labelFlags |= flag; // update our label flags
                switch (flag) {
                    case 0L:
                        // add malformed format to string
                        pt += sprintf(pt,"%s%c%c",format,ch,ch2);
                        break;      // no match
                    case kLabelRun:
                        pt += sprintf(pt,"%ld",(long)data->run_number);
                        break;
                    case kLabelEvID:
                        if (data->hex_id) {
                            len = GetPrecision(format, 6);
                            pt += sprintf(pt,"0x%.*lx",len,(long)data->event_id);
                        } else {
                            pt += sprintf(pt,"%ld",(long)data->event_id);
                        }
                        break;
                    case kLabelNhit:
                        pt += sprintf(pt,"%ld",(long)data->hits.num_nodes);
                        break;
/*                  case kLabelTime:
                        if (!tms) tms = getTms(data->event_time, data->time_zone);
                        if (!data->event_time) {
                            tms->tm_hour = tms->tm_min = tms->tm_sec = 0;
                        }
                        pt += sprintf(pt,"%.2d:%.2d:%.2d",
                                    tms->tm_hour, tms->tm_min, tms->tm_sec);
                        len = GetPrecision(format, 0);
                        if (len) {
                            double dec = data->event_time - (long)data->event_time;
                            for (i=0; i<len; ++i) {
                                dec *= 10.0;
                            }
                            dec += 0.5; // round up to nearest long
                            pt += sprintf(pt,".%.*ld",len,(long)dec);
                        }
                        if (data->time_zone == kTimeZoneUTC) {
                            pt += sprintf(pt," UTC");
                        } else if (data->time_zone == kTimeZoneLocal) {
                            pt += sprintf(pt," Loc");
                        }
                        break;
                    case kLabelDate:
                        if (data->event_time) {
                            if (!tms) tms = getTms(data->event_time, data->time_zone);
                            pt += sprintf(pt,"%.2d/%.2d/%d",
                                        tms->tm_mon+1, tms->tm_mday, tms->tm_year+1900);
                        } else {
                            pt += sprintf(pt,"00/00/0000");
                        }
                        break;
*/
                }
                fmtPt = NULL;   // no longer in format statement
            }
        } else if (!ch) {
            // we done our format string
            *pt = ch;   // null terminate string
            break;      // done
        } else if (ch == '%') {
            fmtPt = format;
            *(fmtPt++) = ch;
        } else if (pt >= last) {
            // stop now if our label is getting too big
            *pt = '\0';
            break;
        } else {
            *(pt++) = ch;   // copy to label string
        }
    }
    // add terminator for label text
    ++lines;
    if (aTextOut) {
        aTextOut[lines].font = NULL;
#ifdef ANTI_ALIAS
        aTextOut[lines].xftFont = NULL;
#endif
        aTextOut[lines].string = NULL;
    }
    
    return(labelFlags);     // return label flags
}

// GetLabelText - make label if necessary and return pointer to the label text array
// - array is terminated by a string=NULL entry in the TextSpec array
// (must be called before GetLabelFlags() if label is dirty)
TextSpec * AgedWindow::GetLabelText()
{
    // only rebuild label if dirty flag is set
    if (mLabelDirty) {
        mLabelDirty = 0;    // reset label dirty flag
        mLabelFlags = BuildLabelString(mData, mLabelText, mData->label_format, mLabelString);
    }
    return mLabelText;
}

void AgedWindow::AboutAged()
{
    Widget      label_text;
    Arg         warg;
    ImageData * data = GetData();
    char      * aboutStr1 = "Aged version " AGED_VERSION "\n"
                            " \n"
                            "ALPHA-g Event Display Program\n"
                            " \n"
                            "Copyright \xa9 2017\n"
                            "Philip Harvey, Queen's University\n"
                            " \n";

    XtSetArg(warg, XmNtitle, "About Aged");
    aboutbox = XmCreateMessageDialog(data->toplevel, "agedAbout", &warg, 1);
    
    XtUnmanageChild(XmMessageBoxGetChild(aboutbox,XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(aboutbox,XmDIALOG_HELP_BUTTON));

    label_text = XmMessageBoxGetChild(aboutbox,XmDIALOG_MESSAGE_LABEL);
    XtSetArg(warg, XmNalignment,XmALIGNMENT_CENTER);
    XtSetValues(label_text, &warg, 1);

    XmString str1 = XmStringCreateLtoR(aboutStr1, XmFONTLIST_DEFAULT_TAG);
    XtSetArg(warg, XmNlabelString, str1);
    XtSetValues(label_text, &warg, 1);
    XmStringFree(str1);

    XtAddCallback(aboutbox,XtNdestroyCallback,(XtCallbackProc)DestroyDialogProc,&aboutbox);
}

MenuList *AgedWindow::GetPopupMenuItem(int id)
{
    MenuList    *ms = mMenu->FindMenuItem(id);
    if (!ms) {
        Printf("Could not find popup menu struct %d!\n",id);
        exit(1);
    }
    return(ms);
}

void AgedWindow::SetHitMaskMenuToggles()
{
    ImageData   *data = GetData();
    
    GetMenu()->SetToggle(IDM_CUT_UNDERSCALE, !(data->bit_mask & HIT_UNDERSCALE));
    GetMenu()->SetToggle(IDM_CUT_OVERSCALE,  !(data->bit_mask & HIT_OVERSCALE));
}

// ShowWindow - show the specified Aged window
void AgedWindow::ShowWindow(int id)
{
    // handle window commands
    PWindow *win = GetData()->mWindow[id];
    
    if (win) {
        win->Raise();   // raise window to top
    } else {
        CreateWindow(id);
    }
}

void AgedWindow::WarnQuit()
{
    // open warning dialog
    if (mWarnDialog) {
        XtDestroyWidget(mWarnDialog);
        mWarnDialog = NULL;
    }
    XmString    str;
    Arg         wargs[10];
    int         n;
    str = XmStringCreateLocalized("Really Quit Aged?  ");
    n = 0;
    XtSetArg(wargs[n], XmNtitle, "Quit Aged"); ++n;
    XtSetArg(wargs[n], XmNmessageString, str); ++n;
    XtSetArg(wargs[n], XmNdefaultButtonType, XmDIALOG_OK_BUTTON); ++n;
    mWarnDialog = XmCreateWarningDialog(GetShell(), "agedWarn",wargs,n);
    XmStringFree(str);  // must free the string
    XtUnmanageChild(XmMessageBoxGetChild(mWarnDialog,XmDIALOG_HELP_BUTTON));
/*  // change the "OK" label to "Quit"
    Widget but = XmMessageBoxGetChild(mWarnDialog,XmDIALOG_OK_BUTTON);
    n = 0;
    str = XmStringCreateLocalized("Quit");
    XtSetArg(wargs[n], XmNlabelString, str); ++n;
    XtSetValues(but, wargs, n);
    XmStringFree(str);
*/
    XtAddCallback(mWarnDialog,XmNcancelCallback,(XtCallbackProc)WarnCancelProc,this);
    XtAddCallback(mWarnDialog,XmNokCallback,(XtCallbackProc)WarnOKProc,this);
    XtAddCallback(mWarnDialog,XtNdestroyCallback,(XtCallbackProc)WarnDestroyProc,this);
    XtManageChild(mWarnDialog);
}

//---------------------------------------------------------------------------------
// Assorted callbacks
//
void AgedWindow::WarnCancelProc(Widget w, AgedWindow *win, caddr_t call_data)
{
    // delete the warning dialog
    XtDestroyWidget(win->mWarnDialog);
    win->mWarnDialog = NULL;
}

void AgedWindow::WarnOKProc(Widget w, AgedWindow *win, caddr_t call_data)
{
    XtDestroyWidget(win->mWarnDialog);
    win->mWarnDialog = NULL;
    // quit Aged
    deleteData(win->GetData());
}

// the warning dialog was destroyed
void AgedWindow::WarnDestroyProc(Widget w, AgedWindow *win, caddr_t call_data)
{
    // must verify that it is the current dialog being destroyed
    // (could a delayed callback from one we destroyed ourself already)
    if (win->mWarnDialog == w) {
        win->mWarnDialog = NULL;
    }
}
void AgedWindow::DestroyDialogProc(Widget w, Widget **dialogPtr, caddr_t call_data)
{
    /* must zero our pointer since the dialog is gone */
    *dialogPtr = NULL;
}

void AgedWindow::CancelProc(Widget w, Widget aShell, caddr_t call_data)
{
    XtDestroyWidget(aShell);
}

//-----------------------------------------------------------------------------------
// CreateWindow - create a sub window with the specified ID
//
void AgedWindow::CreateWindow(int anID)
{
    int         n;
    int         min_width, min_height;
    Arg         wargs[20];
    Widget      window, w;
    ImageData   *data = GetData();
    PImageWindow *pwin = NULL;
    char        buff[128];

    switch (anID) {
        
      case EVT_NUM_WINDOW:  // Create Event control window
        data->mWindow[anID] = new PEventControlWindow(data);        
        break;
      case SETTINGS_WINDOW: // Create Event control window
        data->mWindow[anID] = new PSettingsWindow(data);        
        break;

      case HIT_INFO_WINDOW: // Create Hit info window
        data->mWindow[anID] = new PHitInfoWindow(data);
        break;

      case EVT_INFO_WINDOW: // Create Event info window
        data->mWindow[anID] = new PEventInfoWindow(data);
        break;

      case COLOUR_WINDOW: // Create colour window
        data->mWindow[anID] = new PColourWindow(data);
        break;

      case PROJ_WINDOW: // Create Projection window
        sprintf(buff,"%s Projection", data->projName);
        n = 0;
        XtSetArg(wargs[n], XmNtitle, buff); ++n;
        XtSetArg(wargs[n], XmNx, 250); ++n;
        XtSetArg(wargs[n], XmNy, 250); ++n;
        XtSetArg(wargs[n], XmNminWidth, 100); ++n;
        XtSetArg(wargs[n], XmNminHeight, 100); ++n;
        window = CreateShell("tpPop",data->toplevel,wargs,n);
        n = 0;
        XtSetArg(wargs[n], XmNwidth, 600); ++n;
        XtSetArg(wargs[n], XmNheight, 350); ++n;
        w = XtCreateManagedWidget("imageForm",xmFormWidgetClass,window,wargs,n);
        data->mWindow[anID] = pwin = new PImageWindow(data,window,w);
        // install a map image in the new window
        (void)new PMapImage(pwin);
        break;

      case WAVE_WINDOW:
        data->mWindow[anID] = new PWaveformWindow(data);
        break;

      case HIST_WINDOW: // Create histogram window
        n = 0;
        XtSetArg(wargs[n], XmNtitle, "Event Histogram"); ++n;
        XtSetArg(wargs[n], XmNx, 225); ++n;
        XtSetArg(wargs[n], XmNy, 225); ++n;
        XtSetArg(wargs[n], XmNminWidth, 200); ++n;
        XtSetArg(wargs[n], XmNminHeight, 100); ++n;
        window = CreateShell("ehPop",data->toplevel,wargs,n);
        n = 0;
        XtSetArg(wargs[n], XmNwidth, 400); ++n;
        XtSetArg(wargs[n], XmNheight, 250); ++n;
        w = XtCreateManagedWidget("imageForm",xmFormWidgetClass,window,wargs,n);
        data->mWindow[anID] = pwin = new PImageWindow(data,window,w);
        // install an event histogram in the new window
        (void)new PEventHistogram(pwin);
        break;

      case PRINT_WINDOW:    // Create print window
        data->mWindow[anID] = new PPrintWindow(data, (EPrintType)mPrintType);
        break;

      default:
        return;
    }
    if (data->mWindow[anID]) {
        data->mWindow[anID]->Show();        // show the window
        // resize necessary windows
        if (!data->mWindow[anID]->WasResized()) {
            switch (anID) {
                case PRINT_WINDOW:
                    n = 0;
                    XtSetArg(wargs[n], XmNminWidth, &min_width); ++n;
                    XtSetArg(wargs[n], XmNminHeight, &min_height); ++n;
                    XtGetValues(data->mWindow[anID]->GetShell(), wargs, n);
                    data->mWindow[anID]->Resize(min_width, min_height);
                    break;
            }
        }
    }
}

// CheckMenuCommand - check state of a menu command prior to opening of the menu
int AgedWindow::CheckMenuCommand(int anID, int flags)
{
    switch (anID) {
        case IDM_DETECTOR:
        case IDM_FIT:
        case IDM_MOVE_TOP:
            // can only do these commands with a 3-D geometry
            //PMenu::SetEnabled(&flags, Is3d(GetData()->wGeo));
            break;
    }
    return(flags);
}

void aged_next(ImageData *data, int direction)
{
    /* step forward in real time */
    PEventControlWindow::SetEventFilter(data);
    setTriggerFlag(data,TRIGGER_SINGLE);
}

//--------------------------------------------------------------------------------------
// DoMenuCommand
//
void AgedWindow::DoMenuCommand(int anID)
{
    ImageData   *data = GetData();
                
    if (anID < NUM_WINDOWS) {   // Is this a window menu command?
    
        ShowWindow(anID);       // Yes -- show the window
        
    } else switch (anID) {
    
        case IDM_ABOUT:
            if (aboutbox) {
                XtUnmanageChild(aboutbox);
            } else {
                AboutAged();
            }
            XtManageChild(aboutbox);
            break;
            
// no Print Window feature if we can't fork a process
#ifndef NO_FORK
        case IDM_PRINT_WINDOW:
            mPrintType = kPrintWindow;
            if (data->mWindow[PRINT_WINDOW]) {
                ((PPrintWindow *)data->mWindow[PRINT_WINDOW])->SetPrintType((EPrintType)mPrintType);
            } else {
                CreateWindow(PRINT_WINDOW);
            }
            break;
#endif // NO_FORK
            
        case IDM_PRINT_IMAGE:
            mPrintType = kPrintImage;
            if (data->mWindow[PRINT_WINDOW]) {
                ((PPrintWindow *)data->mWindow[PRINT_WINDOW])->SetPrintType((EPrintType)mPrintType);
            } else {
                CreateWindow(PRINT_WINDOW);
            }
            break;
            
        case IDM_SAVE_SETTINGS:
            SaveResources(1);
            break;
            
        case IDM_NEXT_EVENT:
            aged_next(data, 1);
            break;
            
        case IDM_NEXT_SPCPT:
            if (data->hits.num_nodes) {
                data->cursor_hit += 1;
                if (data->cursor_hit >= data->hits.num_nodes) data->cursor_hit = 0;
                data->cursor_sticky = 1;
                sendMessage(data, kMessageCursorHit);
            }
            break;
            
        case IDM_PREV_SPCPT:
            if (data->hits.num_nodes) {
                data->cursor_hit -= 1;
                if (data->cursor_hit < 0) data->cursor_hit = data->hits.num_nodes-1;
                data->cursor_sticky = 1;
                sendMessage(data, kMessageCursorHit);
            }
            break;
            
//      case IDM_PREV_EVENT:
//          aged_next(data, -1);
//          break;
            
        case IDM_CLEAR_EVENT:
            clearEvent(data);
            break;

        case IDM_QUIT:
            if (PMenu::WasAccelerator()) {
                WarnQuit();
            } else {
                deleteData(data);
                exit(0);
            }
            break;
            
        case IDM_CUT_OVERSCALE:
            data->bit_mask ^= HIT_OVERSCALE;
            sendMessage(data, kMessageHitsChanged);
            break;
        
        case IDM_CUT_UNDERSCALE:
            data->bit_mask ^= HIT_UNDERSCALE;
            sendMessage(data, kMessageHitsChanged);
            break;
            
        case IDM_DETECTOR:
            data->show_detector ^= 1;
            sendMessage(data, kMessageDetectorChanged);
            break;
        
        case IDM_FIT:
            data->show_fit ^= 1;
            sendMessage(data, kMessageFitChanged);
            break;
        
        case IDM_WHITE_BKG:
            PResourceManager::SetColours(data->image_col ^ kWhiteBkg);
            break;
            
        case IDM_GREYSCALE:
            PResourceManager::SetColours(data->image_col ^ kGreyscale);
            break;
            
        case IDM_MOVE_HOME:
            SetToHome();
            break;
            
        case IDM_MOVE_TOP:
            SetToHome(1);
            break;
            
        case IDM_SP_ERRORS:
        case IDM_SP_SQUARES:
        case IDM_SP_CIRCLES:
        case IDM_SP_NONE:
            PMenu::UpdateTogglePair(&data->wSpStyle);
            sendMessage(data,kMessageHitsChanged);
            break;

        case IDM_TIME:
        case IDM_HEIGHT:
        case IDM_ERROR:
        case IDM_DISP_WIRE:
        case IDM_DISP_PAD:
        {
            if (PMenu::UpdateTogglePair(&data->wDataType)) {
                char *str = PMenu::GetLabel((MenuList *)NULL);
                if (str) {
                    XtFree(data->dispName);
                    data->dispName = str;
                }
                // must reset histogram grab to allow scale to change
                PHistImage *hist = PEventHistogram::GetEventHistogram(data);
                if (hist) hist->ResetGrab(0);
                
                /* re-calculate hit values and redisplay all images */
                calcHitVals(data);
                sendMessage(data,kMessageHitsChanged);
            }
        } break;

        default:
            Printf("Unknown menu command ID - %d\n",anID);
            break;
    }
}





