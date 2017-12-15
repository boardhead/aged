//==============================================================================
// File:        PDrawXPixmap.h
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
/*
** X-window offscreen drawable - PH 09/22/99
*/
#ifndef __PDrawXPixmap_h__
#define __PDrawXPixmap_h__

#include "PDrawable.h"

class PDrawXPixmap : public PDrawable
{
public:
    PDrawXPixmap(Display *dpy, GC gc, int depth, Widget w=NULL);
    virtual ~PDrawXPixmap();
    
    virtual int     BeginDrawing(int width,int height);
    virtual void    EndDrawing();
    
    virtual void    SetForeground(int col_num, int alpha=0xffff);
    virtual void    SetForegroundPixel(Pixel pixel, int alpha=0xffff);
    virtual void    SetFont(XFontStruct *font);
    virtual void    SetLineWidth(float width);
    virtual void    SetLineType(ELineType type);
    virtual int     GetTextWidth(char *str);
    virtual void    DrawSegments(XSegment *segments, int num, int smooth=1);
    virtual void    DrawPoint(int x,int y);
    virtual void    DrawLine(int x1,int y1,int x2,int y2);
    virtual void    DrawRectangle(int x,int y,int w,int h);
    virtual void    FillRectangle(int x,int y,int w,int h);
    virtual void    FillPolygon(XPoint *point, int num);
    virtual void    DrawString(int x, int y, char *str,ETextAlign_q align);
    virtual void    DrawArc(int cx,int cy,int rx,int ry,float ang1,float ang2);
    virtual void    FillArc(int cx,int cy,int rx,int ry,float ang1,float ang2);

    virtual void    PutImage(XImage *image, int dest_x, int dest_y);
    virtual XImage* GetImage(int x, int y, int width, int height);  
    virtual int     CopyArea(int x,int y,int w,int h,Window dest);
    virtual int     HasPixmap();

    virtual EDevice GetDeviceType()     { return kDeviceVideo; }        
        
private:
    void            CreatePixmap();
    void            FreePixmap();
    
    Pixmap          mPix;               // offscreen pixmap for drawing
    Display       * mDpy;               // X display for drawing
    GC              mGC;                // X graphics context for drawing
    Widget          mAltWidget;         // widget to draw into if pixmap not available
    Drawable        mDrawable;          // the X drawable
    int             mDepth;             // depth of screen
    int             mWidth;             // pixmap width
    int             mHeight;            // pixmap height

#ifdef ANTI_ALIAS
    void            DrawSmoothLine(double x1, double y1, double x2, double y2);
    void            SetXftColour(Pixel pixel, int alpha);

    XftDraw       * mXftDraw;           // current Xft drawable
    XftDraw       * mXftDrawPix;        // pixmap Xft drawable
    XftDraw       * mXftDrawDpy;        // display Xft drawable
    XftColor        mXftColor;          // current drawing colour (both drawables)
    XRenderPictFormat *mXftFmt;         // render format
    Picture         mXftPicture;        // current Xft picture
    double          mLineWidth;         // current drawing line width
    int             mAlpha;             // current drawing alpha
#endif
};


#endif // __PDrawXPixmap_h__
