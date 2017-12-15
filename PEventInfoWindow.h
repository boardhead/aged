//==============================================================================
// File:        PEventInfoWindow.h
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#ifndef __PEventInfoWindow_h__
#define __PEventInfoWindow_h__

#include <Xm/Xm.h>
#include "PWindow.h"
#include "PListener.h"
#include "PLabel.h"

class PEventInfoWindow : public PWindow, public PListener {
public:
    PEventInfoWindow(ImageData *data);
    
    virtual void    UpdateSelf();
    virtual void    Listen(int message, void *message_data);
    
private:
    PLabel          tw_evt, tw_nhit, tw_run, tw_tracks, tw_lines;
    PLabel          tw_helices, tw_vertexX, tw_vertexY, tw_vertexZ;
    
    int             mTimeZone;
};


#endif // __PEventInfoWindow_h__
