#ifndef HBasicDefines_H__
#define HBasicDefines_H__

#define STR1(x) #x
#define STR2(x) STR1(x)

#define HFLAG_WIDTH 8
#define HNAME_WIDTH 256
#define HVERSION_WIDTH 3

#define HSUCCESS 0
#define HMALLOC_ERROR 1
#define HFILE_OPEN_ERROR 2
#define HFILE_VERSION_ERROR 3

#define NOISE_DIODE_SWITCHING_FREQ 80.0
#define NOISE_DIODE_BLANK_PERIOD 1e-3

#endif /* end of include guard: HBasicDefines_H__ */
