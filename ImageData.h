#ifndef __ImageData_h__
#define __ImageData_h__

#include "AgedResource.h"
#include "AgedWindow.h"
#include "PSpeaker.h"
#include "PProjImage.h"
#include "PHistImage.h"

#define MAX_FNODES		4               // maximum number of nodes per face
#define	NODE_HID		0x01            // node is hidden
#define	NODE_OUT		0x02            // node is outside plotting area
#define	NODE_MISSED		0x04			// node in cherenkov cone missed intersecting sphere
#define EDGE_HID		0x01            // edge is hidden
#define FACE_HID		0x01            // face is hidden
#define FACE_COL_SHFT	2				// bit shift for colour in face flags

#define FORMAT_LEN		512				// maximum length of event label
#define FILELEN			4096			// maximum length of file name

#define AG_SCALE        150.0           // size of Alpha-G geometry (units?)

#ifndef PI
#define	PI				3.14159265358979324
#endif

#define MAX_EDGES		350
#define NUM_AG_WIRES    256             //TEST
#define NUM_AG_PADS     (32 * 576)      //TEST

const int kMaxWaveformChannels = 8;

enum HitInfoFlags {
	HIT_NORMAL 		= 0x01,
	HIT_ALL_MASK    = HIT_NORMAL,
	HIT_DISCARDED	= 0x100,
	HIT_OVERSCALE	= 0x200,
	HIT_UNDERSCALE	= 0x400,
	HIT_HIDDEN		= 0x800
};

enum ESmoothFlags {
    kSmoothText     = 0x01,
    kSmoothLines    = 0x02
};

struct Point3 {
	float		x,y,z;
};

struct Node {
	float		x3,y3,z3;			// physical sphere coordinates (radius=1)
	float		xr,yr,zr;			// rotated physical coordinates
	int			x,y;				// screen coordinates after rotating
	int			flags;				// flag for x,y outside limits
};

struct Face {
	int			num_nodes;
	Node		*nodes[MAX_FNODES];
	Point3		norm;				// unit vector normal to face
	int			flags;
};

struct Edge {
	Node		*n1, *n2;			// nodes for endpoints of edge
	Face		*f1, *f2;			// associated faces
	int			flags;				// used to indicate a hidden line
};

struct WireFrame {
	int			num_nodes;
	Node		*nodes;
	int			num_edges;
	Edge		*edges;
};

struct Polyhedron {
	int			num_nodes;
	Node		*nodes;
	int			num_edges;
	Edge		*edges;
	int			num_faces;
	Face		*faces;
	float		radius;
};

struct HitInfo {
	float		height;				// pulse height
	float		time;				// pulse time
	float       error[3];           // error in XYZ position
	short		hit_val;			// colour index for drawing this hit
	short		index;				// index of hi in position array
	int 		wire;				// wire number
	int 		pad;				// pad number
	short		flags;				// hit info flags
};

struct SpacePoints {
	int			num_nodes;
	Node		*nodes;
	HitInfo		*hit_info;			// corresponding array of hit information
};

class TStoreEvent;
class AgAnalysisFlow;
class AgSignalsFlow;

struct ImageData : AgedResource {
	AgedWindow    *	mMainWindow;		// main Aged window
	PWindow		  * mWindow[NUM_WINDOWS];// Aged windows
	PSpeaker	  *	mSpeaker;			// broadcasts messages to listeners
	MenuStruct	  *	main_menu;			// pointer to menu struct
	PProjImage	  *	mLastImage;			// last projection image to transform hits
	PProjImage	  *	mCursorImage;		// last image to be cursor'd in
	PHistImage	  *	mScaleHist;			// histogram image currently being scaled
	int             mNext;              // true to step to next event (exit event loop)

    TStoreEvent   * agEvent;            // the event we are displaying
    AgAnalysisFlow* anaFlow;            // the analysis flow
    AgSignalsFlow * sigFlow;            // the signals flow

	Widget			toplevel;			// top level Aged widget
	SpacePoints		hits;				// tube hit information
	
	Node			sun_dir;			// direction to sun
	int				num_disp;			// number of displayed hits
	float			angle_conv;			// conversion from radians to displayed angle units

	// menu ID (IDM_) variables
	int				wDataType;			// flag for displayed data type
	int				wSpStyle;			// flag for displayed space point style
	int				wProjType;			// current projection type
	int				wShapeOption;		// current projection hit shape option

	char		  *	projName;			// name of current projection
	char		  *	dispName;			// name of data display type
	int				cursor_hit;			// hit index for current cursor location
    int             cursor_sticky;      // flag for sticky hit cursor
	char			print_string[2][FILELEN];// print command(0)/filename(1) strings
	char			label_format[FORMAT_LEN];// format of event label
	long			event_id;			// global trigger ID of currently displayed event
	double			event_time;			// time of event
	char		  *	argv;				// command line to run program
	
	double			display_time;		// time to display next event

	int				trigger_flag;		// trigger flag (continuous/single/off)
	long			run_number;			// run number for event
    int      		last_cur_x;			// last cursor x location
    int				last_cur_y;			// last cursor y location

    int             wave_min[kMaxWaveformChannels]; // waveform Y scale minimum
    int             wave_max[kMaxWaveformChannels]; // waveform Y scale maximum
};

// ImageData routines
void	freePoly(Polyhedron *poly);
void	freeWireFrame(WireFrame *frame);
void	initNodes(WireFrame *fm, Point3 *pt, int num);
void	initEdges(WireFrame *fm, int *n1, int *n2, int num);
char *	loadGeometry(Polyhedron *poly, int geo, char *argv);
void	transform(Node *node, Projection *pp, int num);
void	transformPoly(Polyhedron *poly, Projection *pp);
struct tm *getTms(double aTime, int time_zone);
int isIntegerDataType(ImageData *data);

void    initData(ImageData *data, int load_settings);
void    freeData(ImageData *data);
void    deleteData(ImageData *data);
void	sendMessage(ImageData *data, int message, void *dataPt=NULL);
void    aged_next(ImageData *data, int dir);
void    setTriggerFlag(ImageData *data, int theFlag, int end_of_data=0);
void	setLabel(ImageData *data, int on);
float	getHitValPad(ImageData *data, HitInfo *hi);
void    calcHitVals(ImageData *data);
void    clearEvent(ImageData *data);

#endif // __ImageData_h__
