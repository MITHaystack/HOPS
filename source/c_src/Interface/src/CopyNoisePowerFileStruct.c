#include "HNoisePowerFile.h"

void CopyNoisePowerFileStruct(const struct HNoisePowerFileStruct* src, struct HNoisePowerFileStruct* dest)
{
    ClearNoisePowerFileStruct(dest);
    InitializeNoisePowerFileStruct(dest);

    unsigned int i;
    dest->fHeader.fHeaderSize = src->fHeader.fHeaderSize;
    for(i=0; i<HFLAG_WIDTH; i++)
    {
        dest->fHeader.fVersionFlag[i] = src->fHeader.fVersionFlag[i];
        dest->fHeader.fSidebandFlag[i] = src->fHeader.fSidebandFlag[i];
        dest->fHeader.fPolarizationFlag[i] = src->fHeader.fPolarizationFlag[i];
    }
    dest->fHeader.fStartTime = src->fHeader.fStartTime; 
    dest->fHeader.fSampleRate = src->fHeader.fSampleRate;
    dest->fHeader.fLeadingSampleIndex = src->fHeader.fLeadingSampleIndex;
    dest->fHeader.fSampleLength = src->fHeader.fSampleLength; 
    dest->fHeader.fAccumulationLength = src->fHeader.fAccumulationLength;
    dest->fHeader.fSwitchingFrequency = src->fHeader.fSwitchingFrequency;
    dest->fHeader.fBlankingPeriod = src->fHeader.fBlankingPeriod;

    for(i=0; i<HNAME_WIDTH; i++)
    {
        dest->fHeader.fExperimentName[i] = src->fHeader.fExperimentName[i];
        dest->fHeader.fSourceName[i] = src->fHeader.fSourceName[i] ;
        dest->fHeader.fScanName[i] = src->fHeader.fScanName[i];
    }

    if(src->fAccumulations != NULL)
    {
        //malloc space for the accumulation data
        dest->fAccumulations = malloc( (dest->fHeader.fAccumulationLength)*sizeof(struct HDataAccumulationStruct) );
        if(dest->fAccumulations != NULL)
        {
            //copy accumulation data
            for(i = 0; i <dest->fHeader.fAccumulationLength; i++)
            {
                struct HDataAccumulationStruct* stat_src = &(src->fAccumulations[i]);
                struct HDataAccumulationStruct* stat_dest = &(dest->fAccumulations[i]);
                stat_dest->sum_x = stat_src->sum_x;
                stat_dest->sum_x2 = stat_src->sum_x2;
                stat_dest->count = stat_src->count;
                stat_dest->state_flag = stat_src->state_flag;
                stat_dest->start_index = stat_src->start_index;
                stat_dest->stop_index = stat_src->stop_index;
            }
        }
    }


}
