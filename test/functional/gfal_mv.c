#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <posix/gfal_posix_api.h>

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
        char errbuffer[512];
        perror ("gfal_rename");
        gfal_posix_strerror_r(errbuffer, sizeof(errbuffer));
        fprintf(stderr, "%s\n", errbuffer);
        return 1;
    }

    return 0;
}
