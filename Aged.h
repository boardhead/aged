//-----------------------------------------------------------------------------
// File:        Aged.h
//
// Description: ALPHA-g Event Display main class
//
// Created:     2017-08-01 - Phil Harvey
//-----------------------------------------------------------------------------

class TStoreEvent;
class PWindow;
class AgAnalysisFlow;
class TARunInfo;
struct ImageData;

class Aged
{
public:
    Aged();
    ~Aged();
    
    void ShowEvent(AgAnalysisFlow* analysis_flow, TARunInfo* runinfo);

private:
    ImageData   *fData;
    PWindow     *fWindow;
};
