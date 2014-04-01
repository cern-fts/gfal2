// sequential little read/write 
/**
 * Compile command : gcc gfal_testrw_seq.c `pkg-config --libs --cflags gfal2`
 */

#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include <common/gfal_lib_test.h>

#define BLKLEN 65536

int main(int argc, char **argv)
{
    int fd;

    int rc;

    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE); // switch Gfal in verbose mode
    if (argc != 4) {
        fprintf(stderr, "usage: %s base_path size_read size_file \n", argv[0]);
        exit(1);
    }

    long size_read = atol(argv[2]);
    long size_total = atol(argv[3]);
    if (size_read > size_total / 2) {
        fprintf(stderr, "constrain size_total > size_read*2 not respected \n");
        exit(1);
    }

    char buff_name[2048];
    char ibuf[size_total];
    char obuf[size_total];
    long i, j, n;
    srand(time(NULL ));

    printf(" create data of size %ld \n", size_total);
    for (i = 0; i < size_total; i++)
        obuf[i] = i;

    generate_random_uri(argv[1], "testrw_full_seq", buff_name, 2048);

    printf("creating file name %s\n", buff_name);
    if ((fd = gfal_open(buff_name, O_WRONLY | O_CREAT, 0644)) < 0) {
        gfal_posix_check_error();
        exit(1);
    }

    printf("open successful, fd = %d\n", fd);

    i = j = n = 0;
    while (i < size_total) {
        j = ((i + size_read) < size_total) ? size_read : size_total - i;

        printf("write successful %ld from %ld of %ld\n", n, i, j);
        if ((rc = gfal_write(fd, obuf + i, j)) != j) {
            gfal_posix_check_error();
            (void) gfal_close(fd);
            exit(1);
        }
        i += j;
        printf("write successful %ld \n", n++);
    }

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

    i = j = n = 0;
    while (i < size_total) {
        j = ((i + size_read) < size_total) ? size_read : size_total - i;

        if ((rc = gfal_read(fd, ibuf + i, j)) < 0) {
            gfal_posix_check_error();
            (void) gfal_close(fd);
            exit(1);
        }
        i += (long) rc;
        printf("read successful %ld of size %d \n", n++, rc);
    }

    if ((rc = gfal_close(fd)) < 0) {
        gfal_posix_check_error();
        exit(1);
    }
    printf("close successful\n");

    for (i = 0; i < size_total; i++) {
        if (ibuf[i] != obuf[i]) {
            fprintf(stderr, "compare failed at offset %ld\n", i);
            exit(1);
        }
    }

    g_assert(gfal_unlink(buff_name) == 0);

    printf("compare successful\n");
    return 0;
}
