#ifndef HNoisePowerFile_H__
#define HNoisePowerFile_H__

#include "HDataAccumulationStruct.h"
#include "HNoisePowerHeaderStruct.h"
#include "HNoisePowerFileStruct.h"

static char const * const NOISE_POWER_HEADER_VERSION = "001";

extern struct HNoisePowerFileStruct* CreateNoisePowerFileStruct();
extern void InitializeNoisePowerFileStruct(struct HNoisePowerFileStruct* power);
extern void ClearNoisePowerFileStruct(struct HNoisePowerFileStruct* power);
extern void DestroyNoisePowerFileStruct(struct HNoisePowerFileStruct* power);
extern void CopyNoisePowerFileStruct(const struct HNoisePowerFileStruct* src, struct HNoisePowerFileStruct* dest);
extern int ReadNoisePowerFile(const char* filename, struct HNoisePowerFileStruct* power);
extern int WriteNoisePowerFile(const char* filename, struct HNoisePowerFileStruct* power);


#endif /* end of include guard: HNoisePowerFile_H__ */
