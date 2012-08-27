/**
 * Compile command : gcc gfal_test_del.c `pkg-config --libs --cflags gfal2`
 */

#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#define BLKLEN 65536

/**
 *
 * Description: This test creates and writes an deletes a short file
 * It is the successfull deletion which is checked by this test
*/

int main(int argc, char **argv)
{
   int fd;
   int i;
   char buf[BLKLEN];
   int rc,err;

   // gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE);	// switch Gfal in verbose mode
   if (argc != 2) {
	   fprintf (stderr, "usage: %s filename\n", argv[0]);
	   exit (1);
   }

   for (i = 0; i < BLKLEN; i++)
	   buf[i] = i;

   printf ("creating file %s\n", argv[1]);
   if ((fd = gfal_open (argv[1], O_WRONLY|O_CREAT, 0644)) < 0) {
		gfal_posix_check_error();
		exit (1);
   }
   printf ("open successful, fd = %d\n", fd);

   if ((rc = gfal_write (fd, buf, BLKLEN)) != BLKLEN) {
        gfal_posix_check_error();
	(void) gfal_close (fd);
	exit (1);
   }
   printf ("write successful\n");

   if ((rc = gfal_close (fd)) < 0) {
	gfal_posix_check_error();
	exit (1);
   }
   printf ("close successful\n");

  if ((rc =gfal_unlink (argv[1])) < 0) {
        gfal_posix_check_error();
	exit(1);
  }

  rc =gfal_unlink (argv[1]); 
  gfal_posix_check_error();
  err = gfal_posix_code_error();
  if (err != ENOENT) {
	exit(1);
  } else {
	exit(0);
  }	
}
