/**
 * Compile command : gcc -o gfalt_copyfile gfalt_copyfile.c `pkg-config --libs --cflags gfal_transfer`
 */

#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <errno.h>
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
    generate_random_uri(src_root, "checksum_source", src_uri, 2048);
    if (generate_file_if_not_exists(handle, src_uri, "file:///etc/hosts", &tmp_err) != 0) {
        fprintf(stderr, "Could not generate the source: %s", tmp_err->message);
        g_assert_not_reached();
    }

    // Tests
    char chk_buffer[2048];
    char dst_uri[2048];
    char dst_uri2[2048];
    generate_random_uri(dst_root, "checksum", dst_uri, 2048);
    generate_random_uri(dst_root, "checksum2", dst_uri2, 2048);

    printf(" get source checksum \n");
    if (gfal2_checksum(handle, src_uri, "ADLER32", 0, 0, chk_buffer, 2048, &tmp_err) != 0) {
        g_assert_not_reached();
    }
    g_assert(tmp_err == NULL);
    printf(" source checksum %s \n", chk_buffer);

    printf(" create params without replace but with checksum verification\n");
    gfalt_params_t params = gfalt_params_handle_new(&tmp_err);
    g_assert(tmp_err == NULL);
    gfalt_set_checksum_check(params, TRUE, &tmp_err);
    g_assert(tmp_err == NULL);
    gfalt_set_user_defined_checksum(params, "ADLER32", chk_buffer, &tmp_err);
    g_assert(tmp_err == NULL);

    // begin copy
    if (gfalt_copy_file(handle, params, src_uri, dst_uri, &tmp_err) != 0) {
        printf(" error while the file transfer %d : %s.\n", tmp_err->code, tmp_err->message);
        g_assert_not_reached();
    }
    else
        printf(" transfer sucessfull, valid initial copy! \n");

    printf(" add params replace \n");

    printf(" copyfile 2 with bad checksum  \n");
    gfalt_set_user_defined_checksum(params, "ADLER32", "aaaaaaa", &tmp_err);

    if (gfalt_copy_file(handle, params, src_uri, dst_uri2, &tmp_err) == 0) {
        g_assert_not_reached();
    }
    g_assert(tmp_err->code == EIO);
    g_clear_error(&tmp_err);

    g_assert(gfal_unlink(src_uri) == 0);
    g_assert(gfal_unlink(dst_uri) == 0);
    // dst_uri2 should not exist, but just in case
    gfal_unlink(dst_uri2);

    gfalt_params_handle_delete(params, &tmp_err);
    gfal2_context_free(handle);
    return 0;
}
