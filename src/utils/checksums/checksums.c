#include <string.h>
#include "checksums.h"


static const char* _no_zeros(const char* str)
{
    const char* p = str;
    while (*p == '0')
        ++p;
    return p;
}



int gfal_compare_checksums(const char* chk1, const char* chk2, size_t len)
{
    const char* c1 = _no_zeros(chk1);
    const char* c2 = _no_zeros(chk2);

    return strncasecmp(c1, c2, len);
}
