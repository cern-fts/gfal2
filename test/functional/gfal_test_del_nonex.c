/**
 * Compile command : gcc gfal_test_del_nonex.c `pkg-config --libs --cflags gfal2`
 */

#include <stdio.h>
#include <gfal_api.h>

int main(int argc, char **argv)
{
    int err;

    gfal2_log_set_level(G_LOG_LEVEL_DEBUG);

    char buff[2048];

    if (argc != 2) {
        fprintf(stderr, "usage: %s valid_dir\n", argv[0]);
        exit(1);
    }

    // create a enoent file
    g_strlcpy(buff, argv[1], 2048);
    g_strlcat(buff, "/", 2048);
    g_strlcat(buff, "testfileunlink_enoent", 2048);

    printf("unlinking (deleting) the file: %s\n", buff);
    err = gfal_unlink(buff);
    err = gfal_posix_code_error();
    if (err != ENOENT) {
        return 1;
    }
    else {
        return 0;
    }
}
