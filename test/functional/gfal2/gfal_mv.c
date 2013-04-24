#include <stdio.h>
#include <stdlib.h>
#include "gfal_api.h"
#include "gfal_posix_internal.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf (stderr, "usage: %s original new\n", argv[0]);
        return 1;
    }

    gfal_set_verbose(10);

    const char *src = argv[1];
    const char *dst = argv[2];

    if (gfal_rename(src, dst) != 0) {
        perror ("gfal_rename");
        fprintf(stderr, "%s\n", (*gfal_posix_get_last_error())->message);
        return 1;
    }

    return 0;
}
