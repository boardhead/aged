#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include "ImageData.h"
#include "PHitInfoWindow.h"
#include "PImageWindow.h"
#include "PProjImage.h"
#include "PSpeaker.h"
#include "PUtils.h"
#include "menu.h"

//---------------------------------------------------------------------------------
// PHitInfoWindow constructor
//
PHitInfoWindow::PHitInfoWindow(ImageData *data)
			  : PWindow(data)
{
	Widget	rc1, rc2;
	int		n;
	Arg		wargs[16];
		
	mLastNum = -1;
	
	n = 0;
	XtSetArg(wargs[n], XmNtitle, "Hit Info"); ++n;
	XtSetArg(wargs[n], XmNx, 200); ++n;
	XtSetArg(wargs[n], XmNy, 200); ++n;
	XtSetArg(wargs[n], XmNminWidth, 165); ++n;
	XtSetArg(wargs[n], XmNminHeight, 100); ++n;
	SetShell(CreateShell("hiPop",data->toplevel,wargs,n));
	SetMainPane(XtCreateManagedWidget("agedForm", xmFormWidgetClass,GetShell(),NULL,0));

	n = 0;
	XtSetArg(wargs[n], XmNpacking, 			XmPACK_COLUMN); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, 	XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	rc1 = XtCreateManagedWidget("hiRC1",xmRowColumnWidgetClass,GetMainPane(),wargs,n);

	n = 0;
	XtSetArg(wargs[n], XmNpacking, 			XmPACK_COLUMN); ++n;
	XtSetArg(wargs[n], XmNleftAttachment, 	XmATTACH_WIDGET); ++n;
	XtSetArg(wargs[n], XmNleftWidget, rc1); ++n;
	XtSetArg(wargs[n], XmNtopAttachment, 	XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
	XtSetArg(wargs[n], XmNrightAttachment,	XmATTACH_FORM); ++n;
	rc2 = XtCreateManagedWidget("eiRC2",xmRowColumnWidgetClass,GetMainPane(),wargs,n);
	
//	XtCreateManagedWidget("EvID:",   xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Time:",   xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Height:", xmLabelWidgetClass,rc1,NULL,0);
//	XtCreateManagedWidget("Qhl:",    xmLabelWidgetClass,rc1,NULL,0);
//	hi_qlx_label.CreateLabel("Qlx:", rc1,NULL,0);
	XtCreateManagedWidget("Wire:",   xmLabelWidgetClass,rc1,NULL,0);
	XtCreateManagedWidget("Pad:",    xmLabelWidgetClass,rc1,NULL,0);
//	XtCreateManagedWidget("Channel:",xmLabelWidgetClass,rc1,NULL,0);
//	XtCreateManagedWidget("Cell:",   xmLabelWidgetClass,rc1,NULL,0);
//	XtCreateManagedWidget("Type:",   xmLabelWidgetClass,rc1,NULL,0);
//	XtCreateManagedWidget("Panel:",  xmLabelWidgetClass,rc1,NULL,0);
//	XtCreateManagedWidget("PMT:",    xmLabelWidgetClass,rc1,NULL,0);
	
	// create XYZ labels invisible
	hi_xyz_labels[0].CreateLabel("X:",  rc1,NULL,0);
	hi_xyz_labels[1].CreateLabel("Y:",  rc1,NULL,0);
	hi_xyz_labels[2].CreateLabel("Z:",  rc1,NULL,0);
	
//	hi_gt 		.CreateLabel("hiGT",     rc2,NULL,0);
	hi_time  	.CreateLabel("hiTac",    rc2,NULL,0);
	hi_height  	.CreateLabel("hiQhs",    rc2,NULL,0);
//	hi_qhl 		.CreateLabel("hiQhl",    rc2,NULL,0);
//	hi_qlx 		.CreateLabel("hiQlx",    rc2,NULL,0);
//	hi_nhit		.CreateLabel("hiNhit",   rc2,NULL,0);
	hi_wire 	.CreateLabel("hiCrate",  rc2,NULL,0);
	hi_pad 	    .CreateLabel("hiCard",   rc2,NULL,0);
//	hi_channel	.CreateLabel("hiChannel",rc2,NULL,0);
//	hi_cell 	.CreateLabel("hiCell",   rc2,NULL,0);
//	hi_type 	.CreateLabel("hiType",   rc2,NULL,0);
//	hi_panel 	.CreateLabel("hiPanel",  rc2,NULL,0);
//	hi_pmt  	.CreateLabel("hiPMT",    rc2,NULL,0);
	
	// create XYZ labels invisible
	hi_xyz[0].CreateLabel("hiX",  	rc2,NULL,0);
	hi_xyz[1].CreateLabel("hiY",  	rc2,NULL,0);
	hi_xyz[2].CreateLabel("hiZ",  	rc2,NULL,0);
	
	// manage XYZ labels if hit_xyz is on
	if (!data->hit_xyz) ManageXYZ(0);
	
	data->mSpeaker->AddListener(this);	// listen for cursor motion
	
	ClearEntries();
}

PHitInfoWindow::~PHitInfoWindow()
{
}

void PHitInfoWindow::ClearEntries()
{
	int		i;
	
	char	*str = "-";
//	hi_gt.SetString(str);
	hi_time.SetString(str);
	hi_height.SetString(str);
//	hi_qhl.SetString(str);
//	hi_qlx.SetString(str);
//	hi_nhit.SetString(str);
	hi_wire.SetString(str);
	hi_pad.SetString(str);
//	hi_channel.SetString(str);
//	hi_cell.SetString(str);
//	hi_type.SetString(str);
//	hi_panel.SetString(str);
//	hi_pmt.SetString(str);
	for (i=0; i<3; ++i) {
		hi_xyz[i].SetString(str);
	}
}

/* UpdateSelf - set hit information window for PMT nearest the specified cursor position */
/* Note: hits must be tranformed to appropriate projection BEFORE calling this routine */
void PHitInfoWindow::UpdateSelf()
{
	HitInfo		*hi;
	char		buff[64];
	ImageData	*data = mData;
	int			num = data->cursor_hit;	// current hit number near cursor
	
#ifdef PRINT_DRAWS
	Printf("-updateHitInfo\n");
#endif
	mLastNum = num;
	if (num == -1 ) {
		ClearEntries();
	} else {
		hi = data->hits.hit_info + num;
//		sprintf(buff,data->hex_id ? "0x%.6lx" : "%ld",hi->gt);
//		hi_gt.SetString(buff);
		sprintf(buff,"%g",hi->time);
		hi_time.SetString(buff);
		sprintf(buff,"%g",hi->height);
		hi_height.SetString(buff);
		sprintf(buff,"%d",(int)hi->wire);
		hi_wire.SetString(buff);
		sprintf(buff,"%d",(int)hi->pad);
		hi_pad.SetString(buff);
		float *xyz = &(data->hits.nodes[num].x3);
        for (int i=0; i<3; ++i) {
#ifdef ANTI_ALIAS
            sprintf(buff,"%.1f \xc2\xb1 %.1f",xyz[i] * AG_SCALE,hi->error[i]);
#else
            sprintf(buff,"%.1f \xb1 %.1f",xyz[i] * AG_SCALE,hi->error[i]);
#endif
            hi_xyz[i].SetString(buff);
        }
	}
}

void PHitInfoWindow::ManageXYZ(int manage)
{
	Widget		widgets[6];
	
	for (int i=0; i<3; ++i) {
		widgets[i] = hi_xyz[i].GetWidget();
		widgets[i+3] = hi_xyz_labels[i].GetWidget();
	}
	if (manage) {
	    // must manage right and left rowcol widgets separately
		XtManageChildren(widgets, 3);
		XtManageChildren(widgets+3, 3);
	} else {
		XtUnmanageChildren(widgets, 3);
		XtUnmanageChildren(widgets+3, 3);
	}
}

// ResizeToFit - resize shell height to fit the labels
void PHitInfoWindow::ResizeToFit()
{
/*	Widget		last_label;

	if (GetData()->hit_xyz) {
		last_label = hi_xyz[2].GetWidget();
	} else {
		last_label = hi_pmt.GetWidget();
	}
	PWindow::ResizeToFit(last_label);*/
}

// SetHitXYZ - show or hide XYZ labels according to data->hit_xyz setting
void PHitInfoWindow::SetHitXYZ()
{
	if (GetData()->hit_xyz) {
		// show the XYZ labels
		ManageXYZ(1);
	} else {
		// hide the XYZ labels
		ManageXYZ(0);
	}
	ResizeToFit();
}

void PHitInfoWindow::Listen(int message, void *message_data)
{
	switch (message) {
		case kMessageNewEvent:
			if (mLastNum >= 0) {
				// the event has changed, so the displayed hit data is now invalid
				// -> set the last displayed hit number to something invalid too
				//    to force the new data to be displayed
				mLastNum = 99999;
			}
			SetDirty();
			break;
		case kMessageCursorHit:
			// only dirty if this is a different hit
			if (mLastNum != mData->cursor_hit) {
				SetDirty();
			}
			break;
		case kMessageHitDiscarded:
			SetDirty();
			break;
		case kMessageHitXYZChanged:
			SetHitXYZ();
			break;
	}
}
