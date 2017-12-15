//==============================================================================
// File:        PProjImage.cxx
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#include <math.h>
#include "ImageData.h"
#include "PProjImage.h"
#include "PImageWindow.h"
#include "PResourceManager.h"
#include "PSpeaker.h"
#include "menu.h"

int PProjImage::sButtonDown = 0;


PProjImage::PProjImage(PImageWindow *owner, Widget canvas, EventMask eventMask)
          : PImageCanvas(owner,canvas,eventMask)
{
    mScaleProportional  = 1;
    mMarginPix          = 5;        // pixel margin
    mMarginFactor       = 1;
    mProj.proj_type     = -1;
    mImageSizeX         = 1.0;
    mImageSizeY         = 1.0;
    mMinMagAtan         = atan(0.5);
    mDiffMagAtan        = atan(10.0) - mMinMagAtan;
    mInvisibleHits      = 0;
    
    SetToHome();
}

PProjImage::~PProjImage()
{
    ImageData *data = mOwner->GetData();
    if (data) {
        if (data->mLastImage == this) {
            data->mLastImage = NULL;
        }
        if (data->mCursorImage == this) {
            data->mCursorImage = NULL;
        }
    }
}

void PProjImage::Listen(int message, void *dataPt)
{
    switch (message) {
        case kMessageNewEvent:
        case kMessageEventCleared:
        case kMessageColoursChanged:
        case kMessageHitsChanged:
            SetDirty();
            break;
        case kMessageCursorHit:
            SetDirty(kDirtyCursor);
            break;
        default:
            PImageCanvas::Listen(message, dataPt);
            break;
    }
}

/* draw projection angles */
void PProjImage::DrawAngles(int horiz,int angleFlags)
{
    int         len, y;
    ImageData * data = mOwner->GetData();
    
    if (data->angle_rad > 1) return;
    
    SetForeground(TEXT_COL);
    SetFont(PResourceManager::sResource.hist_font);
#ifdef ANTI_ALIAS
    SetFont(PResourceManager::sResource.xft_hist_font);
#endif

    char buff[128];
    if (horiz) {
        len = 0;
        if (angleFlags & kAngleTheta) {
            if (data->angle_rad) {
                len += sprintf(buff+len,"T=%.3f rad", mProj.theta+PI/2);
            } else {
                len += sprintf(buff+len,"T=%.1f\xb0",(mProj.theta+PI/2) * data->angle_conv);
            }
        }
        if (angleFlags & kAnglePhi) {
            if (len) len += sprintf(buff+len,"  ");
            if (data->angle_rad) {
                len += sprintf(buff+len,"P=%.3f rad", mProj.phi);
            } else {
                len += sprintf(buff+len,"P=%.1f\xb0", mProj.phi*data->angle_conv);
            }
        }
        if (angleFlags & kAngleGamma) {
            if (len) len += sprintf(buff+len,"  ");
            if (data->angle_rad) {
                len += sprintf(buff+len,"G=%.3f rad", mProj.gamma);
            } else {
                len += sprintf(buff+len,"G=%.1f\xb0", mProj.gamma*data->angle_conv);
            }
        }
        DrawString(mWidth-14*GetScaling(),3*GetScaling(),buff,kTextAlignTopRight);
    } else {
        y = 5 * GetScaling();
        if (angleFlags & kAngleTheta) {
            if (data->angle_rad) {
                len = sprintf(buff,"T=%.3f rad",mProj.theta+PI/2);
            } else {
                len = sprintf(buff,"T=%.1f\xb0",(mProj.theta+PI/2)*data->angle_conv);
            }
            DrawString(mWidth-8*GetScaling(), y, buff, kTextAlignTopRight);
            y += GetScaling() * (GetFontAscent() + GetFontDescent());
        }
        if (angleFlags & kAnglePhi) {
            if (data->angle_rad) {
                len = sprintf(buff,"P=%.3f rad",mProj.phi);
            } else {
                len = sprintf(buff,"P=%.1f\xb0",mProj.phi*data->angle_conv);
            }
            DrawString(mWidth-8*GetScaling(), y, buff, kTextAlignTopRight);
            y += GetScaling() * (GetFontAscent() + GetFontDescent());
        }
        if (angleFlags & kAngleGamma) {
            if (data->angle_rad) {
                len = sprintf(buff,"G=%.3f rad",mProj.gamma);
            } else {
                len = sprintf(buff,"G=%.1f\xb0",mProj.gamma*data->angle_conv);
            }
            DrawString(mWidth-8*GetScaling(), y, buff, kTextAlignTopRight);
        }
    }
}


/* set projection to home (starting) position */
void PProjImage::SetToHome(int n)
{
    matrixIdent(mProj.rot);
    
    mProj.proj_max      = 1e10;
    mProj.proj_screen   = -1.0;
    mProj.proj_min      = -0.99;
    mProj.pt[0]         = mProj.pt[1] = 0;
    mProj.pt[2]         = mProj.proj_max;
    mProj.mag           = 1.;
    mProj.theta         = 0.;
    mProj.phi           = 0.;
    mProj.xscl          = 200;
    mProj.yscl          = 200;
    
    Resize();   // calculate appropriate scaling factors
}

void PProjImage::TransformHits()
{
    ImageData   *data = mOwner->GetData();
    
    // save pointer to this object so we know who was last to transform the hits
    data->mLastImage = this;
    
    // find nearest hit if we are the cursor image and updating all windows
    if (data->mCursorImage == this && !sButtonDown) {
        FindNearestHit();
    }
}

long PProjImage::HiddenHitMask()
{
    return(mInvisibleHits | mOwner->GetData()->bit_mask);
}

/* set data->cursor_hit to the index of the hit closest to the current cursor location */
/* Returns non-zero if cursor hit changes */
int PProjImage::FindNearestHit()
{
    ImageData       *data = mOwner->GetData();

    if (data->mLastImage != this) {
        TransformHits();    // must transform hits to this projection
    }
    
    int         num = -1;
    int         i,t,d,dx,dy;
    Node        *node;
    HitInfo     *hi;
    int         x = data->last_cur_x;
    int         y = data->last_cur_y;

    if (data->hits.num_nodes) {
        d    = 1000000;
        node = data->hits.nodes;
        hi   = data->hits.hit_info;
        for (i=0; i<data->hits.num_nodes; ++i,++node, ++hi) {
            if (hi->flags & (data->bit_mask | mInvisibleHits)) continue;    /* only consider unmasked ncds */
            dx = x - node->x;
            dy = y - node->y;
            t = dx*dx + dy*dy;
            if (t <= d) {           /* find closest node    */
                num = i;
                d = t;
            }
        }
        /* don't show hit info if too far away from hit (16 pixels) */
        if (d > 256) {
            num = -1;
        }
        if (data->cursor_hit != num) {
            data->cursor_hit = num;
            if (num < 0) data->cursor_sticky = 0;
            sendMessage(data, kMessageCursorHit, this);
            return(1);
        }
    }
    return(0);
}

/* Handle mouse button 3 - use to deselect events for fitting */
int PProjImage::HandleButton3(XEvent *event)
{
    if (event->type==ButtonPress && event->xbutton.button==Button3) {
        ImageData *data = mOwner->GetData();
        if (data->cursor_hit >= 0) {
            HitInfo *hi = data->hits.hit_info + data->cursor_hit;
            hi->flags ^= HIT_DISCARDED;     // toggle discarded flag
            // inform listeners that the hit discarded flag has changed
            sendMessage(data, kMessageHitDiscarded,this);
            // redraw images
            calcHitVals(data);
            sendMessage(data, kMessageHitsChanged);
        }
        return(1);
    } else {
        return(0);
    }
}

/* generic event handler for projection images */
/* motion - finds nearest hit and sends kMessageCursorHit messages if it changes */
/* grabs - move projection center */
void PProjImage::HandleEvents(XEvent *event)
{
    ImageData       *data = mOwner->GetData();
    static int      last_x, last_y;
    static float    xc, yc;
    static int      didDrag = 0;

    switch (event->type) {
    
        case kTimerEvent:
            SetCursor(CURSOR_MOVE_4);
            didDrag = 1;
            break;

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
                // delay activating the drag to allow a quick press
                last_x = event->xbutton.x;
                last_y = event->xbutton.y;
                didDrag = 0;
                ArmTimer();
                xc = mProj.pt[0] + (last_x - mProj.xcen)/(float)mProj.xscl;
                yc = mProj.pt[1] - (last_y - mProj.ycen)/(float)mProj.yscl;
            }
            break;
                
        case ButtonRelease:
            if (sButtonDown == (int)event->xbutton.button) {
                ResetTimer();
                if (!didDrag) {
                    int oldSticky = data->cursor_sticky;
                    int oldCursor = data->cursor_hit;
                    data->cursor_sticky ^= 1;
                    data->cursor_hit = -1;
                    data->last_cur_x = event->xmotion.x;
                    data->last_cur_y = event->xmotion.y;
                    FindNearestHit();
                    if (data->cursor_hit == -1) data->cursor_sticky = 0;
                    if (oldCursor >= 0 && oldCursor == data->cursor_hit &&
                        !data->cursor_sticky && oldSticky)
                    {
                        sendMessage(data, kMessageAddOverlay);
                    }
                    sendMessage(data, kMessageCursorHit, this);
                }
                SetCursor(CURSOR_XHAIR);
                XUngrabPointer(data->display, CurrentTime);
                sButtonDown = 0;
            }
            break;

        case MotionNotify:
            if (!sButtonDown) {
                SetCursorForPos(event->xbutton.x, event->xbutton.y);
                /* save last motion coordinates */
                data->last_cur_x = event->xmotion.x;
                data->last_cur_y = event->xmotion.y;
                data->mCursorImage = this;
                /* find the hit nearest the new cursor location */
                if (!data->cursor_sticky) FindNearestHit();
                break;
                
            }
            
            if (event->xmotion.is_hint) break;

            if (!didDrag) {
                int dx = last_x - event->xbutton.x;
                int dy = last_y - event->xbutton.y;
                // must surpass motion threshold before activating grab
                if (dx>-4 && dx<4 && dy>-4 && dy<4) break;
                SetCursor(CURSOR_MOVE_4);
                didDrag = 1;
                ResetTimer();
            }
            if (last_x != event->xbutton.x ||
                last_y != event->xbutton.y)
            {
                last_x = event->xbutton.x;
                last_y = event->xbutton.y;
            
                float xn = mProj.pt[0] + (last_x - mProj.xcen)/(float)mProj.xscl;
                float yn = mProj.pt[1] - (last_y - mProj.ycen)/(float)mProj.yscl;

                mProj.pt[0] += xc - xn;
                mProj.pt[1] += yc - yn;
                        
                SetDirty();
                SetScrolls();
            }
            break;
            
        case LeaveNotify:
            ResetTimer();
            if (data->mCursorImage == this) {
                data->mCursorImage = NULL;
            }
        /*  if (data->cursor_hit >= 0) {
                data->cursor_hit = data->cursor_pmt = -1;
                data->cursor_sticky = 0;
                sendMessage(data, kMessageCursorHit, this);
            }*/
            break;
    }
}

void PProjImage::SetScrolls()
{
    int         pos;

    pos = kScrollMax - (int)(kScrollMax * (atan(mProj.mag) - mMinMagAtan) / mDiffMagAtan + 0.5);
    mOwner->SetScrollValue(kScrollLeft, pos);
    pos = (kScrollMax/2) + (int)(kScrollMax/2 * mProj.pt[0] / mImageSizeX);
    mOwner->SetScrollValue(kScrollBottom, pos);
    pos = (kScrollMax/2) - (int)(kScrollMax/2 * mProj.pt[1] / mImageSizeY);
    mOwner->SetScrollValue(kScrollRight, pos);
}

void PProjImage::ScrollValueChanged(EScrollBar bar, int value)
{
    switch (bar) {
        case kScrollLeft:
            mProj.mag = tan(mMinMagAtan + mDiffMagAtan * (kScrollMax - value) / kScrollMax);
            Resize();
            SetDirty();
            break;
        case kScrollBottom:
            mProj.pt[0] = 2 * (value-(kScrollMax/2))/(float)(kScrollMax / mImageSizeX);
            SetDirty();
            break;
        case kScrollRight:
            mProj.pt[1] = -2 * (value-(kScrollMax/2))/(float)(kScrollMax / mImageSizeY);
            SetDirty();
            break;
        default:
            break;
    }
}

void PProjImage::Resize()
{
    mProj.xsiz = mWidth;
    mProj.ysiz = mHeight;

    mProj.xcen = mWidth / 2;
    mProj.ycen = mHeight / 2;
    
    float scaling;
    if (mDrawable) {
        scaling = mDrawable->GetScaling();
    } else {
        scaling = 1;
    }
    
    // calculate an xsiz and ysiz based on the image size
    float xsiz = (mProj.xsiz - 2 * mMarginPix * scaling) / (mImageSizeX * mMarginFactor);
    float ysiz = (mProj.ysiz - 2 * mMarginPix * scaling) / (mImageSizeY * mMarginFactor);

    if (mScaleProportional) {
        mProj.xscl = (int)(0.5 * mProj.mag * (xsiz<ysiz ? xsiz : ysiz));
        // limit minimum scale
        if (mProj.xscl < 1) mProj.xscl = 1;
        mProj.yscl = mProj.xscl;
    } else {
        mProj.xscl = (int)(0.5 * mProj.mag * xsiz);
        if (mProj.xscl < 1) mProj.xscl = 1;
        mProj.yscl = (int)(0.5 * mProj.mag * ysiz);
        if (mProj.yscl < 1) mProj.yscl = 1;
    }
    
    SetDirty();
}
