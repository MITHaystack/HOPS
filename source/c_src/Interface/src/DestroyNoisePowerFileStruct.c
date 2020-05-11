#include "HNoisePowerFile.h"

void DestroyNoisePowerFileStruct(struct HNoisePowerFileStruct* power)
{
    ClearNoisePowerFileStruct(power);
    free(power);
}
