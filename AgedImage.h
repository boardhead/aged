//==============================================================================
// File:        AgedImage.h
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#ifndef __AgedImage_h__
#define __AgedImage_h__
 
#include "ImageData.h"
#include "PProjImage.h"

enum AgedImageDirtyFlags {
    kDirtyHits      = 0x02,
    kDirtyFit       = 0x04,
    kDirtyDetector  = 0x08,
    kDirtyFrame     = 0x10,
    kDirtyAxes      = 0x20,
    
    kDirtyAll       = 0xfe
};

class AgedImage : public PProjImage
{
public:
    AgedImage(PImageWindow *owner, Widget canvas=0);
    virtual ~AgedImage();
            
    virtual void    Resize();
    virtual void    DrawSelf();
    virtual void    AfterDrawing();
    virtual void    HandleEvents(XEvent *event) ;
    virtual void    Transform(Node *node, int num_nodes) { transform(node, &mProj, num_nodes); }
    virtual void    TransformHits();
    virtual void    Listen(int message, void *dataPt);
    
    virtual void    SetScrolls();
    virtual void    ScrollValueChanged(EScrollBar bar, int value);

    virtual void    SetToHome(int n=0);
    
private:
    void            CalcGrab2(int x,int y);
    void            CalcGrab3(int x,int y);
    void            CalcDetectorShading();
    void            RotationChanged();
    
    Polyhedron          mDet;                   // detector geometry
    WireFrame           mAxes;                  // coordinate axes
    float               mSpinAngle;             // 'Josh' bar spin angle
    double              mMinMagAtan;            // arctan of minimum magnification
    double              mMaxMagAtan;            // arctan of maximum magnification
    float               mHitSize;               // last used size of PMT hexagon
    float               mGrabX,mGrabY,mGrabZ;   // 3-D mouse cursor coordinates for grab
};


#endif // __AgedImage_h__
