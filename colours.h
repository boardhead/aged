//==============================================================================
// File:        colours.h - Aged colour definitions
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
/*
** The basic Aged colours are defined in the enumerated type EColours below.
**
** The hit and detector colour numbers are special, since they both use variable
** length colour tables generated at run time.  The hit colours begin with
** colour number NUM_COLOURS, followed by the detector colours.
**
** When a new colour is added, a corresponding resource should be added to
** PResourceManager.cxx, and a new sColourName should be added to PColourPicker.cxx.
*/
#ifndef __COLOURS_H__
#define __COLOURS_H__

enum EColour {
    BKG_COL,
    TEXT_COL,
    HID_COL,
    FRAME_COL,
    VDARK_COL,
    VLIT_COL,
    AXES_COL,
    CURSOR_COL,
    SELECT_COL,
    VERTEX_COL,
    FIT_BAD_COL,
    FIT_GOOD_COL,
    FIT_SEED_COL,
    FIT_ADDED_COL,
    FIT_SECOND_COL,
    FIT_PHOTON_COL,
    WAVEFORM_COL,
    GRID_COL,
    SCALE_UNDER,
    SCALE_COL0,
    SCALE_COL1,
    SCALE_COL2,
    SCALE_COL3,
    SCALE_COL4,
    SCALE_OVER,
    NUM_COLOURS
};

#endif // __COLOURS_H__
