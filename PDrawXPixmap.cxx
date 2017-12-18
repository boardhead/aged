//==============================================================================
// File:        PDrawXPixmap.cxx
//
// Description: Drawing routines for X11 pixmap images
//
// Notes:       Supports anti-aliased drawing if ANTI_ALIAS is defined
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
/*
** X-window offscreen drawable - PH 09/22/99
*/
#include <stdio.h>
#include <string.h>
#ifdef ANTI_ALIAS
#include <math.h>
#endif
#include "PDrawXPixmap.h"
#include "PResourceManager.h"
#include "PUtils.h"
#include "colours.h"

#ifndef PI
#define PI              3.14159265358979324
#endif

#ifdef ANTI_ALIAS
const int kMinArcPoints = 8;
const int kMaxArcPoints = 120;
#endif

PDrawXPixmap::PDrawXPixmap(Display *dpy, GC gc, int depth, Widget w)
{
    mDpy = dpy;
    mGC  = gc;
    mDepth = depth;
    mAltWidget = w; // widget to draw into if we can't create pixmap
    mPix = 0;
    mWidth = 0;
    mHeight = 0;
#ifdef ANTI_ALIAS
    mLineWidth = 1.0;
    mXftDraw = mXftDrawPix = mXftDrawDpy = NULL;
    mXftFmt = XRenderFindStandardFormat(dpy,PictStandardA8);
#endif
}

PDrawXPixmap::~PDrawXPixmap()
{
    FreePixmap();
#ifdef ANTI_ALIAS
    if (mXftDrawPix) XftDrawDestroy(mXftDrawPix);
    if (mXftDrawDpy) XftDrawDestroy(mXftDrawDpy);
#endif
}

//---------------------------------------------------------------------------------------
// BeginDrawing
//
int PDrawXPixmap::BeginDrawing(int width, int height)
{
    int     sizeChanged = 0;
    
    if (mWidth != width || mHeight != height) {
        // free old pixmap since our size changed
        FreePixmap();
        sizeChanged = 1;
    }
    if (mPix) {
        mDrawable = mPix;
#ifdef ANTI_ALIAS
        mXftPicture = XftDrawPicture(mXftDraw = mXftDrawPix);
#endif
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
#ifdef ANTI_ALIAS
        mXftDrawPix = XftDrawCreate(mDpy, mDrawable, DefaultVisual(mDpy,DefaultScreen(mDpy)),
                      DefaultColormap(mDpy, DefaultScreen(mDpy)));
        mXftPicture = XftDrawPicture(mXftDraw = mXftDrawPix);
#endif
    }
    return(1);
}

void PDrawXPixmap::EndDrawing()
{
    // must draw directly to window since pixmap has already been copied
    mDrawable = XtWindow(mAltWidget);
#ifdef ANTI_ALIAS
    if (!mXftDrawDpy) {
        mXftDrawDpy = XftDrawCreate(mDpy, mDrawable, DefaultVisual(mDpy,DefaultScreen(mDpy)),
                    DefaultColormap(mDpy, DefaultScreen(mDpy)));
    }
    mXftPicture = XftDrawPicture(mXftDraw = mXftDrawDpy);
#endif
}

#ifdef ANTI_ALIAS
void PDrawXPixmap::SetXftColour(Pixel pixel, int alpha)
{
    if (mDpy) {
        XColor c;
        c.pixel = pixel;
        XQueryColor(mDpy, DefaultColormap(mDpy,DefaultScreen(mDpy)), &c);
        XRenderColor xrcolor;
        xrcolor.red   = c.red;
        xrcolor.green = c.green;
        xrcolor.blue  = c.blue;
        xrcolor.alpha = alpha;
        XftColorAllocValue(mDpy, DefaultVisual(mDpy,DefaultScreen(mDpy)),
                   DefaultColormap(mDpy, DefaultScreen(mDpy) ), &xrcolor, &mXftColor);
        mAlpha = alpha;
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
#ifdef ANTI_ALIAS
        if (mXftDrawPix) {
            XftDrawDestroy(mXftDrawPix);
            mXftDrawPix = NULL;
        }
#endif
    }
}

int PDrawXPixmap::HasPixmap()
{
    return(mPix != 0);
}

void PDrawXPixmap::SetForeground(int col_num, int alpha)
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
#ifdef ANTI_ALIAS
    SetXftColour(pixel, alpha);
#endif
}

void PDrawXPixmap::SetForegroundPixel(Pixel pixel, int alpha)
{
    XSetForeground(mDpy, mGC, pixel);
#ifdef ANTI_ALIAS
    SetXftColour(pixel, alpha);
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
#ifdef ANTI_ALIAS
    mLineWidth = width < 1 ? 1 : width;
#endif
}

void PDrawXPixmap::SetLineType(ELineType type)
{
    if (type == kLineTypeOnOffDash) {
        // (this will only work for non-anti-aliased lines)
        XSetLineAttributes(mDpy, mGC, mLineWidth, LineOnOffDash, CapRound, JoinMiter);
    }
}

void PDrawXPixmap::DrawRectangle(int x,int y,int w,int h)
{
    XDrawRectangle(mDpy, mDrawable, mGC, x, y, w, h);
}

void PDrawXPixmap::FillRectangle(int x,int y,int w,int h)
{
#ifdef ANTI_ALIAS
    if (IsSmoothLines() && mAlpha != 0xffff) {
        XPointDouble poly[4];
        XDouble x1 = x + 0.5;
        XDouble y1 = y + 0.5;
        XDouble x2 = x1 + w;
        XDouble y2 = y1 + h;

        poly[0].x = x1;  poly[0].y = y1;
        poly[1].x = x2;  poly[1].y = y1;
        poly[2].x = x2;  poly[2].y = y2;
        poly[3].x = x1;  poly[3].y = y2;

        XRenderCompositeDoublePoly(mDpy,
                                  PictOpOver,
                                  XftDrawSrcPicture(mXftDraw, &mXftColor),
                                  mXftPicture,
                                  mXftFmt,
                                  0, 0, 0, 0, poly, 4, EvenOddRule);
    } else {
#endif

    XFillRectangle(mDpy, mDrawable, mGC, x, y, w, h);

#ifdef ANTI_ALIAS
    }
#endif
}

// Draw line segments in current colour
// - set "smooth" to 0 to override smooth line setting for faster drawing
void PDrawXPixmap::DrawSegments(XSegment *segments, int num, int smooth)
{
#ifdef ANTI_ALIAS
    if (IsSmoothLines() && smooth) {
        XSegment *sp = segments;
        for (int i=0; i<num; ++i, ++sp) {
            if (sp->x1==sp->x2 || sp->y1==sp->y2) {
                XDrawLine(mDpy,mDrawable,mGC,sp->x1,sp->y1,sp->x2,sp->y2);
            } else {
                DrawSmoothLine(sp->x1, sp->y1, sp->x2, sp->y2);
            }
        }
    } else {
#endif

    XDrawSegments(mDpy,mDrawable,mGC,segments,num);

#ifdef ANTI_ALIAS
    }
#endif
}

void PDrawXPixmap::DrawPoint(int x, int y)
{
    XDrawPoint(mDpy,mDrawable,mGC,x,y);
}

#ifdef ANTI_ALIAS
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
                              mXftFmt,
                              0, 0, 0, 0, poly, 4, EvenOddRule);
}
#endif

void PDrawXPixmap::DrawLine(int x1,int y1,int x2,int y2)
{
#ifdef ANTI_ALIAS
    if (x1==x2 || y1==y2) {
#endif

    XDrawLine(mDpy,mDrawable,mGC,x1,y1,x2,y2);

#ifdef ANTI_ALIAS
    } else {
        DrawSmoothLine(x1, y1, x2, y2);
    }
#endif
}

void PDrawXPixmap::FillPolygon(XPoint *point, int num)
{
// Either the OS X version of XRenderCompositeDoublePoly is buggy, or there is
// some unknown restriction on the coordinates, because it will crash when drawing
// some polygons which look like they have perfectly good coordinates,
// eg. (340,276)-(257,381)-(256,383)-(339,278)
#ifdef ANTI_ALIAS_BUGGY
    if (IsSmoothLines()) {
        XPointDouble *poly = new XPointDouble[num*4];
        for (int i=0; i<num; ++i) {
            poly[i].x = point[i].x;
            poly[i].y = point[i].y;
        }
        XRenderCompositeDoublePoly(mDpy,
                              PictOpOver,
                              XftDrawSrcPicture(mXftDraw, &mXftColor),
                              mXftPicture,
                              mXftFmt,
                              0, 0, 0, 0, poly, num, EvenOddRule);
        delete [] poly;
    } else {
#endif

// this works, but is limited to only 4-point polygons and it doesn't
// make things look any better with the current Aged drawing scheme
#ifdef ANTI_ALIAS_USELESS
    if (IsSmoothLines() && num == 4) {
        XTriangle tri[2];
        tri[0].p1.x = tri[1].p2.x = XDoubleToFixed(point[0].x);
        tri[0].p1.y = tri[1].p2.y = XDoubleToFixed(point[0].y);
        tri[0].p2.x = tri[1].p1.x = XDoubleToFixed(point[2].x);
        tri[0].p2.y = tri[1].p1.y = XDoubleToFixed(point[2].y);
        tri[0].p3.x = XDoubleToFixed(point[1].x);
        tri[0].p3.y = XDoubleToFixed(point[1].y);
        tri[1].p3.x = XDoubleToFixed(point[3].x);
        tri[1].p3.y = XDoubleToFixed(point[3].y);
        XRenderCompositeTriangles(mDpy,
                              PictOpOver,
                              XftDrawSrcPicture(mXftDraw, &mXftColor),
                              mXftPicture,
                              mXftFmt,
                              0, 0, tri, 2);
    } else {
#endif

    XFillPolygon(mDpy,mDrawable,mGC,point,num, Convex, CoordModeOrigin);

#if defined(ANTI_ALIAS_BUGGY) || defined(ANTI_ALIAS_USELESS)
    }
#endif
}

void PDrawXPixmap::DrawString(int x, int y, char *str, ETextAlign_q align)
{
    int len = strlen(str);

    switch (align / 3) {
        case 0:     // top
            y += GetFontAscent();
            break;
        case 1:     // middle
            y += GetFontAscent() / 2;
            break;
        case 2:     // bottom
            break;
    }
#ifdef ANTI_ALIAS
    if (IsSmoothText()) {
        XGlyphInfo  extents;
        if (align % 3) {
            XftTextExtents8(mDpy, GetXftFont(), (XftChar8 *)str, len, &extents);
        }
        switch (align % 3) {
            case 0:     // left
                break;
            case 1:     // center
                x -= extents.width / 2;
                break;
            case 2:     // right
                x -= extents.width;
                break;
        }
        XftDrawString8(mXftDraw, &mXftColor, GetXftFont(), x-1, y, (XftChar8 *)str, len);
    } else {
#endif

    if (GetFont()) {
        switch (align % 3) {
            case 0:     // left
                break;
            case 1:     // center
                x -= XTextWidth(GetFont(), str, len) / 2;
                break;
            case 2:     // right
                x -= XTextWidth(GetFont(), str, len);
                break;
        }
    }
    XDrawString(mDpy,mDrawable,mGC,x,y,str,len);

#ifdef ANTI_ALIAS
    }
#endif
}

int PDrawXPixmap::GetTextWidth(char *str)
{
    int width;
#ifdef ANTI_ALIAS
    if (IsSmoothText()) {
        XGlyphInfo    extents;
        XftTextExtents8(mDpy, GetXftFont(), (XftChar8 *)str, strlen(str), &extents );
        width = extents.width - extents.x;
    } else {
#endif
    width = XTextWidth(GetFont(), str, strlen(str));
#ifdef ANTI_ALIAS
    }
#endif
    return(width);
}

void PDrawXPixmap::DrawArc(int cx,int cy,int rx,int ry,float ang1,float ang2)
{
#ifdef ANTI_ALIAS
    if (IsSmoothLines()) {
        int num = rx + ry;
        if (num) {
            if (num < kMinArcPoints) num = kMinArcPoints;
            if (num > kMaxArcPoints) num = kMaxArcPoints;
            double ang = ang1 * PI / 180;
            double step = (ang2 * PI / 180 - ang) / num;
            double halfPix = 0.5 / sqrt(rx*rx + ry*ry);
            for (int i=1; i<=num; ++i) {
                double x1 = cx + rx * cos(ang - halfPix);
                double y1 = cy + ry * sin(ang - halfPix);
                ang += step;
                double x2 = cx + rx * cos(ang + halfPix);
                double y2 = cy + ry * sin(ang + halfPix);
                DrawSmoothLine(x1,y1,x2,y2);
            }
        }
    } else {
#endif

    XDrawArc(mDpy,mDrawable,mGC,cx-rx, cy-ry, 2*rx, 2*ry, (int)(ang1 * 64), (int)(ang2 * 64));

#ifdef ANTI_ALIAS
    }
#endif
}

void PDrawXPixmap::FillArc(int cx,int cy,int rx,int ry,float ang1,float ang2)
{
#ifdef ANTI_ALIAS
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
                              mXftFmt,
                              0, 0, 0, 0, poly, num, EvenOddRule);
    } else {
#endif

    XFillArc(mDpy,mDrawable,mGC,cx-rx, cy-ry, 2*rx+1, 2*ry+1, (int)(ang1 * 64), (int)(ang2 * 64));

#ifdef ANTI_ALIAS
    }
#endif
}

void PDrawXPixmap::PutImage(XImage *image, int dest_x, int dest_y)
{
    XPutImage(mDpy,mDrawable,mGC,image,0,0,dest_x,dest_y,image->width,image->height);
}

XImage* PDrawXPixmap::GetImage(int x, int y, int width, int height)
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
