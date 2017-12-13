//
// File:        PWaveformWindow.cxx
//
// Description: Window to display waveform data
//
// Revisions:   2012/03/06 - PH Created (borrowed heavily from PNCDScopeWindow.cxx)
//
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>
#include <math.h>
#include "PWaveformWindow.h"
#include "PHistImage.h"
#include "ImageData.h"
#include "PSpeaker.h"
#include "CUtils.h"
#include "AgFlow.h"

const int   kDirtyEvent     = 0x02;
const int   kDirtyAll       = 0x04;
const int   kShowChannels   = 0x03;     // show only channels 0 and 1 to start

enum {
    kWireHist,      // index for anode wire histogram in Waveform window
    kPadHist,       // index for pad histogram in Waveform window
    kNumHists
};

static MenuStruct channels_menu[] = {
//      { "Add Overlay",      0, XK_A, IDM_WAVE_ADD_OVERLAY,   NULL, 0, 0 },
      { "Clear Overlays",   0, XK_C, IDM_WAVE_CLEAR_OVERLAY, NULL, 0, 0 },
    //{ NULL,               0, 0,       0,                  NULL, 0, 0},
};

static MenuStruct wave_main_menu[] = {
    { "Display",    0, 0,   0, channels_menu, XtNumber(channels_menu), 0 },
};

static char * hist_label[kMaxWaveformChannels] = { "Wire", "Pad", "","","","","","" };

//---------------------------------------------------------------------------
// PWaveformWindow constructor
//
PWaveformWindow::PWaveformWindow(ImageData *data)
           : PImageWindow(data), mLastNum(-1), mChanMask(0)
{
    int     n;
    Arg     wargs[20];

    data->mSpeaker->AddListener(this);
    
    n = 0;
    XtSetArg(wargs[n], XmNtitle, "Waveforms"); ++n;
    XtSetArg(wargs[n], XmNx, 400);             ++n;
    XtSetArg(wargs[n], XmNy, 300);             ++n;
    XtSetArg(wargs[n], XmNminWidth, 200);      ++n;
    XtSetArg(wargs[n], XmNminHeight, 200);     ++n;
    SetShell(CreateShell("wavePop",data->toplevel,wargs,n));
    
    n = 0;
    XtSetArg(wargs[n], XmNwidth, 600);         ++n;
    XtSetArg(wargs[n], XmNheight, 400);        ++n;
    XtSetArg(wargs[n], XmNbackground, data->colour[BKG_COL]); ++n;
    SetMainPane(XtCreateManagedWidget("waveMain", xmFormWidgetClass,GetShell(),wargs,n));

    n = 0;
    XtSetArg(wargs[n],XmNmarginHeight,      1); ++n;
    XtSetArg(wargs[n],XmNleftAttachment,    XmATTACH_FORM); ++n;
    XtSetArg(wargs[n],XmNtopAttachment,     XmATTACH_FORM); ++n;
    XtSetArg(wargs[n],XmNrightAttachment,   XmATTACH_FORM); ++n;
    Widget menu = XmCreateMenuBar(GetMainPane(), "agedMenu" , wargs, n);
    XtManageChild(menu);
    CreateMenu(menu, wave_main_menu, XtNumber(wave_main_menu), this);

//    GetMenu()->SetToggle(IDM_WAVE_SUBT,      data->wave_subt);
//    GetMenu()->SetToggle(IDM_WAVE_SUM,       data->wave_sum);
//    GetMenu()->SetToggle(IDM_WAVE_PULSE,     data->wave_pulse);
//    GetMenu()->SetToggle(IDM_QT_PULSE,       data->qt_pulse);
//    GetMenu()->SetToggle(IDM_WAVE_CURSOR,    data->wave_cursor);

    // add resource labels to channels in menu
/*    for (int i=0; i<kWaveformRsrcNum; ++i) {
        if (!strncmp(data->wave_lbl[i], "Channel ", 8)) continue;
        char buff[256];
        sprintf(buff, "Channel %d - %s", i, data->wave_lbl[i]);
        GetMenu()->SetLabel(IDM_WAVE_0+i, buff);
    }*/

    n = 0;
    XtSetArg(wargs[n], XmNtopAttachment,    XmATTACH_WIDGET);    ++n;
    XtSetArg(wargs[n], XmNtopWidget,        menu);               ++n;
    XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM);      ++n;
    XtSetArg(wargs[n], XmNleftAttachment,   XmATTACH_FORM);      ++n;
    XtSetArg(wargs[n], XmNrightAttachment,  XmATTACH_FORM);      ++n;
    XtSetArg(wargs[n], XmNbackground,       data->colour[BKG_COL]); ++n;
    Widget form = XtCreateManagedWidget("waveForm", xmFormWidgetClass,GetMainPane(),wargs,n);
    
    for (int i=0; i<kMaxWaveformChannels; ++i) {
        n = 0;
        XtSetArg(wargs[n], XmNbackground,           data->colour[BKG_COL]); ++n;
        XtSetArg(wargs[n], XmNleftAttachment,       XmATTACH_FORM);      ++n;
        XtSetArg(wargs[n], XmNrightAttachment,      XmATTACH_FORM);      ++n;
        mChannel[i] = XtCreateManagedWidget(hist_label[i], xmDrawingAreaWidgetClass, form, wargs, n);

        // create histogram
        mHist[i] = new PHistImage(this, mChannel[i], 0);
        mHist[i]->AllowLabel(FALSE);
        mHist[i]->SetScaleLimits(0,15000,10);
        mHist[i]->SetScaleMin(0);
        mHist[i]->SetScaleMax(4096);
        mHist[i]->SetYMin(data->wave_min[i]);
        mHist[i]->SetYMax(data->wave_max[i]);
        mHist[i]->SetStyle(kHistStyleSteps);
        mHist[i]->SetFixedBins();
        mHist[i]->SetPlotCol(WAVEFORM_COL);
//        mHist[i]->SetOverlayCol(SCOPE0_COL);
//        mHist[i]->SetAutoScale(1);
        mHist[i]->SetCursorTracking(1);
//        mHist[i]->SetLabel(hist_label[i]);
    }
    // arrange our channel histogram widgets in the window
    SetChannels(kShowChannels);
        
    SetDirty(kDirtyAll);
}

PWaveformWindow::~PWaveformWindow()
{
    ImageData *data = GetData();
    for (int i=0; i<kMaxWaveformChannels; ++i) {
        // update current Y scale settings
        data->wave_min[i] = mHist[i]->GetYMin();
        data->wave_max[i] = mHist[i]->GetYMax();
        delete mHist[i];
        mHist[i] = NULL;
    }
    SetImage(NULL);     // make sure base class doesn't re-delete our image
}

void PWaveformWindow::Listen(int message, void *message_data)
{
    switch (message) {
        case kMessageNewEvent:
        case kMessageEventCleared:
            for (int i=0; i<kMaxWaveformChannels; ++i) {
                if (mChanMask & (1 << i)) mHist[i]->ClearOverlays();
            }
            SetDirty(kDirtyEvent);
            break;
        case kMessageColoursChanged:
            SetDirty(kDirtyAll);
            for (int i=0; i<kMaxWaveformChannels; ++i) {
                if (mChanMask & (1 << i)) mHist[i]->SetDirty();
            }
            break;
        case kMessageCursorHit:
            // only dirty if this is a different hit
            if (GetData()->cursor_sticky) {
                GetData()->cursor_sticky = 0;
                for (int i=0; i<kMaxWaveformChannels; ++i) {
                    if (mChanMask & (1 << i)) {
                        mHist[i]->AddOverlay();
                    }
                }
                SetDirty(kDirtyEvent);
            } else if (mLastNum != mData->cursor_hit) {
                SetDirty(kDirtyEvent);
            }
            break;
        case kMessageHistScalesChanged: {
            // update current Y scale settings
            for (int i=0; i<kMaxWaveformChannels; ++i) {
                if (mHist[i] == (PHistImage *)message_data) {
                    GetData()->wave_min[i] = mHist[i]->GetYMin();
                    GetData()->wave_max[i] = mHist[i]->GetYMax();
                    break;
                }
            }
        }   break;
    }
}

void PWaveformWindow::DoMenuCommand(int anID)
{
    switch (anID) {

        case IDM_WAVE_ADD_OVERLAY:
            for (int i=0; i<kMaxWaveformChannels; ++i) {
                if (mChanMask & (1 << i)) {
                    mHist[i]->AddOverlay();
                }
            }
            SetDirty(kDirtyEvent);
            break;

        case IDM_WAVE_CLEAR_OVERLAY:
            for (int i=0; i<kMaxWaveformChannels; ++i) {
                if (mChanMask & (1 << i)) {
                    mHist[i]->ClearOverlays();
                    mHist[i]->SetDirty();
                }
            }
            SetDirty(kDirtyEvent);
            break;
    }
}

// set our displayed channels
void PWaveformWindow::SetChannels(int chan_mask)
{
    int       n, i;
    Arg       wargs[20];
//    ImageData *data = GetData();

    if (chan_mask != mChanMask) {
        // count the number of channels displayed
        int chan_count = 0;
        for (i=0; i<kMaxWaveformChannels; ++i) {
            if (chan_mask & (1 << i)) {
                ++chan_count;
            }
        }
        // make necessary attachments and remap displayed channels
        float height = 100.0 / chan_count;
        int count = 0;
//        int num = chan_mask - 1;
        for (i=0; i<kMaxWaveformChannels; ++i) {
            // toggle on/off our channel displays
//            GetMenu()->SetToggle(IDM_WAVE_0 + i, (i == num ? 1 : 0));
            if (chan_mask & (1 << i)) {
                n = 0;
                if (count) {
                    XtSetArg(wargs[n], XmNtopAttachment,    XmATTACH_POSITION);      ++n;
                    XtSetArg(wargs[n], XmNtopPosition,      int(count*height+0.5));  ++n;
                } else {
                    XtSetArg(wargs[n], XmNtopAttachment,    XmATTACH_WIDGET);        ++n;
                    XtSetArg(wargs[n], XmNtopWidget,        GetMenu()->GetWidget()); ++n;
                }
                if (count < chan_count-1) {
                    XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_POSITION);      ++n;
                    XtSetArg(wargs[n], XmNbottomPosition,   int((count+1)*height+0.5)); ++n;
                } else {
                    XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM);          ++n;
                }
                XtSetValues(mChannel[i], wargs, n);
                ++count;
            }
        }
//        GetMenu()->SetToggle(IDM_WAVE_2, chan_mask == 0x03 ? 1 : 0);
        // map necessary channels
        for (i=0; i<kMaxWaveformChannels; ++i) {
            // only map/unmap channels that have changed
/*            if ((chan_mask ^ data->wave_mask) & (1 << i)) {
                // must be careful not to map a widget before it is realized
                if (XtIsRealized(mChannel[i])) {
                    if (chan_mask & (1 << i)) {
                        // show this channel
                        XtSetMappedWhenManaged(mChannel[i], True);
                    } else {
                        // hide this channel
                        XtSetMappedWhenManaged(mChannel[i], False);
                    }
                } else {
                    n = 0;
                    if (chan_mask & (1 << i)) {
                        XtSetArg(wargs[n], XmNmappedWhenManaged, TRUE); ++n;
                    } else {
                        XtSetArg(wargs[n], XmNmappedWhenManaged, FALSE); ++n;
                    }
                    XtSetValues(mChannel[i], wargs, n);
                }
            }*/
        }
        // set our canvas to the first displayed channel (for Print Image...)
        for (i=0; i<kMaxWaveformChannels; ++i) {
            if (chan_mask & (1 << i)) {
                SetImage(mHist[i]);
                break;
            }
        }
        // allow labels only on the last displayed channel
        int allow = TRUE;
        for (i=kMaxWaveformChannels-1; i>=0; --i) {
            if (chan_mask & (1 << i)) {
                mHist[i]->AllowLabel(allow);
                allow = FALSE;
            }
        }
        mChanMask = chan_mask;
    }
}

// UpdateSelf
void PWaveformWindow::UpdateSelf()
{
    int         i;
    ImageData * data = GetData();
    int         hit_num = data->cursor_hit; // current hit number near cursor
    char        buff[128];

#ifdef PRINT_DRAWS
    printf("-updateWaveformWindow\n");
#endif
    mLastNum = hit_num;

    if (IsDirty() & (kDirtyEvent | kDirtyAll)) {

        HitInfo *hi = NULL;
        void * wave[kMaxWaveformChannels] = { 0 };

        if (hit_num >= 0) {
            // get waveforms for the space point at the cursor
            hi = data->hits.hit_info + hit_num;
            AgSignalsFlow *sigFlow = data->sigFlow;
            for (auto it=sigFlow->AWwf.begin(); it!=sigFlow->AWwf.end(); ++it) {
                if (it->i == hi->wire) {
                    wave[kWireHist] = (void *)it->wf;
                    break;
                }
            }
            for (auto it=sigFlow->PADwf.begin(); it!=sigFlow->PADwf.end(); ++it) {
                int pad = TPCBase::TPCBaseInstance()->SectorAndPad2Index(it->sec,it->i);
                if (pad  == hi->pad) {
                    wave[kPadHist] = (void *)it->wf;
                    break;
                }
            }
        }
        // update data for displayed histograms
        for (i=0; i<kMaxWaveformChannels; ++i) {
            if (!(mChanMask & (1 << i))) continue;
            if (!wave[i]) {
                if (mHist[i]->GetDataPt() || mHist[i]->GetOverlayPt()) {
                    mHist[i]->CreateData(0);
                    mHist[i]->SetDirty();
                }
            } else if (IsDirty() & kDirtyEvent) {
                mHist[i]->SetDirty();
                if (i == kWireHist) {
                    const vector<int16_t> *wf = (const vector<int16_t> *)wave[0];
                    mHist[i]->CreateData(wf->size());
                    long *pt = mHist[i]->GetDataPt();
                    if (pt) {
                        mHist[i]->SetScaleLimits(0, wf->size(), 10);
                        for (int i=0; i<wf->size(); ++i) {
                            *pt++ = wf->at(i);
                        }
                    }
                } else if (i == kPadHist) {
                    const vector<int> *wf = (const vector<int> *)wave[1];
                    mHist[i]->CreateData(wf->size());
                    long *pt = mHist[i]->GetDataPt();
                    if (pt) {
                        mHist[i]->SetScaleLimits(0, wf->size(), 10);
                        for (int i=0; i<wf->size(); ++i) {
                            *pt++ = wf->at(i);
                        }
                    }
                }
            }
            // add wire/pad number to histogram label
            char *pt = NULL;
            if (hi && i < kNumHists) {
                if (i==kWireHist) {
                    sprintf(buff, "%s %d", hist_label[i], hi->wire);
                } else {
                    sprintf(buff, "%s %d", hist_label[i], hi->pad);
                }
                pt = buff;
            }
            if (pt) {
                if (!mHist[i]->GetLabel() || strcmp(pt, mHist[i]->GetLabel())) {
                    mHist[i]->SetLabel(pt);
                    mHist[i]->SetDirty();
                }
            } else if (mHist[i]->GetLabel()) {
                mHist[i]->SetLabel(pt);
                mHist[i]->SetDirty();
            }
        }
    }
    // Update necessary histograms
    for (i=0; i<kMaxWaveformChannels; ++i) {
        // update this channel if required and visible
        if (mHist[i]->IsDirty()){ // && (data->wave_mask & (1<<i))) {
            mHist[i]->Draw();
        }
    }
}

