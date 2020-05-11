#include "HSpectrumFile.h"

void ClearSpectrumFileStruct(struct HSpectrumFileStruct* spectrum)
{
    //free any raw spec data
    if(spectrum->fRawSpectrumData != NULL)
    {
        free(spectrum->fRawSpectrumData);
    }
    spectrum->fRawSpectrumData = NULL;

    //clear the struct
    InitializeSpectrumFileStruct(spectrum);
}
