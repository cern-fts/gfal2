/*
 * Copyright (c) CERN 2013-2015
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
