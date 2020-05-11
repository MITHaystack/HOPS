#include <stdlib.h>

#include "HNoisePowerFile.h"

struct HNoisePowerFileStruct* CreateNoisePowerFileStruct()
{
    //malloc the space for the file struct
    struct HNoisePowerFileStruct* power_file = NULL;
    power_file = (struct HNoisePowerFileStruct*) malloc( sizeof( struct HNoisePowerFileStruct ) );
    if(power_file != NULL)
    {
        InitializeNoisePowerFileStruct(power_file);
    }
    return power_file;
}
