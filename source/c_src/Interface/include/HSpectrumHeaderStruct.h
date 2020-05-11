#ifndef HSpectrumHeaderStruct_H__
#define HSpectrumHeaderStruct_H__

#include <stdint.h>
#include <stdlib.h>

#include "HBasicDefines.h"

/*
*File: HSpectrumHeaderStruct.hh
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/
// 
// #define SPECTRUM_HEADER_VERSION_NUMBER 1
// #define SPECTRUM_HEADER_VERSION STR2(SPECTRUM_HEADER_VERSION_NUMBER)
// 

struct HSpectrumHeaderStruct
{
    uint64_t fHeaderSize; //size of the header struct in bytes
    char fVersionFlag[HFLAG_WIDTH]; //version, format code (also indicates what type (e.g. double, float) the spectrum data is stored in)
    char fSidebandFlag[HFLAG_WIDTH]; //flag indicating the sideband
    char fPolarizationFlag[HFLAG_WIDTH]; //flag indicating the polarization recorded
    uint64_t fStartTime; //acquisition start time in seconds since epoch
    uint64_t fSampleRate; //may need to accomodate doubles?
    uint64_t fLeadingSampleIndex; //sample index since start of the acquisition
    uint64_t fSampleLength; //total number of samples used to compute the spectrum
    uint64_t fNAverages; //if spectral averaging was used, how many averaging periods were used
    uint64_t fSpectrumLength; //number of spectral points
    uint64_t fSpectrumDataTypeSize; //spectrum data point size (e.g. size of data type storing the spectrum; sizeof(float), etc)
    char fExperimentName[HNAME_WIDTH]; //experiment name
    char fSourceName[HNAME_WIDTH]; //source name
    char fScanName[HNAME_WIDTH]; //scan name
};


#endif /* end of include guard: HSpectrumHeaderStruct */
