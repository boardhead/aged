#ifndef __PHitInfoWindow_h__
#define __PHitInfoWindow_h__

#include <Xm/Xm.h>
#include "PWindow.h"
#include "PListener.h"
#include "PLabel.h"


class PHitInfoWindow : public PWindow, public PListener {
public:
	PHitInfoWindow(ImageData *data);
	~PHitInfoWindow();
	
	virtual void	UpdateSelf();
	virtual void	Listen(int message, void *message_data);
	
private:
	void			ClearEntries();
	void			SetHitXYZ();
	void			ManageXYZ(int manage);
	void			ResizeToFit();
	
	PLabel			hi_time, hi_height, hi_hit_label;
	PLabel			hi_wire, hi_pad, hi_type;
	PLabel			hi_xyz_labels[3], hi_xyz[3];
	int				mLastNum;
};


#endif // __PHitInfoWindow_h__
