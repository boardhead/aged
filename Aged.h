//-----------------------------------------------------------------------------
// File:        Aged.h
//
// Description: ALPHA-g Event Display main class
//
// Created:     2017-08-01 - Phil Harvey
//-----------------------------------------------------------------------------

class AgAnalysisFlow;
class AgSignalsFlow;
class TARunInfo;
class PWindow;
struct ImageData;

class Aged
{
public:
    Aged();
    ~Aged();
    
    void ShowEvent(AgAnalysisFlow* anaFlow, AgSignalsFlow* sigFlow, TARunInfo* runinfo);

private:
    ImageData   *fData;
    PWindow     *fWindow;
};
