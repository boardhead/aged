//==============================================================================
// File:        PMapImage.h
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#ifndef __PMapImage_h__
#define __PMapImage_h__

#include "PProjImage.h"
#include "PMenu.h"
#include "matrix.h"

struct MenuStruct;

class PMapImage : public PProjImage, public PMenuHandler {
public:
    PMapImage(PImageWindow *owner, Widget canvas=0);
    virtual ~PMapImage();
    
    virtual void    DrawSelf();
    virtual void    AfterDrawing();
    
    virtual void    Listen(int message, void *dataPt);
    virtual void    SetScrolls();
    virtual void    ScrollValueChanged(EScrollBar bar, int value);

    virtual void    TransformHits();

    virtual void    DoMenuCommand(int anID);
    
    void            AddMenuItem();
    void            SetProjection(int proj_type);
    void            TransformHits(Vector3 vec, Matrix3 rot1);
    void            CalcTransformMatrix();
    
private:
    XSegment      * AddProjLine(XSegment *sp,XSegment *segments,Node *n0,Node *n1,
                                Vector3 vec,Matrix3 rot1,Projection *proj,int nsplit);
                                
    int             mShapeOption;       // shape menu option
    int             mProjType;          // projection type
    int             mSplitThreshold;    // maximum times to split a line for drawing fit
    Vector3         mVec;               // map center position
    Matrix3         mRot1;              // map rotation matrix
};

#endif // __PMapImage_h__
