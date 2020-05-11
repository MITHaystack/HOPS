#include "HNoisePowerFile.h"

void InitializeNoisePowerFileStruct(struct HNoisePowerFileStruct* power)
{
    unsigned int i;
    power->fHeader.fHeaderSize = sizeof(struct HNoisePowerHeaderStruct);
    for(i=0; i<HFLAG_WIDTH; i++)
    {
        power->fHeader.fVersionFlag[i] = '\0';
        power->fHeader.fSidebandFlag[i] = '\0';
        power->fHeader.fPolarizationFlag[i] = '\0';
    }
    power->fHeader.fStartTime = 0; 
    power->fHeader.fSampleRate = 0;
    power->fHeader.fLeadingSampleIndex = 0;
    power->fHeader.fSampleLength = 0; 
    power->fHeader.fAccumulationLength = 0;
    power->fHeader.fSwitchingFrequency = 0.0;
    power->fHeader.fBlankingPeriod = 0.0;

    for(i=0; i<HNAME_WIDTH; i++)
    {
        power->fHeader.fExperimentName[i] = '\0';
        power->fHeader.fSourceName[i] = '\0';
        power->fHeader.fScanName[i] = '\0';
    }

    power->fAccumulations = NULL;

}
