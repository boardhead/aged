/*
** X-window offscreen drawable - PH 09/22/99
*/
#include <stdio.h>
#include <string.h>
#include "PDrawXPixmap.h"
#include "PResourceManager.h"
#include "PUtils.h"
#include "colours.h"

#ifdef SMOOTH_LINES
#include <math.h>
#endif

#ifndef PI
#define	PI				3.14159265358979324
#endif

const int kMinArcPoints = 8;
const int kMaxArcPoints = 120;

PDrawXPixmap::PDrawXPixmap(Display *dpy, GC gc, int depth, Widget w)
{
	mDpy = dpy;
	mGC	 = gc;
	mDepth = depth;
	mAltWidget = w;	// widget to draw into if we can't create pixmap
	mPix = 0;
	mWidth = 0;
	mHeight = 0;
#ifdef SMOOTH_LINES
    mLineWidth = 1.0;
#endif
}

PDrawXPixmap::~PDrawXPixmap()
{
	FreePixmap();
#ifdef SMOOTH_FONTS
	if (mXftDraw) {
	    XftDrawDestroy(mXftDraw);
	    mXftDraw = NULL;
	}
#endif
}

//---------------------------------------------------------------------------------------
// BeginDrawing
//
int PDrawXPixmap::BeginDrawing(int width, int height)
{
	int		sizeChanged = 0;
	
	if (mWidth != width || mHeight != height) {
		// free old pixmap since our size changed
		FreePixmap();
		sizeChanged = 1;
	}
    if (mPix) {
        mDrawable = mPix;
	} else {
		if (!width) return(0);
		mPix = XCreatePixmap(mDpy, DefaultRootWindow(mDpy), width, height, mDepth); 
		if (mPix) {
			mDrawable = mPix;
		} else {
			if (sizeChanged) {
				Printf("No memory for pixmap!\x07\n");
			}
			if (mAltWidget && XtWindow(mAltWidget)) {
				mDrawable = XtWindow(mAltWidget);
			} else {
				return(0);
			}
		}
		mWidth = width;
		mHeight = height;
	}
#ifdef SMOOTH_FONTS
    mXftDraw = XftDrawCreate(mDpy, mDrawable, DefaultVisual(mDpy,DefaultScreen(mDpy)),
               DefaultColormap(mDpy, DefaultScreen(mDpy)));
    SetXftColour(WhitePixel(mDpy, DefaultScreen(mDpy)));
    mXftFmt = XRenderFindStandardFormat(mDpy,PictStandardA8);
#ifdef SMOOTH_LINES
    mXftPicture = XftDrawPicture(mXftDraw);
#endif
#endif

	return(1);
}

void PDrawXPixmap::EndDrawing()
{
    // must draw directly to window since pixmap has already been copied
    mDrawable = XtWindow(mAltWidget);
#ifdef SMOOTH_FONTS
	if (mXftDraw) {
	    XftDrawDestroy(mXftDraw);
	}
    mXftDraw = XftDrawCreate(mDpy, mDrawable, DefaultVisual(mDpy,DefaultScreen(mDpy)),
               DefaultColormap(mDpy, DefaultScreen(mDpy)));
#ifdef SMOOTH_LINES
    mXftPicture = XftDrawPicture(mXftDraw);
#endif
#endif
}

#ifdef SMOOTH_FONTS
void PDrawXPixmap::SetXftColour(Pixel pixel)
{
    if (mDpy) {
        XColor c;
        c.pixel = pixel;
        XQueryColor(mDpy, DefaultColormap(mDpy,DefaultScreen(mDpy)), &c);
        XRenderColor xrcolor;
        xrcolor.red   = c.red;
        xrcolor.green = c.green;
        xrcolor.blue  = c.blue;
        xrcolor.alpha = 0xffff;
        XftColorAllocValue(mDpy, DefaultVisual(mDpy,DefaultScreen(mDpy)),
                   DefaultColormap(mDpy, DefaultScreen(mDpy) ), &xrcolor, &mXftColor );

        mXftPict = XftDrawSrcPicture(mXftDraw, &mXftColor);
    }
}
#endif

void PDrawXPixmap::FreePixmap()
{
	if (mPix) {
		XFreePixmap(mDpy,mPix);
		mPix = 0;
		mWidth = 0;
		mHeight = 0;
		mDrawable = 0;
	}
}

int PDrawXPixmap::HasPixmap()
{
	return(mPix != 0);
}

void PDrawXPixmap::SetForeground(int col_num)
{
    Pixel pixel;
	if (mColours) {
		pixel = mColours[col_num];
	} else  if (col_num < NUM_COLOURS) {
        pixel = PResourceManager::sResource.colour[col_num];
    } else if ((col_num-=NUM_COLOURS) < PResourceManager::sResource.num_cols) {
        pixel = PResourceManager::sResource.scale_col[col_num];
    } else if ((col_num-=PResourceManager::sResource.num_cols) < PResourceManager::sResource.det_cols) {
        pixel = PResourceManager::sResource.det_col[col_num];
    } else {
        pixel = WhitePixel(mDpy, DefaultScreen(mDpy));
	}
	XSetForeground(mDpy, mGC, pixel);
#ifdef SMOOTH_FONTS
    SetXftColour(pixel);
#endif
}

void PDrawXPixmap::SetForegroundPixel(Pixel pixel)
{
	XSetForeground(mDpy, mGC, pixel);
#ifdef SMOOTH_FONTS
    SetXftColour(pixel);
#endif
}

void PDrawXPixmap::SetFont(XFontStruct *font)
{
	PDrawable::SetFont(font);
	XSetFont(mDpy, mGC, font->fid);
}

void PDrawXPixmap::SetLineWidth(float width)
{
	XSetLineAttributes(mDpy, mGC, (int)width, LineSolid, CapButt, JoinMiter);
#ifdef SMOOTH_LINES
    mLineWidth = width < 1 ? 1 : width;
#endif
}

void PDrawXPixmap::DrawRectangle(int x,int y,int w,int h)
{
	XDrawRectangle(mDpy, mDrawable, mGC, x, y, w, h);
}

void PDrawXPixmap::FillRectangle(int x,int y,int w,int h)
{
	XFillRectangle(mDpy, mDrawable, mGC, x, y, w, h);
}

void PDrawXPixmap::DrawSegments(XSegment *segments, int num, int smooth)
{
#ifdef SMOOTH_LINES
    if (smooth) {
        XSegment *sp = segments;
        for (int i=0; i<num; ++i, ++sp) {
            if (sp->x1==sp->x2 || sp->y1==sp->y2) {
                XDrawLine(mDpy,mDrawable,mGC,sp->x1,sp->y1,sp->x2,sp->y2);
            } else {
                DrawSmoothLine(sp->x1, sp->y1, sp->x2, sp->y2);
            }
        }
    } else {
        XDrawSegments(mDpy,mDrawable,mGC,segments,num);
    }
#else
    XDrawSegments(mDpy,mDrawable,mGC,segments,num);
#endif
}

void PDrawXPixmap::DrawPoint(int x, int y)
{
	XDrawPoint(mDpy,mDrawable,mGC,x,y);
}

#ifdef SMOOTH_LINES
void PDrawXPixmap::DrawSmoothLine(double x1, double y1, double x2, double y2)
{
    XPointDouble poly[4];
    XDouble dx = x2 - x1;
    XDouble dy = y2 - y1;
    XDouble len = sqrt(dx*dx + dy*dy);
    XDouble ldx = (mLineWidth/2.0) * dy / len;
    XDouble ldy = (mLineWidth/2.0) * dx / len;

    poly[0].x = x1 + ldx + 0.5;  poly[0].y = y1 - ldy + 0.5;
    poly[1].x = x2 + ldx + 0.5;  poly[1].y = y2 - ldy + 0.5;
    poly[2].x = x2 - ldx + 0.5;  poly[2].y = y2 + ldy + 0.5;
    poly[3].x = x1 - ldx + 0.5;  poly[3].y = y1 + ldy + 0.5;

    XRenderCompositeDoublePoly(mDpy,
                              PictOpOver,
                              XftDrawSrcPicture(mXftDraw, &mXftColor),
                              mXftPicture,
                              XRenderFindStandardFormat(mDpy,PictStandardA8),
                              0, 0, 0, 0, poly, 4, EvenOddRule);
}
#endif

void PDrawXPixmap::DrawLine(int x1,int y1,int x2,int y2)
{
#ifdef SMOOTH_LINES
    if (x1==x2 || y1==y2) {
        XDrawLine(mDpy,mDrawable,mGC,x1,y1,x2,y2);
    } else {
        DrawSmoothLine(x1, y1, x2, y2);
    }
#else
    XDrawLine(mDpy,mDrawable,mGC,x1,y1,x2,y2);
#endif
}

void PDrawXPixmap::FillPolygon(XPoint *point, int num)
{
	XFillPolygon(mDpy,mDrawable,mGC,point,num, Convex, CoordModeOrigin);
}

void PDrawXPixmap::DrawString(int x, int y, char *str, ETextAlign_q align)
{
	int len = strlen(str);

    switch (align / 3) {
        case 0:		// top
            y += GetFontAscent();
            break;
        case 1:		// middle
            y += GetFontAscent() / 2;
            break;
        case 2:		// bottom
            break;
    }
#ifdef SMOOTH_FONTS
    if (IsSmoothText()) {
        XGlyphInfo  extents;
        if (align % 3) {
            XftTextExtents8(mDpy, GetXftFont(), (XftChar8 *)str, len, &extents);
        }
		switch (align % 3) {
			case 0:		// left
				break;
			case 1:		// center
				x -= extents.width / 2;
				break;
			case 2:		// right
				x -= extents.width;
				break;
		}
        XftDrawString8(mXftDraw, &mXftColor, GetXftFont(), x-1, y, (XftChar8 *)str, len);
    } else {
#endif
	if (GetFont()) {
		switch (align % 3) {
			case 0:		// left
				break;
			case 1:		// center
				x -= XTextWidth(GetFont(), str, len) / 2;
				break;
			case 2:		// right
				x -= XTextWidth(GetFont(), str, len);
				break;
		}
	}
	XDrawString(mDpy,mDrawable,mGC,x,y,str,len);
#ifdef SMOOTH_FONTS
    }
#endif
}

void PDrawXPixmap::DrawArc(int cx,int cy,int rx,int ry,float ang1,float ang2)
{
#ifdef SMOOTH_LINES
    if (IsSmoothLines()) {
        int num = rx + ry;
        if (num < kMinArcPoints) num = kMinArcPoints;
        if (num > kMaxArcPoints) num = kMaxArcPoints;
        double ang = ang1 * PI / 180;
        double x1 = cx + rx * cos(ang);
        double y1 = cy + ry * sin(ang);
        double step = (ang2 * PI / 180 - ang) / num;
        for (int i=1; i<=num; ++i) {
            ang += step;
            double x2 = cx + rx * cos(ang);
            double y2 = cy + ry * sin(ang);
            DrawSmoothLine(x1,y1,x2,y2);
            x1 = x2;
            y1 = y2;
        }
    } else {
#endif
	    XDrawArc(mDpy,mDrawable,mGC,cx-rx, cy-ry, 2*rx, 2*ry, (int)(ang1 * 64), (int)(ang2 * 64));
#ifdef SMOOTH_LINES
    }
#endif
}

void PDrawXPixmap::FillArc(int cx,int cy,int rx,int ry,float ang1,float ang2)
{
#ifdef SMOOTH_LINES
    if (IsSmoothLines()) {
        XPointDouble poly[kMaxArcPoints];
        int num = rx + ry;
        if (num < kMinArcPoints) num = kMinArcPoints;
        if (num > kMaxArcPoints) num = kMaxArcPoints;
        double ang = 0;
        double step = 2 * PI / num;
        double rxd = rx + 0.5;
        double ryd = ry + 0.5;
        double cxd = cx + 0.5;
        double cyd = cy + 0.5;
        for (int i=0; i<num; ++i, ang+=step) {
            poly[i].x = cxd + rxd * cos(ang);
            poly[i].y = cyd + ryd * sin(ang);
        }
        XRenderCompositeDoublePoly(mDpy,
                              PictOpOver,
                              XftDrawSrcPicture(mXftDraw, &mXftColor),
                              mXftPicture,
                              XRenderFindStandardFormat(mDpy,PictStandardA8),
                              0, 0, 0, 0, poly, num, EvenOddRule);
    } else {
#endif
	    XFillArc(mDpy,mDrawable,mGC,cx-rx, cy-ry, 2*rx+1, 2*ry+1, (int)(ang1 * 64), (int)(ang2 * 64));
#ifdef SMOOTH_LINES
    }
#endif
}

void PDrawXPixmap::PutImage(XImage *image, int dest_x, int dest_y)
{
	XPutImage(mDpy,mDrawable,mGC,image,0,0,dest_x,dest_y,image->width,image->height);
}

XImage*	PDrawXPixmap::GetImage(int x, int y, int width, int height)
{
	return(XGetImage(mDpy,mDrawable,x,y,width,height,AllPlanes,ZPixmap));
}

int PDrawXPixmap::CopyArea(int x,int y,int w,int h,Window dest)
{
	if (mPix) {
		XCopyArea(mDpy,mPix,dest,mGC,x,y,w,h,x,y);
		return(1);
	} else {
		return(0);
	}
}
