//==============================================================================
// File:        AgedResource.h
//
// Copyright (c) 2017, Phil Harvey, Queen's University
//==============================================================================
#ifndef __AgedResource_h__
#define __AgedResource_h__

#include <Xm/Xm.h>
#ifdef ANTI_ALIAS
#include <X11/Xft/Xft.h>
#endif
#include "colours.h"
#include "menu.h"
#include "PProjImage.h"

// cursor indices
enum ECursorType {
    CURSOR_MOVE_4,
    CURSOR_EXCHANGE,
    CURSOR_MOVE_H,
    CURSOR_MOVE_V,
    CURSOR_XHAIR,
    CURSOR_ARROW_DOWN,
    CURSOR_ARROW_UP,
    NUM_CURSORS
};

enum ETimeZone {
    kTimeZonePST     = 0,
    kTimeZoneLocal   = 1,
    kTimeZoneUTC     = 2
};

enum EGotoBy {
    kGotoEvID,
    kGotoRun,
    kGotoTime
};

const short kCaenRsrcNum = 8;           // number of CAEN channels saved
const int   kMaxWaveformChannels = 8;   // maximum number of channels in the waveform window


// masks for colour set bits
const short     kWhiteBkg   = 0x01;
const short     kGreyscale  = 0x02;

// definititions for start of hit and detector colour numbers
#define FIRST_SCALE_COL     (NUM_COLOURS)
#define FIRST_DET_COL       (NUM_COLOURS + PResourceManager::sResource.num_cols)

/* AgedResource structure definition */
struct AgedResource {
    float           resource_version;           // resource file version number
    float           time_min, time_max;         // range for raw TIME colour scale
    float           height_min, height_max;     // range for pulse heights
    float           error_min, error_max;       // range for error values
    int             hist_bins;                  // number of histogram bins
    int             open_windows;               // bit mask for windows to open at startup
    int             open_windows2;              // bit mask for windows 32-63
    Pixel           colour[NUM_COLOURS];        // sys colour numbers for drawing
    Pixel           colset[2][NUM_COLOURS];     // colour sets
    Pixel         * scale_col;                  // scale colours
    Pixel         * det_col;                    // detector colours
    Pixel           black_col;                  // constant black colour
    Pixel           white_col;                  // constant white colour
    int             num_cols;                   // number of colours in hit colour scale
    int             det_cols;                   // number of colours in detector colour scale
    Projection      proj;                       // current projection
    float           time_interval;              // time interval for displayed events
    XFontStruct   * hist_font;                  // font for histograms
    XFontStruct   * label_font;                 // font for image labels
    XFontStruct   * label_big_font;             // big label font
#ifdef ANTI_ALIAS
    XftFont       * xft_hist_font;              // X FreeType fonts
    XftFont       * xft_label_font;
    XftFont       * xft_label_big_font;
    char          * xft_hist_font_str;          // X FreeType font names
    char          * xft_label_font_str;
    char          * xft_label_big_font_str;
#endif
    int             hex_id;                     // flag to display event ID's in hex
    int             smooth;                     // antialiasing flag 0x01=text, 0x02=lines
    int             time_zone;                  // time zone: 0=local, 1=UTC, 2=PST
    int             angle_rad;                  // flag to display angles in radians
    int             hit_xyz;                    // flag to display hit xyz coordinates
    int             log_scale;                  // flag for log histogram scale
    float           hit_size;                   // hit size multiplier
    float           fit_size;                   // fit size multiplier
    char          * file_path;                  // pointer to alternate search path
    char          * print_string_pt[2];         // pointer to print command/filename
    char          * label_format_pt;            // pointer to event label format string
    Cursor          cursor[NUM_CURSORS];        // cursors
    XtAppContext    the_app;                    // the application context
    Display       * display;                    // the X display
    GC              gc;                         // the X graphics context
    int             dataType;                   // index for data type menu item
    int             spType;                     // drawing style for space points
    int             projType;                   // index for projection menu item
    int             bit_mask;                   // bitmask for hidden hits
    int             show_detector;              // flag to display detector
    int             show_fit;                   // flag to display fit vertex/lines/helices
    int             image_col;                  // index for image colour scheme
    int             print_to;                   // print destination (0=printer, 1=file)
    int             print_col;                  // flag for print colours
    int             print_label;                // flag to print labels
    int             show_label;                 // flag to show event label
    int             shapeOption;                // index for hit shape menu item
    char          * version;                    // Aged version that wrote the resources
    int             save_config;                // flag to save settings on quit
    int             wave_min[kMaxWaveformChannels]; // waveform Y scale minimum
    int             wave_max[kMaxWaveformChannels]; // waveform Y scale maximum
};

typedef AgedResource *AgedResPtr;

#endif // __AgedResource_h__
