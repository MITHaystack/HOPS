#ifndef HSpectrumFile_H__
#define HSpectrumFile_H__

#include "HBasicDefines.h"
#include "HSpectrumHeaderStruct.h"
#include "HSpectrumFileStruct.h"

static char const * const SPECTRUM_HEADER_VERSION = "001";

extern struct HSpectrumFileStruct* CreateSpectrumFileStruct();
extern void InitializeSpectrumFileStruct(struct HSpectrumFileStruct* spectrum);
extern void ClearSpectrumFileStruct(struct HSpectrumFileStruct* spectrum);
extern void DestroySpectrumFileStruct(struct HSpectrumFileStruct* spectrum);
extern void CopySpectrumFileStruct(const struct HSpectrumFileStruct* src, struct HSpectrumFileStruct* dest);
extern int ReadSpectrumFile(const char* filename, struct HSpectrumFileStruct* spectrum);
extern int WriteSpectrumFile(const char* filename, struct HSpectrumFileStruct* spectrum);

#endif /* end of include guard: HSpectrumFile_H__ */
