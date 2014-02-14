#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Compares the checksum 1 with the checksum 2 not taking into
 * account case and heading zeros.
 * The return value is the same as for strncasecmp
 */
int gfal_compare_checksums(const char* chk1, const char* chk2, size_t len);


// md5 checksum camculation

typedef unsigned int gfal2_md5_u32plus;

typedef struct {
    gfal2_md5_u32plus lo, hi;
    gfal2_md5_u32plus a, b, c, d;
    unsigned char buffer[64];
    gfal2_md5_u32plus block[16];
} GFAL_MD5_CTX;


void gfal2_md5_init(GFAL_MD5_CTX *ctx);

void gfal2_md5_update(GFAL_MD5_CTX *ctx, const void *data, unsigned long size);

void gfal2_md5_final(unsigned char *result, GFAL_MD5_CTX *ctx);

void gfal2_md5_to_hex_string(char* bytes, char* hex);

#ifdef __cplusplus
}
#endif
