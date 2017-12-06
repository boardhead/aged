#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "ImageData.h"
#include "matrix.h"
#include "menu.h"
#include "openfile.h"
#include "PProjImage.h"
#include "PEventHistogram.h"
#include "PEventControlWindow.h"
#include "PResourceManager.h"

#define	BUFFLEN				512
#define	PLOT_MAX			8000.0			/* maximum value for x,y plot coords */

char *sFilePath = NULL;

/*------------------------------------------------------------------------------------
*/
void initData(ImageData *data, int load_settings)
{
    int         i;
	
	/* create speaker object */
	data->mSpeaker = new PSpeaker;
/*
** Initialize ImageData from resources
*/
	// initialize ImageData from resources
	memcpy(data, &PResourceManager::sResource, sizeof(AgedResource));

	// range check menu items
	if ((unsigned)data->dataType > IMAX_DATATYPE)		  data->dataType = 0;
	if ((unsigned)data->projType > IMAX_PROJTYPE)	   	  data->projType = 0;
	if ((unsigned)data->shapeOption > IMAX_SHAPEOPTION)   data->shapeOption = 0;
	
	// range check other necessary resources
	if ((unsigned)data->print_to >= 2)	data->print_to = 0;

	// initialize necessary ImageData elements
	/* -- deemed "confusing", so reset the following settings to zero */
	data->bit_mask			= HIT_HIDDEN;

	// reset other settings if not loading settings
	if (!load_settings) {
		data->dataType 		= 0;
		data->projType 		= 0;
		data->shapeOption 	= 0;
		data->show_detector	= 0;
		data->show_fit   	= 0;
		data->open_windows 	= 0;
		data->open_windows2 = 0;
		data->hex_id 		= 0;
#ifndef ANTI_ALIAS
		data->time_zone 	= 0;
#endif
		data->angle_rad 	= 0;
		data->hit_xyz 		= 0;
		data->log_scale		= 0;
		data->hit_size		= 1;
		data->save_config	= 0;
	}
	
	data->wDataType			= IDM_TIME;			// set later
	data->wSpStyle			= IDM_SP_ERRORS;	// set later
	data->wProjType			= data->projType + IDM_PROJ_RECTANGULAR;
	data->wShapeOption		= data->shapeOption + IDM_HIT_SQUARE;
	data->dispName	 		= XtMalloc(20);
	strcpy(data->dispName, "Hit Time");
	data->projName			= XtMalloc(20);
	strcpy(data->projName, "Rectangular");
	data->cursor_hit 		= -1;
	data->cursor_sticky 	= 0;
	data->angle_conv 		= 180 / PI;
	data->sun_dir.x3        = 1 / sqrt(2);
	data->sun_dir.y3        = data->sun_dir.x3;
	data->sun_dir.z3        = 0;
	
	sFilePath = data->file_path;
	
	for (i=0; i<2; ++i) {
		strncpy(data->print_string[i], data->print_string_pt[i], FILELEN);
		data->print_string[i][FILELEN-1] = '\0';
	}
	strncpy(data->label_format, data->label_format_pt, FORMAT_LEN);
	data->label_format[FORMAT_LEN-1] = '\0';
}

/* free all memory allocated in ImageData structure */
/* (except the main window object and the ImageData itself) */
void freeData(ImageData *data)
{
	int		i;
	// delete all our windows
	for (i=0; i<NUM_WINDOWS; ++i) {
		if (data->mWindow[i]) {
			delete data->mWindow[i];
		}
	}
	
	// clear the displayed event
	clearEvent(data);
	
	XtFree(data->projName);
	data->projName = NULL;
	XtFree(data->dispName);
	data->dispName = NULL;
	
	delete data->mSpeaker;
	data->mSpeaker = NULL;
}

/* close all windows and delete image data */
void deleteData(ImageData *data)
{
	if (data) {
		if (data->mMainWindow) {
			// Close the Aged display
			// - deletes all related window objects
			delete data->mMainWindow;
		} else {
			freeData(data);
			delete data;
		}
	}
}

// Send message to windows and update
void sendMessage(ImageData *data, int message, void *dataPt)
{
	data->mSpeaker->Speak(message, dataPt);
}


void clearEvent(ImageData *data)
{
	if (data->hits.num_nodes) {
		data->hits.num_nodes = 0;
		free(data->hits.nodes);
		data->hits.nodes = NULL;
		free(data->hits.hit_info);
		data->hits.hit_info = NULL;
	}
	data->cursor_hit = -1;
	data->run_number = 0;
	data->event_id = 0;
    data->cursor_sticky = 0;
	//setEventTime(data,0);
	data->display_time = 0;
	data->agEvent = 0;

	if (data->mSpeaker) {
		sendMessage(data, kMessageEventCleared);
	}
}

void setLabel(ImageData *data, int on)
{
	if (data->show_label != on) {
		data->show_label = on;
		if (on) {
			data->mMainWindow->LabelFormatChanged();
		} else {
			sendMessage(data, kMessageLabelChanged);
		}
		// also send a show-label-changed message
		sendMessage(data, kMessageShowLabelChanged);
	}
}

void setTriggerFlag(ImageData *data, int theFlag, int end_of_data)
{
	data->trigger_flag = theFlag;

	sendMessage(data, kMessageTriggerChanged);
	
	if (!end_of_data) {
        if (theFlag != TRIGGER_OFF) data->mNext = 1;
		/* set/reset throw out flag as necessary */
		//TEST ActivateTrigger(data);
	
		/* start handling events immediately */
		//TEST HandleEvents(data);
	}
}

// get hit value for currently displayed parameter
float getHitVal(ImageData *data, HitInfo *hi)
{
	float val;
	
	switch (data->wDataType) {
		case IDM_TIME:
            val = hi->time;
			break;
		case IDM_HEIGHT:
            val = hi->height;
			break;
		case IDM_ERROR:
            val = sqrt(hi->error[0]*hi->error[0] + hi->error[1]*hi->error[1] + hi->error[2]*hi->error[2]);
            break;
		case IDM_DISP_WIRE:
			val = hi->wire;
			break;
		case IDM_DISP_PAD:
			val = hi->pad;
			break;
	}
	return(val);
}

// getHitValPad - get hit value, padding with +0.5 for integer data types
float getHitValPad(ImageData *data, HitInfo *hi)
{
	float	val = getHitVal(data, hi);
	
	if (isIntegerDataType(data)) {
		val += 0.5;
	}
	return(val);
}

// calculate the colours corresponding to hit values for display
void calcHitVals(ImageData *data)
{
	int		i;
	float	val, first, last, range;
	long	ncols;
	HitInfo	*hi;
	int		n;

	hi = data->hits.hit_info;
	n  = data->hits.num_nodes;
	
	PEventHistogram::GetBins(data, &first, &last);
	range = last - first;
/*
** Calculate colour indices for each hit
*/
	ncols = data->num_cols - 2;
	for (i=0; i<n; ++i,++hi) {
		if (hi->flags & HIT_DISCARDED) {
			hi->hit_val = (int)ncols + 2;
			continue;
		}
		// calculate scaled hit value
		val = ncols * (getHitValPad(data, hi) - first) / range;

		// reset over/underscale flags
		hi->flags &= ~(HIT_OVERSCALE|HIT_UNDERSCALE);
		if (val < 0) {
			val = -1;
			hi->flags |= HIT_UNDERSCALE;	// set underscale flag
		} else if (val >= ncols) {
			val = ncols;
			hi->flags |= HIT_OVERSCALE;		// set overscale flag
		}
		hi->hit_val = (int)val + 1;
	}
}

void initNodes(WireFrame *fm, Point3 *pt, int num)
{
	int		i;
	Node	*node;

	fm->nodes = (Node *)malloc(num * sizeof(Node));

	if (fm->nodes) {

		for (i=0,node=fm->nodes; i<num; ++i,++node) {
			node->x3 = pt[i].x;
			node->y3 = pt[i].y;
			node->z3 = pt[i].z;
		}
		fm->num_nodes = num;
	}
}
void initEdges(WireFrame *fm, int *n1, int *n2, int num)
{
	int			i;
	Edge		*edge;

	fm->edges = (Edge *)malloc(num * sizeof(Edge));

	if (fm->edges) {

		for (i=0,edge=fm->edges; i<num; ++i,++edge) {
			edge->n1  = fm->nodes + n1[i];
			edge->n2  = fm->nodes + n2[i];
			edge->flags = 0;
		}
		fm->num_edges = num;
	}
}
void freePoly(Polyhedron *poly)
{
	if (poly->num_nodes) {
		poly->num_nodes = 0;
		free(poly->nodes);
		poly->nodes = NULL;
	}
	if (poly->num_edges) {
		poly->num_edges = 0;
		free(poly->edges);
		poly->edges = NULL;
	}
	if (poly->num_faces) {
		poly->num_faces = 0;
		free(poly->faces);
		poly->faces = NULL;
	}
}
void freeWireFrame(WireFrame *frame)
{
	if (frame->num_nodes) {
		frame->num_nodes = 0;
		free(frame->nodes);
		frame->nodes = NULL;
	}
	if (frame->num_edges) {
		frame->num_edges = 0;
		free(frame->edges);
		frame->edges = NULL;
	}
}
char *loadGeometry(Polyhedron *poly, int geo, char *argv)
{
	Edge		*edge;
	Node		*node, *t1, *t2;
	Face		*face;
	int			nn,ne,nf;
	int			i,j,k,ct,found,n,n1,n2;
	char		*pt,*geo_name,buff[BUFFLEN];
	float		x,y,z,x2,y2,z2,len;
	float		r;
	FILE		*fp;
	char		*msg;
	static char	rtn_msg[BUFFLEN];

	msg = 0;
	switch (geo) {
		case IDM_DETECTOR:
			geo_name = "detector.geo";
			break;
		default:
			return("Unknown geometry");
	}
	fp = openFile(geo_name,"r",sFilePath);
	if (!fp) {
		sprintf(rtn_msg,"Can't open Geometry file %s",geo_name);
		return(rtn_msg);
	}
	
	sprintf(rtn_msg, "Bad format .geo file: %s",geo_name);

	for (i=0; i<4; ++i) {

		fgets(buff,BUFFLEN,fp);
		pt = strchr(buff,'=');
		if (!pt) {
			fclose(fp);
			return(rtn_msg);
		}
		++pt;
		switch (buff[0]) {
			case 'R':
				sscanf(pt,"%f",&r);
				break;
			case 'N':
				sscanf(pt,"%d",&nn);
				break;
			case 'F':
				sscanf(pt,"%d",&nf);
				break;
			case 'E':
				sscanf(pt,"%d",&ne);
				break;
			default:
				fclose(fp);
				return(rtn_msg);
		}
	}
	r /= AG_SCALE;
	poly->radius = r;
/*
** Reallocate memory for new arrays
*/
	freePoly(poly);

	poly->nodes = (Node *)malloc(nn * sizeof(Node));
	poly->edges = (Edge *)malloc(ne * sizeof(Edge));
	poly->faces = (Face *)malloc(nf * sizeof(Face));

	if (!poly->nodes || !poly->edges || !poly->faces) {
		fclose(fp);
		return(rtn_msg);
	}

	poly->num_nodes = nn;
	poly->num_edges = ne;
	poly->num_faces = nf;
/*
** Get nodes, edges and faces arrays from file
*/
	for (i=0,node=poly->nodes; i<nn; ++i,++node) {

		fgets(buff,BUFFLEN,fp);
		if (buff[0] != 'N') {
			fclose(fp);
			return(rtn_msg);
		}
		j=sscanf(buff+2,"%f, %f, %f",&x,&y,&z);

		if (j!=3) {
			fclose(fp);
			return(rtn_msg);
		}

		node->x3 = x * r;
		node->y3 = y * r;
		node->z3 = z * r;
	}
	for (i=0,face=poly->faces; i<nf; ++i,++face) {
		fgets(buff,BUFFLEN,fp);
		if (buff[0] != 'F') {
			fclose(fp);
			return(rtn_msg);
		}
		pt = buff + 2;
		n = atoi(pt);
		if (n<3 || n>MAX_FNODES) {
			fclose(fp);
			return(rtn_msg);
		}
		face->num_nodes = n;
		for (j=0; j<n; ++j) {
			pt = strchr(pt,',');
			if (!pt) {
				fclose(fp);
				return(rtn_msg);
			}
			face->nodes[j] = poly->nodes + (atoi(++pt)-1);
		}
		if (face-poly->faces > nf) {
			fclose(fp);
			return(rtn_msg);
		}
/*
** calculate vector normal to this face
*/
		t1 = face->nodes[2];
		t2 = face->nodes[1];
		x  = t1->x3 - t2->x3;
		y  = t1->y3 - t2->y3;
		z  = t1->z3 - t2->z3;
		t1 = face->nodes[0];
		x2 = t1->x3 - t2->x3;
		y2 = t1->y3 - t2->y3;
		z2 = t1->z3 - t2->z3;
		face->norm.x = y*z2 - z*y2;
		face->norm.y = z*x2 - x*z2;
		face->norm.z = x*y2 - y*x2;
		len = vectorLen(face->norm.x, face->norm.y, face->norm.z);
		face->norm.x /= len;
		face->norm.y /= len;
		face->norm.z /= len;
	}
	for (i=0,edge=poly->edges; i<ne; ++i,++edge) {
	
		fgets(buff,BUFFLEN,fp);
		if (buff[0] != 'E') {
			fclose(fp);
			return(rtn_msg);
		}
		j=sscanf(buff+2,"%d, %d",&n1,&n2);
		if (j!=2) {
			fclose(fp);
			return(rtn_msg);
		}
		edge->n1 = poly->nodes + n1 - 1;
		edge->n2 = poly->nodes + n2 - 1;
		edge->flags = 0;
/*
** Figure out which faces belong to this edge
*/
		found = 0;
		for (j=0, face=poly->faces; j<nf; ++j,++face) {
			n = face->num_nodes;
			ct = 0;
			for (k=0; k<n; ++k) {
				if (face->nodes[k]==edge->n1 ||
					face->nodes[k]==edge->n2) ++ct;
			}
			if (ct==2) {
				if (found++) {
					edge->f2 = face;
					break;
				} else {
					edge->f1 = face;
				}
			}
		}
		if (found != 2 && found!=1) {
			sprintf(rtn_msg,"Error matching edge %d and faces in %s (found %d)",i+1,geo_name,found);
			msg = rtn_msg;
		}
	}
	fclose(fp);
	return(msg);
}

/*
 * tranform the coordinates of a node by the specified projection
 * Inputs: node->x3,y3,z3
 * Outputs: node->x,y,xr,yr,zr,flags
 * resets NODE_HID and sets NODE_OUT appropriately
 */
void transform(Node *node, Projection *pp, int num)
{
	int		i;
	float	f;
	float	x,y,z;
	float	xt,yt,zt;
	float	axt,ayt;
	float	xsc   = pp->xscl;
	float	ysc   = pp->yscl;
	int		xcn   = pp->xcen;
	int		ycn   = pp->ycen;
	float	(*rot)[3] = pp->rot;
	float  *vec  = pp->pt;
	int		pers  = vec[2] < pp->proj_max;

	for (i=0; i<num; ++i,++node) {
		x = node->x3;
		y = node->y3;
		z = node->z3;
		xt = (node->xr = x*rot[0][0] + y*rot[0][1] + z*rot[0][2]) - vec[0];
		yt = (node->yr = x*rot[1][0] + y*rot[1][1] + z*rot[1][2]) - vec[1];
		zt = (node->zr = x*rot[2][0] + y*rot[2][1] + z*rot[2][2]) - vec[2];

		// reset NODE_OUT and NODE_HID flags
		node->flags &= ~(NODE_OUT | NODE_HID);
		
		if (pers) {
			if (zt >= 0) {
				node->flags |= NODE_OUT;
				axt = fabs(xt);
				ayt = fabs(yt);
				if (axt > ayt) f = PLOT_MAX/axt;
				else  if (ayt) f = PLOT_MAX/ayt;
				else {
					f = PLOT_MAX;
					xt = yt = 1;
				}
				node->x =   (int)(f * xt);
				node->y = - (int)(f * yt);
			} else {
/*
** Distort image according to projection point while maintaining
** a constant magnification for the projection screen.
*/
				x = xcn + xt * (pp->proj_screen-vec[2]) * xsc / zt;
				y = ycn - yt * (pp->proj_screen-vec[2]) * ysc / zt;
				axt = fabs(x);
				ayt = fabs(y);
				if (axt>PLOT_MAX || ayt>PLOT_MAX) {
					node->flags |= NODE_OUT;
					if (axt > ayt) f = PLOT_MAX/axt;
					else  if (ayt) f = PLOT_MAX/ayt;
					else {
						f = PLOT_MAX;
						x = y = 1;
					}
					node->x = (int)(f * x);
					node->y = (int)(f * y);
				} else {
					node->x = (int)(x);
					node->y = (int)(y);
				}
			}
		} else {
			node->x = (int)(xcn + xsc * xt);
			node->y = (int)(ycn - ysc * yt);
		}
	}
}

void transformPoly(Polyhedron *poly, Projection *pp)
{
	int		i;
	Vector3	rp, t;
	float	dot;
	Node	*node;
	Face	*face = poly->faces;
	int		num   = poly->num_faces;
	float	*pt   = pp->pt;

	transform(poly->nodes,pp,poly->num_nodes);
/*
** Calculate dot product of normals to face with vector to proj point.
** If product is negative, face is hidden.
** First, rotate projection point into detector frame.
*/
	if (pt[2] < pp->proj_max) {

		vectorMult(pp->inv, pt, rp);

		node = poly->nodes;

		for (i=0; i<num; ++i,++face) {
			node = face->nodes[0];			/* get first node of face */
			dot = (rp[0] - node->x3) * face->norm.x +
				  (rp[1] - node->y3) * face->norm.y +
				  (rp[2] - node->z3) * face->norm.z;
			if (dot < 0) face->flags |= FACE_HID;
			else		 face->flags &= ~FACE_HID;
		}

	} else {

		t[0] = 0;							/* no  perspective */
		t[1] = 0;
		t[2] = 1;
		vectorMult(pp->inv, t, rp);

		for (i=0; i<num; ++i,++face) {
			dot = rp[0] * face->norm.x +
				  rp[1] * face->norm.y +
				  rp[2] * face->norm.z;

			if (dot < 0) face->flags |= FACE_HID;
			else		 face->flags &= ~FACE_HID;
		}
	}
}

int isIntegerDataType(ImageData *data)
{
	switch (data->wDataType) {
		case IDM_TIME:
		case IDM_HEIGHT:
		case IDM_ERROR:
			break;
		case IDM_DISP_WIRE:
		case IDM_DISP_PAD:
			return(1);
	}
	return(0);
}

static int is_sudbury_dst(struct tm *tms)
{
	int		mday;
	int 	is_dst = 0;		// initialize is_dst flag

    // check to see if we should have been in daylight savings time
    if (tms->tm_year < 107) {    // before 2007
        if (tms->tm_mon>3 && tms->tm_mon<9) {   // after April and before October
            is_dst = 1;             // we are in daylight savings time
        } else if (tms->tm_mon == 3) {          // the month is April
            // calculate the date of the first Sunday in the month
            mday = ((tms->tm_mday - tms->tm_wday + 6) % 7) + 1;
            if (tms->tm_mday > mday ||          // after the first Sunday or
                (tms->tm_mday == mday &&        // (on the first Sunday and
                tms->tm_hour >= 2))             //  after 2am)
            {
                is_dst = 1;         // we are in daylight savings time
            }
        } else if (tms->tm_mon == 9) {          // the month is October
            // calculate the date of the last Sunday in the month
            // (remember, October has 31 days)
            mday = ((tms->tm_mday - tms->tm_wday + 10) % 7) + 25;
            if (tms->tm_mday < mday ||          // before the last Sunday or
                (tms->tm_mday == mday &&        // (on the last Sunday and
                tms->tm_hour < 1))              //  before 1am (2am DST))
            {
                is_dst = 1;         // we are in daylight savings time
            }
        }
    } else {                    // 2007 and later
        if (tms->tm_mon>2 && tms->tm_mon<10) {  // after March and before November
            is_dst = 1;             // we are in daylight savings time
        } else if (tms->tm_mon == 2) {          // the month is March
            // calculate the date of the 2nd Sunday in the month
            mday = ((tms->tm_mday - tms->tm_wday + 6) % 7) + 8;
            if (tms->tm_mday > mday ||          // after the 2nd Sunday or
                (tms->tm_mday == mday &&        // (on the 2nd Sunday and
                tms->tm_hour >= 2))             //  after 2am)
            {
                is_dst = 1;         // we are in daylight savings time
            }
        } else if (tms->tm_mon == 10) {         // the month is November
            // calculate the date of the first Sunday in the month
            mday = ((tms->tm_mday - tms->tm_wday + 6) % 7) + 1;
            if (tms->tm_mday < mday ||          // before the first Sunday or
                (tms->tm_mday == mday &&        // (on the first Sunday and
                tms->tm_hour < 1))              //  before 1am (2am DST))
            {
                is_dst = 1;         // we are in daylight savings time
            }
        }
    }
	return(is_dst);
}

// return a time structure in the specified time zone (0=sudbury, 1=local, 2=UTC)
// - input time in UTC seconds since unix time zero
struct tm *getTms(double aTime, int time_zone)
{
	int			is_dst;
	struct tm	*tms;
	time_t 		the_time = (time_t)aTime;
	
	switch (time_zone) {
		default: //case kTimeZoneEST:
			// adjust to EST (+5:00) -- initially without daylight savings time
			the_time -= 5 * 3600L;
			tms = gmtime(&the_time);
			is_dst = is_sudbury_dst(tms);
			if (is_dst) {
				the_time += 3600L;		// spring forward into DST
				tms = gmtime(&the_time);
				tms->tm_isdst = 1;		// set isdst flag
			}
			break;
		case kTimeZoneLocal:
			tms = localtime(&the_time);
			break;
		case kTimeZoneUTC:
			tms = gmtime(&the_time);
			break;
	}
	return(tms);
}

// return time_t from given tms and time zone
time_t getTime(struct tm *tms, int time_zone)
{
	time_t theTime;
	
	tms->tm_isdst = 0;	// reset DST flag initially
	theTime = mktime(tms);
	
	switch (time_zone) {
		default: //case kTimeZoneEST:
			theTime	-= timezone;	// convert to GMT
			// adjust to EST (intially with no DST)
			theTime += 5 * 3600L;
			// SHOULD FIX THIS FOR OTHER TIME ZONES!
			if (is_sudbury_dst(tms)) {
				theTime -= 3600L;	// sprint forward into DST
			}
			break;
		case kTimeZoneLocal:
			// nothing to do, mktime assumes local time
			break;
		case kTimeZoneUTC:
			theTime	-= timezone;	// convert to GMT
			break;
	}
	return(theTime);
}
