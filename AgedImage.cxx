#include <math.h>
#include <stdlib.h>
#include "AgedImage.h"
#include "AgedWindow.h"
#include "PResourceManager.h"
#include "PHitInfoWindow.h"
#include "PSpeaker.h"
#include "PUtils.h"
#include "menu.h"
#include "TStoreEvent.hh"
#include "TStoreLine.hh"
#include "TStoreHelix.hh"

#define STRETCH				2
#define RCON_WID			(2 * GetScaling())	// half-width of reconstructed point
#define SOURCE_WID			(2 * GetScaling())	// half-width of source monte-carlo point
#define	CELL				0.02
#define	HEX_ASPECT			0.866025
#define CONE_MAG			0.85	// default cone mag. for set-to-vertex

#define UPDATE_FIT    		0x01	// update fit information window
#define	UPDATE_HIT_VALS		0x02	// update displays where hit values are used

#define	NN_AXES				16
#define NE_AXES				11

static Point3 axes_nodes[NN_AXES] = {
							{	0   ,  0   ,  0		},
							{	1.1 ,  0   ,  0		},
							{	0   ,  1.1 ,  0		},
							{	0   ,  0   ,  1.1	},
							{	1.15,  0.05, -0.05	},
							{	1.25, -0.05,  0.05	},
							{	1.15, -0.05,  0.05	},
							{	1.25,  0.05, -0.05	},
							{	0.05,  1.25, -0.05	},
							{	0   ,  1.2 ,  0		},
							{  -0.05,  1.25,  0.05	},
							{	0   ,  1.15,  0		},
							{  -0.05,  0.05,  1.15	},
							{	0.05, -0.05,  1.15	},
							{  -0.05,  0.05,  1.25	},
							{	0.05, -0.05,  1.25	} };

static int	axes_node1[NE_AXES] = { 0,0,0,4,6,8, 9, 9,12,13,14 };
static int	axes_node2[NE_AXES] = { 1,2,3,5,7,9,10,11,13,14,15 };

//-----------------------------------------------------------------------------

AgedImage::AgedImage(PImageWindow *owner, Widget canvas)
		   : PProjImage(owner,canvas)
{
	char		*msg;
	ImageData	*data = owner->GetData();
	
	mHitSize = 0;
	mMaxMagAtan = atan(100.0);	// 100.0 = maximum magnification
	mMarginPix = 2;
	mMarginFactor = 1.25;
	mProj = data->proj;
	mProj.proj_type = IDM_PROJ_3D;
	SetDirty(kDirtyAll);
	
	data->mSpeaker->AddListener(this);

	/* transform the sun direction */
	Transform(&data->sun_dir, 1);
	
	memset(&mAxes, 0, sizeof(mAxes));
	memset(&mDet, 0, sizeof(mDet));
	
	initNodes(&mAxes, axes_nodes, NN_AXES);
	initEdges(&mAxes, axes_node1, axes_node2, NE_AXES);
	msg = loadGeometry(&mDet, IDM_DETECTOR, data->argv);
	if (msg) quit(msg);
	
	SetToHome();

	if (data->show_detector) {
		CalcDetectorShading();
	}
}

AgedImage::~AgedImage()
{
	freeWireFrame(&mAxes);
	freePoly(&mDet);
}


void AgedImage::Listen(int message, void *dataPt)
{
	ImageData	*data = mOwner->GetData();
	
	switch (message) {
		case kMessageDetectorChanged:
			if (data->show_detector) {
				CalcDetectorShading();
				SetDirty(kDirtyDetector);
			} else {
				SetDirty();
			}
			break;
		case kMessageCursorHit:
		case kMessageEventCleared:
		case kMessageFitLinesChanged:
		case kMessageHitSizeChanged:
		case kMessageFitSizeChanged:
			SetDirty();
			break;
		case kMessageNewEvent:
			SetDirty(kDirtyAll);
			break;
		case kMessageFitChanged:
			SetDirty(kDirtyFit);
			break;
		case kMessageAngleFormatChanged:
			if (mProj.theta || mProj.phi || mProj.gamma) {
				SetDirty();
			}
			break;
		default:
			PProjImage::Listen(message,dataPt);
			break;
	}
}


void AgedImage::CalcGrab3(int x,int y)
{
	float		al,fr;

	CalcGrab2(x,y);
	al = sqrt(mGrabX*mGrabX + mGrabY*mGrabY);
	mGrabZ = cos(al);
	if (!al) fr = 0;
	else     fr = sin(al)/al;
	mGrabX *= fr;				
	mGrabY *= fr;	
}


void AgedImage::CalcGrab2(int x,int y)
{
/*
** Calculate distance from center (xc,yc) expressed as a fraction of r
*/
	mGrabX = mProj.pt[0] + (x - mProj.xcen)/(float)mProj.xscl;
	mGrabY = mProj.pt[1] - (y - mProj.ycen)/(float)mProj.yscl;
}


void AgedImage::HandleEvents(XEvent *event)
{
	float		xl,yl,zl;
	Vector3		v1;
	float		theta, phi, alpha;
	Matrix3		tmp;
	ImageData	*data = mOwner->GetData();
	static int	rotate_flag;
	static int	update_flags;

	switch (event->type) {
	
		case ButtonPress:
			if (HandleButton3(event)) return;
			if (!sButtonDown) {
				if (IsInLabel(event->xbutton.x, event->xbutton.y)) {
					ShowLabel(!IsLabelOn());
					SetCursorForPos(event->xbutton.x, event->xbutton.y);
					break;
				}
				XGrabPointer(data->display, XtWindow(mCanvas),0,
							 PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
							 GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
				sButtonDown = event->xbutton.button;
				
				switch (sButtonDown) {
					case Button1:
						rotate_flag = 1;
						break;
					case Button2:
						rotate_flag = 0;
						break;
					default:
						sButtonDown = 0;
						break;
				}
				if (sButtonDown) {
					if (rotate_flag) {
						CalcGrab3(event->xbutton.x, event->xbutton.y);
					} else {
						CalcGrab2(event->xbutton.x, event->xbutton.y);
					}
					SetCursor(CURSOR_MOVE_4);
				}
				update_flags = 0;
			}
			break;			
		case ButtonRelease:
			if (sButtonDown == (int)event->xbutton.button) {
				SetCursor(CURSOR_XHAIR);
				XUngrabPointer(data->display, CurrentTime);
				sButtonDown = 0;
/*
** Update all necessary windows after grab is released
*/
				if (update_flags & UPDATE_HIT_VALS)  {
					sendMessage(data, kMessageHitsChanged);
				}
				if (update_flags & UPDATE_FIT) {
					sendMessage(data, kMessageFitChanged);
				}
			}
			break;

		case MotionNotify:
			if (!sButtonDown) {
				// let the base class handle pointer motion
				PProjImage::HandleEvents(event);
				data->mSpeaker->Speak(kMessage3dCursorMotion,(void *)this);
				break;
			}

			if (event->xmotion.is_hint) break;

			xl = mGrabX;
			yl = mGrabY;
			zl = mGrabZ;
/*
** Shift detector
*/
			if (!rotate_flag) {
				CalcGrab2(event->xbutton.x, event->xbutton.y);
                mProj.pt[0] += xl - mGrabX;
                mProj.pt[1] += yl - mGrabY;
                mGrabX = xl;
                mGrabY = yl;
                SetDirty(kDirtyAll);
                mOwner->SetScrolls();
				Draw();
				break;
			}
			CalcGrab3(event->xmotion.x, event->xmotion.y);

			v1[0] = yl*mGrabZ - zl*mGrabY;		/* calculate axis of rotation */
			v1[1] = zl*mGrabX - xl*mGrabZ;
			v1[2] = xl*mGrabY - yl*mGrabX;

			unitVector(v1);				/* (necessary for phi calculation) */

			if (v1[0] || v1[1]) theta = atan2(v1[1],v1[0]);
			else theta = 0;
			phi   = acos(v1[2]);
			alpha = -vectorLen(mGrabX-xl, mGrabY-yl, mGrabZ-zl);

			getRotMatrix(tmp, theta, phi, alpha);

            matrixMult(mProj.rot, tmp);
            RotationChanged();
            SetDirty(kDirtyAll);
			break;
			
		default:
			PProjImage::HandleEvents(event);
			break;
	}
}


/*
** Set scrollbars to proper location
*/
void AgedImage::SetScrolls()
{
	int			pos;

	pos = kScrollMax - (int)(kScrollMax * atan(mProj.mag) / mMaxMagAtan + 0.5);
	mOwner->SetScrollValue(kScrollLeft, pos);
	pos = (kScrollMax/2) + (int)(kScrollMax*mSpinAngle/(4*PI));
	mOwner->SetScrollValue(kScrollBottom, pos);

	if (mProj.pt[2] >= mProj.proj_max) {
		pos = kScrollMax;
	} else if (mProj.pt[2] <= mProj.proj_min) {
		pos = 0;
	} else {
		pos = (int)(kScrollMax*(1.0-atan(STRETCH/(mProj.pt[2]-mProj.proj_min))/(PI/2))+0.5);
	}
	mOwner->SetScrollValue(kScrollRight, pos);
}


void AgedImage::ScrollValueChanged(EScrollBar bar, int value)
{
	int			val;
	float		t;
	
	switch (bar) {
		case kScrollRight:
			val = kScrollMax - value;
			if (val == 0) {
				mProj.pt[2] = mProj.proj_max;
			} else if (val == kScrollMax) {
				mProj.pt[2] = mProj.proj_min;
			} else {
				t = val * (PI/(2*kScrollMax));
				mProj.pt[2] = STRETCH / tan(t) + mProj.proj_min;
			}
			SetDirty(kDirtyAll);
			break;
		case kScrollLeft:
			mProj.mag = tan(mMaxMagAtan * (kScrollMax - value) / kScrollMax);
			Resize();
			break;
		case kScrollBottom: {
			Matrix3	rot;
			float newSpin = (value - kScrollMax/2) * (4*PI) /kScrollMax;
			get3DMatrix(rot,0.0,0.0,mSpinAngle-newSpin);
			mSpinAngle = newSpin;
			matrixMult(rot,mProj.rot);
			memcpy(mProj.rot,rot,sizeof(rot));
			RotationChanged();
			SetDirty(kDirtyAll);
		} 	break;
		
		default:
			break;
	}
}


void AgedImage::Resize()
{
	mProj.xsiz = mWidth;
	mProj.ysiz = mHeight;

	mProj.xcen = mWidth/2;
	mProj.ycen = mHeight/2;

	mProj.xscl = (int)(0.4 * mProj.mag * (mProj.xsiz<mProj.ysiz ? mProj.xsiz : mProj.ysiz));
	if (mProj.xscl < 1) mProj.xscl = 1;
	mProj.yscl = mProj.xscl;

	SetDirty(kDirtyAll);
}


void AgedImage::TransformHits()
{
	int			i;
	ImageData	*data = mOwner->GetData();
	int			num = data->hits.num_nodes;
#ifdef PRINT_DRAWS
	Printf(":transform 3-D\n");
#endif
	if (num) {

		Node *node = data->hits.nodes;
		
		Transform(node,num);

		float *pt = mProj.pt;
		
		if (pt[2] < mProj.proj_max) {

			for (i=0; i<num; ++i,++node) {
				float dot = (pt[0] - node->xr) * node->xr +
						    (pt[1] - node->yr) * node->yr +
						    (pt[2] - node->zr) * node->zr;
				if (dot > 0) node->flags |= NODE_HID;
			}

		} else {

			for (i=0; i<num; ++i,++node) {
				if (node->zr > 0) node->flags |= NODE_HID;
			}
		}
	}

	/* must do this to save mLastImage */
	PProjImage::TransformHits();
}


/* the rotation matrix has changed */
void AgedImage::RotationChanged()
{
	matrixTranspose(mProj.rot,mProj.inv);
	
	/* calculate current viewing angles */
	// theta is the angle from the z axis to the viewing direction
	if (mProj.rot[2][2]) {
		mProj.theta = acos(mProj.rot[2][2]) - PI/2;
	} else {
		mProj.theta = 0;
	}
	// phi is the angle from the x axis to the viewing direction
	// measured CCW in the x-y plane
	if (mProj.rot[2][0]!=1.0 || mProj.rot[2][1]) {
		double len = sqrt(mProj.rot[2][0]*mProj.rot[2][0] + mProj.rot[2][1]*mProj.rot[2][1]);
		if (len) {
			double dx = mProj.rot[2][0] / len;
			double dy = mProj.rot[2][1] / len;
			mProj.phi = atan2(dy,dx);
		} else {
			mProj.phi = 0;
		}
	} else {
		mProj.phi = 0;
	}
	// gamma is the CW angle between the projected z axis and 'up' on the screen
	if (mProj.rot[0][2] || mProj.rot[1][2]) {
		mProj.gamma = atan2(mProj.rot[0][2], mProj.rot[1][2]);
	} else {
		mProj.gamma = 0;
	}
}


void AgedImage::SetToHome(int n)
{
	mProj.pt[0] = mProj.pt[1] = 0;
	mProj.pt[2] = 5; //mProj.proj_max;
	mProj.mag   = 0.70;
	mProj.theta	= 0.;
	mProj.phi	= 0.;
	mProj.gamma = 0.;
	mSpinAngle	= 0.;

	if (n == 0) {
        /* set home position to z left, y up */
        mProj.rot[0][0] = 0;  mProj.rot[0][1] = 0;  mProj.rot[0][2] = -1;
        mProj.rot[1][0] = 0;  mProj.rot[1][1] = 1;  mProj.rot[1][2] = 0;
        mProj.rot[2][0] = 1;  mProj.rot[2][1] = 0;  mProj.rot[2][2] = 0;
    } else {
        // set to top position - x right, y up
        mProj.rot[0][0] = -1;  mProj.rot[0][1] = 0;  mProj.rot[0][2] = 0;
        mProj.rot[1][0] = 0;  mProj.rot[1][1] = 1;  mProj.rot[1][2] = 0;
        mProj.rot[2][0] = 0;  mProj.rot[2][1] = 0;  mProj.rot[2][2] = -1;
    }
	RotationChanged();

	Resize();
}


/*
** Calculate shading for detector faces
*/
void AgedImage::CalcDetectorShading()
{	
	Face		*face, *lface;
	Node		*n2;
	float		dot;
	ImageData	*data = mOwner->GetData();

	face = mDet.faces;
	lface= face + mDet.num_faces;
	n2   = &data->sun_dir;
	for (; face<lface; ++face) {
		dot = face->norm.x*n2->x3 + face->norm.y*n2->y3 + face->norm.z*n2->z3;
		face->flags = ((int)((dot + 1) * data->det_cols / 2) << FACE_COL_SHFT) 
						| (face->flags & FACE_HID);	/* preserve hidden flags */
	}
}


/* ----------------------------------------------------------------------------
** Main Aged 3-D drawing routine
*/
void AgedImage::DrawSelf()
{
	ImageData	*data = mOwner->GetData();
	XSegment	segments[MAX_EDGES], *sp;
	XPoint		point[6];
	int			i,j,n,num;
	Node		*n1,*n2;
	Node        nod[6];
	Edge		*edge, *last;
	Face		*face, *lface;

/*
** recalculate necessary values for display
*/
	if (IsDirty() & kDirtyAll) {
		if (IsDirty() & kDirtyHits) {
			TransformHits();
		}
		if (IsDirty() & kDirtyAxes) {
			Transform(mAxes.nodes, mAxes.num_nodes);
		}
		if ((IsDirty() & kDirtyDetector) && data->show_detector) {
			Vector3		ip;
			transformPoly(&mDet,&mProj);
			vectorMult(mProj.inv,mProj.pt,ip);
		}
	}
/*
** Start drawing
*/
#ifdef PRINT_DRAWS
	Printf("draw 3-D image\n");
#endif

	PImageCanvas::DrawSelf();	// let the base class clear the drawing area

	// transform hits for this image if necessary
	if (data->mLastImage != this) {
	    TransformHits();
	}
	SetFont(data->hist_font);

	/* draw projection angles */
	if (mProj.theta || mProj.phi || mProj.gamma) DrawAngles(0,kAngleAll);
/*
** Draw detector
*/
	if (data->show_detector) {
		mDrawable->Comment("Detector");
		lface= mDet.faces + mDet.num_faces;
		n2 = &data->sun_dir;
		for (face=mDet.faces; face<lface; ++face) {
            if (face->flags & FACE_HID) continue;
			num = face->num_nodes;
            for (i=0,n=0; i<num; ++i) {
                n1 = face->nodes[i];
                if (n1->flags & NODE_OUT) ++n;	/* count # of nodes behind proj screen */
                point[i].x = n1->x;
                point[i].y = n1->y;
            }
            if (n<num) {
                SetForeground(FIRST_DET_COL + (face->flags>>FACE_COL_SHFT));
                FillPolygon(point,num);
                for (i=0; i<num; ++i) {
                    j = (i + 1) % num;
                    segments[i].x1 = point[i].x;
                    segments[i].y1 = point[i].y;
                    segments[i].x2 = point[j].x;
                    segments[i].y2 = point[j].y;
                }
                SetForeground(HID_COL);
                DrawSegments(segments, num);
            }
		}
		for (face=mDet.faces; face<lface; ++face) {
			if (!(face->flags & FACE_HID)) continue;
			num = face->num_nodes;
            for (i=0,n=0; i<num; ++i) {
                n1 = face->nodes[i];
                if (n1->flags & NODE_OUT) ++n;	/* count # of nodes behind proj screen */
                segments[i].x1 = n1->x;
                segments[i].y1 = n1->y;
            }
            if (n<num) {
                for (i=0; i<num; ++i) {
                    j = (i + 1) % num;
                    segments[i].x2 = segments[j].x1;
                    segments[i].y2 = segments[j].y1;
                }
                SetForeground(FRAME_COL);
                DrawSegments(segments, num);
            }
		}
	}
/*
** Draw axes
*/
	mDrawable->Comment("Axes and Sun vector");

	edge = mAxes.edges;
	last = edge + mAxes.num_edges;
	for (sp=segments; edge<last; ++edge) {
		n1  = edge->n1;
		n2  = edge->n2;
		if (n1->flags & n2->flags & (NODE_HID | NODE_OUT)) continue;
		sp->x1 = n1->x;
		sp->y1 = n1->y;
		sp->x2 = n2->x;
		sp->y2 = n2->y;
		++sp;
	}
	SetForeground(AXES_COL);
	SetLineWidth(2);
	DrawSegments(segments,sp-segments);
	SetLineWidth(THICK_LINE_WIDTH);

    TStoreEvent *evt = data->agEvent;
    if (!evt) return;
/*
** Draw space points
*/
    const TObjArray *points = evt->GetSpacePoints();
	if (points && points->GetEntries() > 0 && data->wSpStyle != IDM_SP_NONE) {
        num = points->GetEntries();
        HitInfo *hi = data->hits.hit_info;
        int bit_mask = data->bit_mask;
        int sz = (int)(data->hit_size * 2 + 0.5);
		for (i=0, n1=data->hits.nodes; i<num; ++i, ++hi, ++n1) {
            if (hi->flags & bit_mask) continue;	/* only consider unmasked hits */
            SetForeground(FIRST_SCALE_COL + hi->hit_val);
            switch (data->wSpStyle) {
                case IDM_SP_ERRORS: {
                    TSpacePoint* spi = (TSpacePoint*) points->At(i);
                    nod[0].x3 = nod[1].x3 = nod[2].x3 = nod[3].x3 = nod[4].x3 = nod[5].x3 = spi->GetX() / AG_SCALE;
                    nod[0].y3 = nod[1].y3 = nod[2].y3 = nod[3].y3 = nod[4].y3 = nod[5].y3 = spi->GetY() / AG_SCALE;
                    nod[0].z3 = nod[1].z3 = nod[2].z3 = nod[3].z3 = nod[4].z3 = nod[5].z3 = spi->GetZ() / AG_SCALE;
                    nod[0].x3 -= spi->GetErrX() / AG_SCALE;
                    nod[1].x3 += spi->GetErrX() / AG_SCALE;
                    nod[2].y3 -= spi->GetErrY() / AG_SCALE;
                    nod[3].y3 += spi->GetErrY() / AG_SCALE;
                    nod[4].z3 -= spi->GetErrZ() / AG_SCALE;
                    nod[5].z3 += spi->GetErrZ() / AG_SCALE;
                    Transform(nod,6);
                    for (j=0, sp=segments; j<6; j+=2, ++sp) {
                        sp->x1 = nod[j].x;
                        sp->y1 = nod[j].y;
                        sp->x2 = nod[j+1].x;
                        sp->y2 = nod[j+1].y;
                    }
                    DrawSegments(segments, 3);
                }   break;
                case IDM_SP_SQUARES:
                    FillRectangle(n1->x-sz, n1->y-sz, sz*2+1, sz*2+1);
                    break;
                case IDM_SP_CIRCLES:
                    FillArc(n1->x, n1->y, sz, sz);
                    break;
            }
		}
    }
#if 0 //TEST
    if (points) {
        for (int i=0; i<num; ++i) {
            TSpacePoint* spi = (TSpacePoint*) points->At(i);
            double x=spi->GetX(),y=spi->GetY(),z=spi->GetZ();
            if (x*x+y*y>100) continue;
            printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            printf("%d) %g %g %g %lx %lx %lx time=%g h=%g r=%g phi=%g wire=%d pad=%d\n",
                i,x,y,z,
                *(unsigned long *)&x, *(unsigned long *)&y, *(unsigned long *)&z,
                spi->GetTime(),spi->GetHeight(),spi->GetR(),spi->GetPhi(),spi->GetWire(),spi->GetPad());
        }
    }
#endif
/*
** Draw the cursor
*/
    if (data->cursor_hit >= 0) {
        Node *nod = data->hits.nodes + data->cursor_hit;
        SetForeground(CURSOR_COL);
        int sz = (int)(data->hit_size * 3 + 0.5);
        if (data->wSpStyle == IDM_SP_SQUARES) {
            DrawRectangle(nod->x-sz, nod->y-sz, sz*2, sz*2);
        } else {
            DrawArc(nod->x, nod->y, sz, sz);
        }
    }
/*
** Draw fit lines
*/
    SetForeground(FIT_COL);
    const TObjArray *lines = evt->GetLineArray();
    if (lines && lines->GetEntries() > 0) {
        for (i=0, sp=segments; i<lines->GetEntries(); ++i, ++sp) {
            if (sp - segments >= MAX_EDGES) {
                DrawSegments(segments, MAX_EDGES);
                sp = segments;
            }
            TStoreLine *line = (TStoreLine *)lines->At(i);
            nod[0].x3 = line->GetPoint()->X() + line->GetDirection()->X() * 2;
            nod[1].x3 = line->GetPoint()->X() - line->GetDirection()->X() * 2;
            nod[0].y3 = line->GetPoint()->Y() + line->GetDirection()->Y() * 2;
            nod[1].y3 = line->GetPoint()->Y() - line->GetDirection()->Y() * 2;
            nod[0].z3 = line->GetPoint()->Z() + line->GetDirection()->Z() * 2;
            nod[1].z3 = line->GetPoint()->Z() - line->GetDirection()->Z() * 2;
    		Transform(nod,2);
    		sp->x1 = nod[0].x;
    		sp->y1 = nod[0].y;
    		sp->x2 = nod[1].x;
    		sp->y2 = nod[1].y;
        }
        DrawSegments(segments, sp - segments);
    }
/*
** Draw fit helices
*/
    const TObjArray *helices = evt->GetHelixArray();
    if (data->show_fit && helices && helices->GetEntries() > 0) {
        for (i=0, sp=segments; i<helices->GetEntries(); ++i, ++sp) {
            TStoreHelix *helix = (TStoreHelix *)helices->At(i);
            const int kNumPoints = 100;
            n1 = n2 = 0;
            for (j=0, sp=segments; j<kNumPoints; ++j) {
                double phi = helix->GetPhi0() + j * PI / kNumPoints;
                n1 = n2;
                n2 = nod + (j & 1);
                n2->x3 = (helix->GetD() * cos(helix->GetPhi0()) + (cos(helix->GetPhi0()) - cos(phi))/helix->GetC()) / AG_SCALE;
                n2->y3 = (helix->GetD() * sin(helix->GetPhi0()) + (sin(helix->GetPhi0()) - sin(phi))/helix->GetC()) / AG_SCALE;
                n2->z3 = (helix->GetZ0() - tan(helix->GetLambda()) * phi) / AG_SCALE;
                Transform(n2,1);
                if (!n1 || (n1->flags & n2->flags & (NODE_HID | NODE_OUT))) continue;
                sp->x1 = n1->x;
                sp->y1 = n1->y;
                sp->x2 = n2->x;
                sp->y2 = n2->y;
                ++sp;
            }
            DrawSegments(segments, sp - segments);
        }
    }
/*
** Draw fit vertex
*/
    if (data->show_fit && evt->GetVertex().X() > -998) {
        nod[0].x3 = evt->GetVertex().X() / AG_SCALE;
        nod[0].y3 = evt->GetVertex().Y() / AG_SCALE;
        nod[0].z3 = evt->GetVertex().Z() / AG_SCALE;
        Transform(nod,1);
        int sz = (int)(data->fit_size * 3 + 0.5);
    	FillArc(nod[0].x, nod[0].y, sz, sz);
    }
/*
** Restore default drawing parameters
*/
	SetLineWidth(THICK_LINE_WIDTH);
	SetForeground(TEXT_COL);
}
