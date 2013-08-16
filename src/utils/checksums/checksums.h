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

#ifdef __cplusplus
}
#endif
