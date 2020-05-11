#ifndef HSpectrumFileStruct_H__
#define HSpectrumFileStruct_H__

#include <stdint.h>
#include <stdlib.h>

#include "HSpectrumHeaderStruct.h"

/*
*File: HSpectrumFileStruct.hh
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

struct HSpectrumFileStruct
{
    //header data
    struct HSpectrumHeaderStruct fHeader;
    //raw spectrum data as raw char blob
    char* fRawSpectrumData;
};


#endif /* end of include guard: HSpectrumFileStruct */
