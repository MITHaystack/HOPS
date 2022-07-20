/*
 * Fix to retain precision in GPS post 2019
 */
#ifndef TIME_FLOAT_TYPE_H__

#define TIME_FLOAT_TYPE_H__

/*
 * We now rely on hops_config.h/autoconf to determine if this
 * system supports 80-bit long doubles.  This is needed for
 * ~microsecond level precision for times which are more than
 * ~40 years after the GPS epoch start.  Tempus fugit.
 */
 
#include "hops_config.h"

#ifdef HAVE_LONG_DOUBLE_WIDER
        #define USE_LONG_DOUBLE
        typedef long double     HTLdbl;
        #define HLDL            "L"
#else
        typedef double          HTLdbl;
        #define HLDL            "l"
        #warning "This system does not support a long double type" \
            " greater than 64 bits. GPS time precision will be impaired."
#endif

#endif
/*
 * eof
 */
