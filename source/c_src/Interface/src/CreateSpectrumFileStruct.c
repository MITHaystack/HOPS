#include <stdlib.h>

#include "HSpectrumFile.h"

struct HSpectrumFileStruct* CreateSpectrumFileStruct()
{
    //malloc the space for the file struct
    struct HSpectrumFileStruct* spec_file = NULL;
    spec_file = (struct HSpectrumFileStruct*) malloc( sizeof( struct HSpectrumFileStruct) );
    if(spec_file != NULL)
    {
        InitializeSpectrumFileStruct(spec_file);
    }
    return spec_file;
}
