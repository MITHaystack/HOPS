#include "HSpectrumFile.h"

void DestroySpectrumFileStruct(struct HSpectrumFileStruct* spectrum)
{
    ClearSpectrumFileStruct(spectrum);
    free(spectrum);
}
