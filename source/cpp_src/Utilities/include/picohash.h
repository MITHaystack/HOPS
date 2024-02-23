/*
 * The code is placed under public domain by Kazuho Oku <kazuhooku@gmail.com>.
 *
 * The MD5 implementation is based on a public domain implementation written by
 * Solar Designer <solar@openwall.com> in 2001, which is used by Dovecot.
 *
 * The SHA1 implementation is based on a public domain implementation written
 * by Wei Dai and other contributors for libcrypt, used also in liboauth.
 *
 * The SHA224/SHA256 implementation is based on a public domain implementation
 * by Sam Hocevar <sam@hocevar.net> for LibTomCrypt.
 */

/*!
* J. Barrett: cloned from https://github.com/kazuho/picohash.git at commit ID:
* 57fe0cb4c32527c71dab5973539673dcba0280c5 on Mar 12, 2021
* Mar 16, 2021 : modified to strip out everything but md5
* and fix char* <-> void* casts
*/

#ifndef _picohash_h_
#define _picohash_h_

#include <assert.h>
#include <inttypes.h>
#include <string.h>

#ifdef __BIG_ENDIAN__
#define _PICOHASH_BIG_ENDIAN
#elif defined __LITTLE_ENDIAN__
/* override */
#elif defined __BYTE_ORDER
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _PICOHASH_BIG_ENDIAN
#endif
#else               // ! defined __LITTLE_ENDIAN__
#include <endian.h> // machine/endian.h
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _PICOHASH_BIG_ENDIAN
#endif
#endif

#define PICOHASH_MD5_BLOCK_LENGTH 64
#define PICOHASH_MD5_DIGEST_LENGTH 16

typedef struct {
    uint_fast32_t lo, hi;
    uint_fast32_t a, b, c, d;
    unsigned char buffer[64];
    uint_fast32_t block[PICOHASH_MD5_DIGEST_LENGTH];
} _picohash_md5_ctx_t;

static void _picohash_md5_init(_picohash_md5_ctx_t *ctx);
static void _picohash_md5_update(_picohash_md5_ctx_t *ctx, const void *data, size_t size);
static void _picohash_md5_final(_picohash_md5_ctx_t *ctx, void *digest);

/* following are private definitions */

/*
 * The basic MD5 functions.
 *
 * F is optimized compared to its RFC 1321 definition just like in Colin
 * Plumb's implementation.
 */
#define _PICOHASH_MD5_F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define _PICOHASH_MD5_G(x, y, z) ((y) ^ ((z) & ((x) ^ (y))))
#define _PICOHASH_MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define _PICOHASH_MD5_I(x, y, z) ((y) ^ ((x) | ~(z)))

/*
 * The MD5 transformation for all four rounds.
 */
#define _PICOHASH_MD5_STEP(f, a, b, c, d, x, t, s)                                                                                 \
    (a) += f((b), (c), (d)) + (x) + (t);                                                                                           \
    (a) = (((a) << (s)) | (((a)&0xffffffff) >> (32 - (s))));                                                                       \
    (a) += (b);

/*
 * SET reads 4 input bytes in little-endian byte order and stores them
 * in a properly aligned word in host byte order.
 *
 * The check for little-endian architectures which tolerate unaligned
 * memory accesses is just an optimization.  Nothing will break if it
 * doesn't work.
 */
#if defined(__i386__) || defined(__x86_64__) || defined(__vax__)
#define _PICOHASH_MD5_SET(n) (*(const uint32_t *)&ptr[(n)*4])
#define _PICOHASH_MD5_GET(n) _PICOHASH_MD5_SET(n)
#else
#define _PICOHASH_MD5_SET(n)                                                                                                       \
    (ctx->block[(n)] = (uint_fast32_t)ptr[(n)*4] | ((uint_fast32_t)ptr[(n)*4 + 1] << 8) | ((uint_fast32_t)ptr[(n)*4 + 2] << 16) |  \
                       ((uint_fast32_t)ptr[(n)*4 + 3] << 24))
#define _PICOHASH_MD5_GET(n) (ctx->block[(n)])
#endif

/*
 * This processes one or more 64-byte data blocks, but does NOT update
 * the bit counters.  There're no alignment requirements.
 */
static const void *_picohash_md5_body(_picohash_md5_ctx_t *ctx, const void *data, size_t size)
{
    const unsigned char *ptr;
    uint_fast32_t a, b, c, d;
    uint_fast32_t saved_a, saved_b, saved_c, saved_d;

    ptr = (const unsigned char*) data;

    a = ctx->a;
    b = ctx->b;
    c = ctx->c;
    d = ctx->d;

    do {
        saved_a = a;
        saved_b = b;
        saved_c = c;
        saved_d = d;

        /* Round 1 */
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, a, b, c, d, _PICOHASH_MD5_SET(0), 0xd76aa478, 7)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, d, a, b, c, _PICOHASH_MD5_SET(1), 0xe8c7b756, 12)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, c, d, a, b, _PICOHASH_MD5_SET(2), 0x242070db, 17)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, b, c, d, a, _PICOHASH_MD5_SET(3), 0xc1bdceee, 22)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, a, b, c, d, _PICOHASH_MD5_SET(4), 0xf57c0faf, 7)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, d, a, b, c, _PICOHASH_MD5_SET(5), 0x4787c62a, 12)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, c, d, a, b, _PICOHASH_MD5_SET(6), 0xa8304613, 17)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, b, c, d, a, _PICOHASH_MD5_SET(7), 0xfd469501, 22)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, a, b, c, d, _PICOHASH_MD5_SET(8), 0x698098d8, 7)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, d, a, b, c, _PICOHASH_MD5_SET(9), 0x8b44f7af, 12)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, c, d, a, b, _PICOHASH_MD5_SET(10), 0xffff5bb1, 17)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, b, c, d, a, _PICOHASH_MD5_SET(11), 0x895cd7be, 22)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, a, b, c, d, _PICOHASH_MD5_SET(12), 0x6b901122, 7)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, d, a, b, c, _PICOHASH_MD5_SET(13), 0xfd987193, 12)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, c, d, a, b, _PICOHASH_MD5_SET(14), 0xa679438e, 17)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_F, b, c, d, a, _PICOHASH_MD5_SET(15), 0x49b40821, 22)

        /* Round 2 */
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, a, b, c, d, _PICOHASH_MD5_GET(1), 0xf61e2562, 5)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, d, a, b, c, _PICOHASH_MD5_GET(6), 0xc040b340, 9)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, c, d, a, b, _PICOHASH_MD5_GET(11), 0x265e5a51, 14)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, b, c, d, a, _PICOHASH_MD5_GET(0), 0xe9b6c7aa, 20)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, a, b, c, d, _PICOHASH_MD5_GET(5), 0xd62f105d, 5)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, d, a, b, c, _PICOHASH_MD5_GET(10), 0x02441453, 9)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, c, d, a, b, _PICOHASH_MD5_GET(15), 0xd8a1e681, 14)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, b, c, d, a, _PICOHASH_MD5_GET(4), 0xe7d3fbc8, 20)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, a, b, c, d, _PICOHASH_MD5_GET(9), 0x21e1cde6, 5)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, d, a, b, c, _PICOHASH_MD5_GET(14), 0xc33707d6, 9)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, c, d, a, b, _PICOHASH_MD5_GET(3), 0xf4d50d87, 14)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, b, c, d, a, _PICOHASH_MD5_GET(8), 0x455a14ed, 20)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, a, b, c, d, _PICOHASH_MD5_GET(13), 0xa9e3e905, 5)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, d, a, b, c, _PICOHASH_MD5_GET(2), 0xfcefa3f8, 9)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, c, d, a, b, _PICOHASH_MD5_GET(7), 0x676f02d9, 14)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_G, b, c, d, a, _PICOHASH_MD5_GET(12), 0x8d2a4c8a, 20)

        /* Round 3 */
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, a, b, c, d, _PICOHASH_MD5_GET(5), 0xfffa3942, 4)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, d, a, b, c, _PICOHASH_MD5_GET(8), 0x8771f681, 11)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, c, d, a, b, _PICOHASH_MD5_GET(11), 0x6d9d6122, 16)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, b, c, d, a, _PICOHASH_MD5_GET(14), 0xfde5380c, 23)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, a, b, c, d, _PICOHASH_MD5_GET(1), 0xa4beea44, 4)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, d, a, b, c, _PICOHASH_MD5_GET(4), 0x4bdecfa9, 11)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, c, d, a, b, _PICOHASH_MD5_GET(7), 0xf6bb4b60, 16)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, b, c, d, a, _PICOHASH_MD5_GET(10), 0xbebfbc70, 23)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, a, b, c, d, _PICOHASH_MD5_GET(13), 0x289b7ec6, 4)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, d, a, b, c, _PICOHASH_MD5_GET(0), 0xeaa127fa, 11)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, c, d, a, b, _PICOHASH_MD5_GET(3), 0xd4ef3085, 16)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, b, c, d, a, _PICOHASH_MD5_GET(6), 0x04881d05, 23)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, a, b, c, d, _PICOHASH_MD5_GET(9), 0xd9d4d039, 4)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, d, a, b, c, _PICOHASH_MD5_GET(12), 0xe6db99e5, 11)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, c, d, a, b, _PICOHASH_MD5_GET(15), 0x1fa27cf8, 16)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_H, b, c, d, a, _PICOHASH_MD5_GET(2), 0xc4ac5665, 23)

        /* Round 4 */
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, a, b, c, d, _PICOHASH_MD5_GET(0), 0xf4292244, 6)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, d, a, b, c, _PICOHASH_MD5_GET(7), 0x432aff97, 10)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, c, d, a, b, _PICOHASH_MD5_GET(14), 0xab9423a7, 15)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, b, c, d, a, _PICOHASH_MD5_GET(5), 0xfc93a039, 21)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, a, b, c, d, _PICOHASH_MD5_GET(12), 0x655b59c3, 6)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, d, a, b, c, _PICOHASH_MD5_GET(3), 0x8f0ccc92, 10)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, c, d, a, b, _PICOHASH_MD5_GET(10), 0xffeff47d, 15)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, b, c, d, a, _PICOHASH_MD5_GET(1), 0x85845dd1, 21)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, a, b, c, d, _PICOHASH_MD5_GET(8), 0x6fa87e4f, 6)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, d, a, b, c, _PICOHASH_MD5_GET(15), 0xfe2ce6e0, 10)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, c, d, a, b, _PICOHASH_MD5_GET(6), 0xa3014314, 15)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, b, c, d, a, _PICOHASH_MD5_GET(13), 0x4e0811a1, 21)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, a, b, c, d, _PICOHASH_MD5_GET(4), 0xf7537e82, 6)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, d, a, b, c, _PICOHASH_MD5_GET(11), 0xbd3af235, 10)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, c, d, a, b, _PICOHASH_MD5_GET(2), 0x2ad7d2bb, 15)
        _PICOHASH_MD5_STEP(_PICOHASH_MD5_I, b, c, d, a, _PICOHASH_MD5_GET(9), 0xeb86d391, 21)

        a += saved_a;
        b += saved_b;
        c += saved_c;
        d += saved_d;

        ptr += 64;
    } while (size -= 64);

    ctx->a = a;
    ctx->b = b;
    ctx->c = c;
    ctx->d = d;

    return ptr;
}

inline void _picohash_md5_init(_picohash_md5_ctx_t *ctx)
{
    ctx->a = 0x67452301;
    ctx->b = 0xefcdab89;
    ctx->c = 0x98badcfe;
    ctx->d = 0x10325476;

    ctx->lo = 0;
    ctx->hi = 0;
}

inline void _picohash_md5_update(_picohash_md5_ctx_t *ctx, const void *data, size_t size)
{
    uint_fast32_t saved_lo;
    unsigned long used, free;

    saved_lo = ctx->lo;
    if ((ctx->lo = (saved_lo + size) & 0x1fffffff) < saved_lo)
        ctx->hi++;
    ctx->hi += size >> 29;

    used = saved_lo & 0x3f;

    if (used) {
        free = 64 - used;

        if (size < free) {
            memcpy(&ctx->buffer[used], data, size);
            return;
        }

        memcpy(&ctx->buffer[used], data, free);
        data = (const unsigned char *)data + free;
        size -= free;
        _picohash_md5_body(ctx, ctx->buffer, 64);
    }

    if (size >= 64) {
        data = _picohash_md5_body(ctx, data, size & ~(unsigned long)0x3f);
        size &= 0x3f;
    }

    memcpy(ctx->buffer, data, size);
}

inline void _picohash_md5_final(_picohash_md5_ctx_t *ctx, void *_digest)
{
    unsigned char *digest = (unsigned char *) _digest;
    unsigned long used, free;

    used = ctx->lo & 0x3f;

    ctx->buffer[used++] = 0x80;

    free = 64 - used;

    if (free < 8) {
        memset(&ctx->buffer[used], 0, free);
        _picohash_md5_body(ctx, ctx->buffer, 64);
        used = 0;
        free = 64;
    }

    memset(&ctx->buffer[used], 0, free - 8);

    ctx->lo <<= 3;
    ctx->buffer[56] = ctx->lo;
    ctx->buffer[57] = ctx->lo >> 8;
    ctx->buffer[58] = ctx->lo >> 16;
    ctx->buffer[59] = ctx->lo >> 24;
    ctx->buffer[60] = ctx->hi;
    ctx->buffer[61] = ctx->hi >> 8;
    ctx->buffer[62] = ctx->hi >> 16;
    ctx->buffer[63] = ctx->hi >> 24;

    _picohash_md5_body(ctx, ctx->buffer, 64);

    digest[0] = ctx->a;
    digest[1] = ctx->a >> 8;
    digest[2] = ctx->a >> 16;
    digest[3] = ctx->a >> 24;
    digest[4] = ctx->b;
    digest[5] = ctx->b >> 8;
    digest[6] = ctx->b >> 16;
    digest[7] = ctx->b >> 24;
    digest[8] = ctx->c;
    digest[9] = ctx->c >> 8;
    digest[10] = ctx->c >> 16;
    digest[11] = ctx->c >> 24;
    digest[12] = ctx->d;
    digest[13] = ctx->d >> 8;
    digest[14] = ctx->d >> 16;
    digest[15] = ctx->d >> 24;

    memset(ctx, 0, sizeof(*ctx));
}


#endif
