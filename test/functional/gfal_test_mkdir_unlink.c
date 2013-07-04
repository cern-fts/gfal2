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
 * Unlink on directoies should fail.
 * The test will also fail if the cleanup (rmdir) of the directory fails.
*/

int main(int argc, char **argv)
{
   int rc,ret;

   char buff[2048];
   gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE);	// switch Gfal in verbose mode
   if (argc != 2) {
	   fprintf (stderr, "usage: %s filename\n", argv[0]);
	   exit (1);
   }

   // create a random file uri file
   generate_random_uri(argv[1], "test_mkdir_unlink_file_", buff, 2048);

   if ((rc = gfal_mkdir(buff, 777)) < 0) {
	gfal_posix_check_error();
	exit (1);
   }

   if ((rc = gfal_unlink (buff)) < 0) {
        ret = 0;
   } else { ret = 1;}

// Removing the directory

   if (( rc = gfal_rmdir(buff)) < 0) {
	gfal_posix_check_error();
	exit (1);
   }
   return ret;	
}
