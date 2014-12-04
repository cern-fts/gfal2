/**
 * Compile command : gcc gfal_test_rmdir_full.c `pkg-config --libs --cflags gfal2`
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <gfal_api.h>

#include <common/gfal_lib_test.h>

static void test_enoent(const char* test_root)
{
    char dir_name[2048];
    generate_random_uri(test_root, "test_rmdir_full_enoent", dir_name, 2048);

    printf ("check enoent directory  %s ...\n", dir_name);
    if(gfal_rmdir(dir_name) == 0) { // must not exist
        gfal_posix_check_error();
        g_assert_not_reached();
    }
    printf ("enoent code : %d  ...\n", gfal_posix_code_error());
    g_assert(gfal_posix_code_error() == ENOENT);
    gfal_posix_clear_error();
    g_assert(gfal_posix_code_error() == 0);
}

static void test_eexist(const char* test_root)
{
    char dir_name[2048];
    generate_random_uri(test_root, "testrmdir_full_exist", dir_name, 2048);

    printf ("create and delete directory  %s ...\n", dir_name);
    if(gfal_mkdir(dir_name, 0777) != 0) {
       gfal_posix_check_error();
       g_assert_not_reached();
    }
    g_assert(gfal_posix_code_error() == 0);
    if(gfal_rmdir(dir_name) != 0) {
       gfal_posix_check_error();
       g_assert_not_reached();
    }
    g_assert(gfal_posix_code_error() == 0);

    // Make sure it is not there!
    g_assert(gfal_access(dir_name, F_OK) < 0);
    gfal_posix_clear_error();
}

static void test_eexist2(const char* test_root)
{
    char dir_name[2048];
    generate_random_uri(test_root, "testrmdir_full_exist2", dir_name, 2048);

    g_strlcat(dir_name, "/", 2048);
    printf ("create and delete directory with slash  %s   ...\n", dir_name);
    if(gfal_mkdir(dir_name, 0777) != 0) {
        gfal_posix_check_error();
        g_assert_not_reached();
    }
    g_assert(gfal_posix_code_error() == 0);
    if(gfal_rmdir(dir_name) != 0) {
        gfal_posix_check_error();
        g_assert_not_reached();
    }
    g_assert(gfal_posix_code_error() == 0);

    // Make sure it is not there!
    g_assert(gfal_access(dir_name, F_OK) < 0);
    gfal_posix_clear_error();
}

static void test_nested(const char* test_root)
{
    char dir_name[2048];
    char dir_nested_name[2048];

    generate_random_uri(test_root, "testrmdir_full_eaccess", dir_name, 2048);
    generate_random_uri(dir_name, "testdirinside", dir_nested_name, 2048);

    printf ("create eaccess and enotempty dir   %s  %s ...\n", dir_name, dir_nested_name);
    if(gfal_mkdir(dir_name, 0777) != 0) {
        gfal_posix_check_error();
        g_assert_not_reached();
    }
    g_assert(gfal_posix_code_error() == 0);


    if(gfal_mkdir(dir_nested_name, 0777) != 0) {
        gfal_posix_check_error();
        g_assert_not_reached();
    }
    g_assert(gfal_posix_code_error() == 0);

    printf ("try enotempty   %s   ...\n", dir_name);
    if(gfal_rmdir(dir_name) == 0) {  // must ENOTEMPTY
        gfal_posix_check_error();
        g_assert_not_reached();
    }
    printf(" err code enotempty : %d\n", gfal_posix_code_error());
    g_assert(gfal_posix_code_error() == ENOTEMPTY);
    gfal_posix_clear_error();
    g_assert(gfal_posix_code_error() == 0);

    if(gfal_chmod(dir_name, 0000) != 0) {
        gfal_posix_check_error();
        g_assert_not_reached();
    }
    g_assert(gfal_posix_code_error() == 0);

    // Skip this bit if file:// is used, since the rmdir will succeed
    if (strncmp("file:", dir_nested_name, 5) != 0) {
        printf ("try eacess %s   ...\n", dir_nested_name);
        if(gfal_rmdir(dir_nested_name) == 0) {  // must eaccess
            gfal_posix_check_error();
            g_assert_not_reached();
        }
        g_assert(gfal_posix_code_error() == EACCES);
        gfal_posix_clear_error();
        g_assert(gfal_posix_code_error() == 0);
    }
    else {
        printf("skipping eacess test for file://\n");
    }

    // Clean up
    if(gfal_chmod(dir_name, 0775) != 0) {
        gfal_posix_check_error();
        g_assert_not_reached();
    }

    gfal_rmdir(dir_nested_name);
    gfal_rmdir(dir_name);
}

static void test_enotdir(const char* test_root)
{
    char valid_file[2048];
    generate_random_uri(test_root, "testrmdir_enotdir", valid_file, 2048);

    if (strncmp(test_root, "lfc:/", 5) == 0 || strncmp(test_root, "lfn:/", 5) == 0)
    {
        printf("skip test_enotdir for lfc\n");
        return;
    }

    // Create a file
    int fd = gfal_creat(valid_file, 0775);
    g_assert(fd >= 0);
    gfal_write(fd, "1234", 4);
    g_assert(gfal_close(fd) == 0);

    if(gfal_rmdir(valid_file) == 0) {  // must ENOTDIR
        gfal_posix_check_error();
        g_assert_not_reached();
    }

    printf(" enotdir err : %d\n", gfal_posix_code_error())  ;
    g_assert(gfal_posix_code_error() == ENOTDIR);
    gfal_posix_clear_error();
    g_assert(gfal_posix_code_error() == 0);

    gfal_unlink(valid_file);
}

static int is_dav(const char* surl)
{
    return strncmp(surl, "dav:", 4) == 0 ||
           strncmp(surl, "davs:", 5) == 0 ||
           strncmp(surl, "http:", 5) == 0 ||
           strncmp(surl, "https:", 6) == 0 ||
           strncmp(surl, "dav+3rd:", 8) == 0 ||
           strncmp(surl, "davs+3rd:", 9) == 0 ||
           strncmp(surl, "http+3rd:", 9) == 0 ||
           strncmp(surl, "https+3rd:", 9) == 0;
}

int main(int argc, char **argv)
{
	char *test_root;

	if (argc < 2) {
		fprintf (stderr, "usage: %s test-root\n", argv[0]);
		exit (1);
	}

	test_root = argv[1];

	gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_DEBUG | GFAL_VERBOSE_VERBOSE);

	test_enoent(test_root);
	test_eexist(test_root);
	test_eexist2(test_root);

	// dav does not support chmod AND it will remove directories recursively,
	// so skip this test
	if (!is_dav(test_root))
	    test_nested(test_root);

	test_enotdir(test_root);

    printf ("All is ok.\n");
	return 0;
}
