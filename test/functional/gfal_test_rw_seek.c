// random read
// regression for DMC-531
// random write is not tested because most storage elements will fail if this is attempted,
// Sequential writes calling seek should work, however

#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include <common/gfal_lib_test.h>


int main(int argc, char **argv)
{
    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE);

    if (argc != 4) {
        fprintf(stderr, "usage: %s base_path <block size> <file size> \n", argv[0]);
        exit(1);
    }

    long block_size = atol(argv[2]);
    long file_size = atol(argv[3]);
    if (file_size / 2 < block_size) {
        fprintf(stderr, "block size should be less than half the file size\n");
        exit(1);
    }

    // Create source file
    char file_name[2048];
    generate_random_uri(argv[1], "testrw_seek", file_name, 2048);

    char buffer[file_size];
    long i;
    for (i = 0; i < file_size; ++i)
        buffer[i] = i;

    printf("Creating file %s\n", file_name);
    int fd = gfal_open(file_name, O_WRONLY | O_CREAT, 0644);
    if (fd < 0) {
        gfal_posix_check_error();
        exit(1);
    }

    printf("open successful, fd = %d\n", fd);

    i = 0;
    while (i < file_size) {
        long j = ((i + block_size) < file_size) ? block_size : file_size - i;

        if (gfal_lseek(fd, i, SEEK_SET) < 0) {
            gfal_posix_check_error();
            (void) gfal_close(fd);
            exit(1);
        }
        if (gfal_write(fd, buffer + i, j) != j) {
            gfal_posix_check_error();
            (void) gfal_close(fd);
            exit(1);
        }

        printf("write successful from %ld to %ld\n", i, i + j);

        i += j;
    }

    if (gfal_close(fd) < 0) {
        gfal_posix_check_error();
        exit(1);
    }
    printf("close successful\n");

    // Now, try a "random" read
    off_t offset = file_size / 2;

    fd = gfal_open(file_name, O_RDONLY, 0);
    if (fd < 0) {
        gfal_posix_check_error();
        exit(1);
    }
    printf("open successful, fd = %d\n", fd);

    gfal_lseek(fd, offset, SEEK_SET);

    char test_buffer[block_size];
    long rc = gfal_read(fd, test_buffer, sizeof(test_buffer));
    if (rc < 0) {
        gfal_posix_check_error();
        exit(1);
    }

    if ((rc = gfal_close(fd)) < 0) {
        gfal_posix_check_error();
        exit(1);
    }
    printf("close successful\n");

    g_assert(gfal_unlink(file_name) == 0);

    for (i = offset; i < block_size; i++) {
        if (buffer[i] != test_buffer[i]) {
            fprintf(stderr, "compare failed at offset %ld\n", i);
            exit(1);
        }
    }

    printf("compare successful\n");
    return 0;
}
