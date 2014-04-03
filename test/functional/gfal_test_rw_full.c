/**
 * Compile command : gcc gfal_testrw_full.c `pkg-config --libs --cflags gfal2`
 */

#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <glib.h>

#include <common/gfal_lib_test.h>

#define BLKLEN 65536

int main(int argc, char **argv)
{
    int fd;
    int i;

    int rc;

    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE); // switch Gfal in verbose mode
    if (argc != 3) {
        fprintf(stderr, "usage: %s base_path size_file \n", argv[0]);
        exit(1);
    }

    long size = atol(argv[2]);

    char buff_name[2048];
    char ibuf[size];
    char obuf[size];
    srand(time(NULL ));

    printf(" create data of size %ld \n", size);
    for (i = 0; i < size; i++)
        obuf[i] = i;

    generate_random_uri(argv[1], "testrw_full", buff_name, 2048);

    // try to get a enoent
    printf("try to access unexisting file %s\n", buff_name);
    if ((fd = gfal_open(buff_name, O_RDONLY, 0)) > 0) {
        g_assert_not_reached()
        ;
        exit(1);
    }
    g_assert(gfal_posix_code_error() == ENOENT);
    g_assert(errno == ENOENT);
    errno = 0;
    gfal_posix_clear_error();

    printf("creating file name %s\n", buff_name);
    if ((fd = gfal_open(buff_name, O_WRONLY | O_CREAT, 0644)) < 0) {
        gfal_posix_check_error();
        exit(1);
    }

    printf("open successful, fd = %d\n", fd);

    if ((rc = gfal_write(fd, obuf, size)) != size) {
        gfal_posix_check_error();
        (void) gfal_close(fd);
        exit(1);
    }
    printf("write successful\n");

    if ((rc = gfal_close(fd)) < 0) {
        gfal_posix_check_error();
        exit(1);
    }
    printf("close successful\n");

    printf("reading back %s\n", buff_name);
    if ((fd = gfal_open(buff_name, O_RDONLY, 0)) < 0) {
        gfal_posix_check_error();
        exit(1);
    }
    printf("open successful, fd = %d\n", fd);

    if ((rc = gfal_read(fd, ibuf, size)) != size) {
        gfal_posix_check_error();
        (void) gfal_close(fd);
        exit(1);
    }
    printf("read successful\n");

    if ((rc = gfal_close(fd)) < 0) {
        gfal_posix_check_error();
        exit(1);
    }
    printf("close successful\n");

    for (i = 0; i < size; i++) {
        if (ibuf[i] != obuf[i]) {
            fprintf(stderr, "compare failed at offset %d\n", i);
            exit(1);
        }
    }
    printf("compare successful\n");

    printf("%s\n", buff_name);
    if (gfal_unlink(buff_name) != 0) {
        gfal_posix_check_error();
        exit(1);
    }

    return 0;
}
