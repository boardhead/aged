//==============================================================================
// File:        menu.h
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#ifndef __MENU_H__
#define __MENU_H__

#define MAX_EXTRA_NUM       100 // max 100 extra hit data types

/*
** Menu ID definitions
*/
enum {
    // popup window ID's (note: no #ifdef's here or resources get misaligned!)
    EVT_NUM_WINDOW = 1,
    EVT_INFO_WINDOW,
    HIST_WINDOW,
    WAVE_WINDOW,
    PROJ_WINDOW,
    SCALE_WINDOW,
    SETTINGS_WINDOW,
    PRINT_WINDOW,
    HIT_INFO_WINDOW,    // Note: this must come after all projection windows
                        // for hit information to be available when window is drawn
    COLOUR_WINDOW,
    NUM_WINDOWS,
    
    // menu item ID's
    IDM_WHITE_BKG = 100,
    IDM_GREYSCALE,
    IDM_PRINT_WINDOW,
    IDM_PRINT_IMAGE,
    IDM_ABOUT,
    IDM_QUIT,
    IDM_SAVE_SETTINGS,
    IDM_CLEAR_EVENT,
    IDM_MOVE_HOME,
    IDM_MOVE_TOP,
    IDM_CUT_NORMAL,
    IDM_CUT_OVERSCALE,
    IDM_CUT_UNDERSCALE,
    IDM_TIME,
    IDM_HEIGHT,
    IDM_ERROR,
    IDM_DISP_dummy,
    IDM_DISP_WIRE,
    IDM_DISP_PAD,
    IDM_NEXT_EVENT,
    IDM_NEXT_SPCPT,
    IDM_PREV_SPCPT,
    IDM_MOVE_SUN,
    IDM_DETECTOR,
    IDM_FIT,
    IDM_SP_ERRORS,
    IDM_SP_SQUARES,
    IDM_SP_CIRCLES,
    IDM_SP_NONE,
    IDM_PROJ_TO_HOME,
    IDM_PROJ_3D,
    IDM_PROJ_RECTANGULAR,
    IDM_PROJ_LAMBERT,
    IDM_PROJ_SINUSOID,
    IDM_PROJ_ELLIPTICAL,
    IDM_PROJ_MOLLWEIDE,
    IDM_PROJ_HAMMER,
    IDM_PROJ_EXTENDED_HAMMER,
    IDM_PROJ_POLAR,
    IDM_PROJ_POLAR_EQUAL,
    IDM_PROJ_dummy,
    IDM_PROJ_DUAL_SINUSOID,
    IDM_PROJ_DUAL_ELLIPTICAL,
    IDM_PROJ_DUAL_MOLLWEIDE,
    IDM_PROJ_DUAL_HAMMER,
    IDM_PROJ_DUAL_POLAR,
    IDM_PROJ_DUAL_POLAR_EQUAL,
    IDM_WAVE_ADD_OVERLAY,
    IDM_WAVE_CLEAR_OVERLAY,
    IDM_HIT_SQUARE,
    IDM_HIT_CIRCLE,
    IDM_DATA_MENU,
};

// constants used to range check menu radio settings
#define IMAX_DATATYPE       (IDM_DISP_PAD - IDM_TIME)
#define IMAX_PROJTYPE       (IDM_PROJ_DUAL_POLAR_EQUAL - IDM_PROJ_RECTANGULAR)
#define IMAX_SHAPEOPTION    (IDM_HIT_CIRCLE - IDM_HIT_SQUARE)

#endif // __MENU_H__
