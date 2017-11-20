//-----------------------------------------------------------------------------
// File:        Aged.h
//
// Description: ALPHA-g Event Display main class
//
// Created:     2017-08-01 - Phil Harvey
//-----------------------------------------------------------------------------

class TStoreEvent;
class PWindow;
struct ImageData;

class Aged
{
public:
    Aged();
    ~Aged();
    
    void ShowEvent(TStoreEvent* anEvent, int runNum);

private:
    ImageData   *fdata;
    PWindow     *fwindow;
};
