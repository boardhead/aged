//
// File:    	PEventInfoWindow.cxx
//
// Description: Window to display event information in text form
//
// Notes:   	11/11/99 - PH To reduce flickering of text items in this window,
//  			I'm now calling SetStringNow() instead of SetString().  Previously,
//  			the flickering could be so bad that the text could be invisible
//  			most of the time.
//
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <time.h>
#include "ImageData.h"
#include "PEventInfoWindow.h"
#include "PUtils.h"
#include "PSpeaker.h"
#include "TStoreEvent.hh"

//---------------------------------------------------------------------------
// PEventInfoWindow constructor
//
PEventInfoWindow::PEventInfoWindow(ImageData *data)
    			: PWindow(data)
{
    Widget	rc1, rc2;
    int		n;
    Arg		wargs[16];

    mTimeZone = 0;

    data->mSpeaker->AddListener(this);
    
    n = 0;
    XtSetArg(wargs[n], XmNtitle, "Event Info"); ++n;
    XtSetArg(wargs[n], XmNx, 175); ++n;
    XtSetArg(wargs[n], XmNy, 175); ++n;
    XtSetArg(wargs[n], XmNminWidth, 182); ++n;
    XtSetArg(wargs[n], XmNminHeight, 100); ++n;
    SetShell(CreateShell("eiPop",data->toplevel,wargs,n));
    SetMainPane(XtCreateManagedWidget("agedForm", xmFormWidgetClass,GetShell(),NULL,0));

    n = 0;
    XtSetArg(wargs[n], XmNpacking, 			XmPACK_COLUMN); ++n;
    XtSetArg(wargs[n], XmNleftAttachment, 	XmATTACH_FORM); ++n;
    XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_FORM); ++n;
    XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
    rc1 = XtCreateManagedWidget("eiRC1",xmRowColumnWidgetClass,GetMainPane(),wargs,n);
    
    XtCreateManagedWidget("Event #:",  	 xmLabelWidgetClass,rc1,NULL,0);
    XtCreateManagedWidget("Run #:",  	 xmLabelWidgetClass,rc1,NULL,0);
    XtCreateManagedWidget("NHIT:",  	 xmLabelWidgetClass,rc1,NULL,0);
    XtCreateManagedWidget("Tracks:",  	 xmLabelWidgetClass,rc1,NULL,0);
    XtCreateManagedWidget("Lines:",  	 xmLabelWidgetClass,rc1,NULL,0);
    XtCreateManagedWidget("Helices:",  	 xmLabelWidgetClass,rc1,NULL,0);
    XtCreateManagedWidget("Vertex X:",   xmLabelWidgetClass,rc1,NULL,0);
    XtCreateManagedWidget("Vertex Y:",   xmLabelWidgetClass,rc1,NULL,0);
    XtCreateManagedWidget("Vertex Z:",   xmLabelWidgetClass,rc1,NULL,0);

    n = 0;
    XtSetArg(wargs[n], XmNpacking, 			XmPACK_COLUMN); ++n;
    XtSetArg(wargs[n], XmNleftAttachment, 	XmATTACH_WIDGET); ++n;
    XtSetArg(wargs[n], XmNleftWidget, rc1); ++n;
    XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_FORM); ++n;
    XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
    XtSetArg(wargs[n], XmNrightAttachment,	XmATTACH_FORM); ++n;
    rc2 = XtCreateManagedWidget("eiRC2",xmRowColumnWidgetClass,GetMainPane(),wargs,n);
    
    tw_evt		.CreateLabel("evt", 	 rc2,NULL,0);
    tw_run		.CreateLabel("run", 	 rc2,NULL,0);
    tw_nhit		.CreateLabel("nhit", 	 rc2,NULL,0);
    tw_tracks	.CreateLabel("tracks", 	 rc2,NULL,0);
    tw_lines	.CreateLabel("lines", 	 rc2,NULL,0);
    tw_helices	.CreateLabel("helices",  rc2,NULL,0);
    tw_vertexX	.CreateLabel("vx", 	    rc2,NULL,0);
    tw_vertexY	.CreateLabel("vy", 	    rc2,NULL,0);
    tw_vertexZ	.CreateLabel("vz", 	    rc2,NULL,0);
}

void PEventInfoWindow::Listen(int message, void *message_data)
{
    switch (message) {
    	case kMessageNewEvent:
    	case kMessageEventCleared:
    	case kMessageTimeFormatChanged:
    	case kMessageEvIDFormatChanged:
    	case kMessageHitsChanged:
    		SetDirty();
    		break;
    }
}

// UpdateSelf
void PEventInfoWindow::UpdateSelf()
{
    ImageData	*data = GetData();
    char		buff[256];
    TStoreEvent *evt = data->agEvent;
 
#ifdef PRINT_DRAWS
    Printf("-updateEventInfo\n");
#endif

    if (!evt) {
        strcpy(buff, "-");
        tw_evt  	.SetStringNow(buff);
        tw_run  	.SetStringNow(buff);
        tw_nhit 	.SetStringNow(buff);
        tw_tracks   .SetStringNow(buff);
        tw_lines    .SetStringNow(buff);
        tw_helices  .SetStringNow(buff);
        tw_vertexX  .SetStringNow(buff);
        tw_vertexY  .SetStringNow(buff);
        tw_vertexZ  .SetStringNow(buff);
        return;
    }

    sprintf(buff,data->hex_id ? "0x%.6lx" : "%ld",(long)evt->GetEventNumber());
    tw_evt.SetStringNow(buff);
    sprintf(buff, "%ld", (long)data->run_number);
    tw_run.SetStringNow(buff);
    sprintf(buff, "%ld", (long)evt->GetNumberOfHits());
    tw_nhit.SetStringNow(buff);
    sprintf(buff, "%ld", (long)evt->GetNumberOfTracks());
    tw_tracks.SetStringNow(buff);
    sprintf(buff, "%d", evt->GetLineArray() ? evt->GetLineArray()->GetEntries() : 0);
    tw_lines.SetStringNow(buff);
    sprintf(buff, "%d", evt->GetHelixArray() ? evt->GetHelixArray()->GetEntries() : 0);
    tw_helices.SetStringNow(buff);
    if (evt->GetVertex().X() < -998) {
        strcpy(buff,"-");
    } else {
        sprintf(buff, "%g", evt->GetVertex().X());
    }
    tw_vertexX.SetStringNow(buff);
    if (evt->GetVertex().Y() < -998) {
        strcpy(buff,"-");
    } else {
        sprintf(buff, "%g", evt->GetVertex().Y());
    }
    tw_vertexY.SetStringNow(buff);
    if (evt->GetVertex().Z() < -998) {
        strcpy(buff,"-");
    } else {
        sprintf(buff, "%g", evt->GetVertex().Z());
    }
    tw_vertexZ.SetStringNow(buff);
}
