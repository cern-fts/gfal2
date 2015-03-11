/**
 * Compile command : gcc gfal_test_mkdir_unlink.c `pkg-config --libs --cflags gfal2`
 */

#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#include <common/gfal_lib_test.h>
#define BLKLEN 65536

/**
 *
 * Description: This test creates a directory then tries to unlink it.
 * It is the failure of the unlink operation which is checked.
 * Unlink on directories should fail.
 * The test will also fail if the cleanup (rmdir) of the directory fails.
*/

int main(int argc, char **argv)
{
   int rc,ret;

   char buff[2048];
   gfal2_log_set_level(G_LOG_LEVEL_DEBUG);

   if (argc != 2) {
	   fprintf (stderr, "usage: %s filename\n", argv[0]);
	   exit (1);
   }

   // create a random file uri file
   generate_random_uri(argv[1], "test_mkdir_unlink_dir_", buff, 2048);

   fprintf(stderr, "mkdir %s\n", buff);
   if ((rc = gfal_mkdir(buff, 777)) < 0) {
	gfal_posix_check_error();
	exit (1);
   }

   fprintf(stderr, "try unlink (must fail) %s\n", buff);
    if ((rc = gfal_unlink(buff)) < 0) {
        ret = 0;
    }
    else {
        fprintf(stderr, "unlink worked for a directory!\n");
        exit (1);
    }

// Removing the directory

   fprintf(stderr, "rmdir %s\n", buff);
   if (( rc = gfal_rmdir(buff)) < 0) {
	gfal_posix_check_error();
	exit (1);
   }
   return ret;
}
