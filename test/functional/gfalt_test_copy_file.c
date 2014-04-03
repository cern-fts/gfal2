/**
 * Compile command : gcc -o gfalt_copyfile gfalt_copyfile.c `pkg-config --libs --cflags gfal_transfer`
 */

#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <transfer/gfal_transfer.h>

#include <common/gfal_lib_test.h>

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf(" Usage %s [src_dir] [[dst_dir]]\n", argv[0]);
        return 1;
    }
    const char* src_root = argv[1];
    const char* dst_root;
    if (argc >= 3)
        dst_root = argv[2];
    else
        dst_root = src_root;

    // initialize gfal
    GError * tmp_err = NULL; // classical GError/glib error management
    gfal2_context_t handle;
    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);
    if ((handle = gfal2_context_new(&tmp_err)) == NULL ) {
        printf(" bad initialization %d : %s.\n", tmp_err->code, tmp_err->message);
        return -1;
    }

    // Source uri
    char src_uri[2048];
    generate_random_uri(src_root, "copyfile_source", src_uri, 2048);
    if (generate_file_if_not_exists(handle, src_uri, "file:///etc/hosts", &tmp_err) != 0) {
        fprintf(stderr, "Could not generate the source: %s", tmp_err->message);
        g_assert_not_reached();
    }

    // begin copy
    char dst_uri[2048];
    generate_random_uri(dst_root, "copyfile", dst_uri, 2048);

    printf(" begin to copyfile from %s to %s", src_uri, dst_uri);
    if (gfalt_copy_file(handle, NULL, src_uri, dst_uri, &tmp_err) != 0) {
        printf(" error while the file transfer %d : %s.\n", tmp_err->code, tmp_err->message);
        return -1;
    }
    else
        printf(" transfer sucessfull ! \n");

    g_assert(gfal_unlink(src_uri) == 0);
    g_assert(gfal_unlink(dst_uri) == 0);

    gfal2_context_free(handle);
    return 0;
}
