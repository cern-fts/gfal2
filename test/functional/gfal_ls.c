/**
 * Compile command : gcc gfal_testdir.c `pkg-config --libs --cflags gfal2`
 */

#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <posix/gfal_posix_api.h>

int main(int argc, char **argv)
{
    struct dirent *d;
    DIR *dir;

    if (argc != 2) {
        fprintf(stderr, "usage: %s filename\n", argv[0]);
        return 1;
    }

    gfal_set_verbose(10);

    if ((dir = gfal_opendir(argv[1])) == NULL ) {
        char errbuffer[512];
        perror("gfal_opendir");
        gfal_posix_strerror_r(errbuffer, sizeof(errbuffer));
        fprintf(stderr, "%s\n", errbuffer);
        return 1;
    }

    while ((d = gfal_readdir(dir))) {
        printf("%s\n", d->d_name);
    }

    if (gfal_closedir(dir) < 0) {
        perror("gfal_closedir");
        return 1;
    }
    return 0;
}
