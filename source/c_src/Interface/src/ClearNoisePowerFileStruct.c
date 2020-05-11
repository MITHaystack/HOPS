#include "HNoisePowerFile.h"

void ClearNoisePowerFileStruct(struct HNoisePowerFileStruct* power)
{
    //free any accumulation data
    if(power->fAccumulations != NULL)
    {
        free(power->fAccumulations);
    }
    power->fAccumulations = NULL;

    //clear the struct
    InitializeNoisePowerFileStruct(power);
}
