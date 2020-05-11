#include "HSpectrumFile.h"

#include <string.h>

void CopySpectrumFileStruct(const struct HSpectrumFileStruct* src, struct HSpectrumFileStruct* dest)
{
    ClearSpectrumFileStruct(dest);
    InitializeSpectrumFileStruct(dest);

    unsigned int i;
    dest->fHeader.fHeaderSize = src->fHeader.fHeaderSize;
    for(i=0; i<HFLAG_WIDTH; i++)
    {
        dest->fHeader.fVersionFlag[i] = src->fHeader.fVersionFlag[i];
        dest->fHeader.fSidebandFlag[i] = src->fHeader.fSidebandFlag[i];
        dest->fHeader.fPolarizationFlag[i] = src->fHeader.fPolarizationFlag[i];
    }
    dest->fHeader.fStartTime = src->fHeader.fStartTime; //acquisition start time in seconds since epoch
    dest->fHeader.fSampleRate = src->fHeader.fSampleRate; //may need to accomodate doubles?
    dest->fHeader.fLeadingSampleIndex = src->fHeader.fLeadingSampleIndex; //sample index since start of the acquisition
    dest->fHeader.fSampleLength = src->fHeader.fSampleLength ; //total number of samples used to compute the spectrum
    dest->fHeader.fNAverages = src->fHeader.fNAverages ; //if spectral averaging was used, how many averaging periods were used
    dest->fHeader.fSpectrumLength = src->fHeader.fSpectrumLength; //number of spectral points
    dest->fHeader.fSpectrumDataTypeSize = src->fHeader.fSpectrumDataTypeSize; 

    for(i=0; i<HNAME_WIDTH; i++)
    {
        dest->fHeader.fExperimentName[i] = src->fHeader.fExperimentName[i];
        dest->fHeader.fSourceName[i] = src->fHeader.fSourceName[i];
        dest->fHeader.fScanName[i] = src->fHeader.fScanName[i];
    }

    if(src->fRawSpectrumData != NULL)
    {
        //malloc space for the raw spectrum data
        dest->fRawSpectrumData = malloc( (dest->fHeader.fSpectrumLength)*(dest->fHeader.fSpectrumDataTypeSize) );
        if(dest->fRawSpectrumData != NULL)
        {
            //copy the raw spectrum data
            memcpy( dest->fRawSpectrumData, src->fRawSpectrumData, (dest->fHeader.fSpectrumLength)*(dest->fHeader.fSpectrumDataTypeSize) );
        }
    }
};
