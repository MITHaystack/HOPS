#include "hops_config.h"
#include "mk4_util.h"

#ifndef BYTFLP_H
#define BYTFLP_H

#ifdef BYTFLP

#define cp_short(a,b)  {a = b; short_reverse (&(a));}
#define cp_unsig(a,b)  {a = b; unsig_reverse (&(a));}
#define cp_int(a,b)    {a = b; int_reverse (&(a));}
#define cp_uint(a,b)    {a = b; uint_reverse (&(a));}
#define cp_long(a,b)   {a = b; long_reverse (&(a));}
#define cp_float(a,b)  {a = b; float_reverse (&(a));}
#define cp_double(a,b) {a = b; double_reverse (&(a));}

#else

#define cp_short(a,b)  a = b
#define cp_unsig(a,b)  a = b
#define cp_int(a,b)    a = b
#define cp_uint(a,b)    a = b
#define cp_long(a,b)   a = b
#define cp_float(a,b)  a = b
#define cp_double(a,b) a = b

#endif


// strncpy/memcpy guard/helper functions
// these are needed to avoid undefined behavior in the copy_XXX functions when
// source/dest buffers are the same, on AVX-512 machines glibc zeros part of the
// buffer which clobbers any in-place string copies
#include <string.h>

#define cp_strn(a,b,n) do { if ((void *)(a) != (void *)(b)) strncpy (a, b, n); } while (0)
#define cp_str(a,b)    do { if ((void *)(a) != (void *)(b)) strcpy (a, b); } while (0)
#define cp_mem(a,b,n)  memmove (a, b, n)

#endif
