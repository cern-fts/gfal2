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
    GError * tmp_err = NULL;
    gfal2_context_t handle;
    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);
    if ((handle = gfal2_context_new(&tmp_err)) == NULL ) {
        printf(" bad initialization %d : %s.\n", tmp_err->code, tmp_err->message);
        return -1;
    }

    // create source if not there
    char src_uri[2048];
    generate_random_uri(src_root, "replace", src_uri, 2048);
    if (generate_file_if_not_exists(handle, src_uri, "file:///etc/hosts", &tmp_err) != 0) {
        fprintf(stderr, "Could not generate the source: %s", tmp_err->message);
        g_assert_not_reached();
    }

    // begin copy
    char dst_uri[2048];
    generate_random_uri(dst_root, "replace_source", dst_uri, 2048);

    if (gfalt_copy_file(handle, NULL, src_uri, dst_uri, &tmp_err) != 0) {
        printf(" error while the file transfer %d : %s.\n", tmp_err->code, tmp_err->message);
        g_assert_not_reached();
        return -1;
    }
    else
        printf(" transfer sucessfull, valid initial copy! \n");

    // copy again -> should fail
    printf(" try to copy again without replace flag \n");
    if (gfalt_copy_file(handle, NULL, src_uri, dst_uri, &tmp_err) != 0) {
        printf(" transfer failed, like wanted, %d \n", gfal_posix_code_error());
        g_assert( tmp_err->code == EEXIST);
        g_clear_error(&tmp_err);
    }
    else {
        printf(" copy should fail fatal \n");
        g_assert_not_reached();
    }

    printf(" create params replace \n");
    gfalt_params_t params = gfalt_params_handle_new(&tmp_err);
    g_assert(tmp_err==NULL);
    gfalt_set_replace_existing_file(params, TRUE, &tmp_err);
    g_assert(tmp_err==NULL);

    printf(" try replace transfer \n");
    if (gfalt_copy_file(handle, params, src_uri, dst_uri, &tmp_err) != 0) {
        printf(" error while the file transfer %d : %s.\n", tmp_err->code, tmp_err->message);
        g_assert_not_reached();
    }
    else
        printf(" second transfer is a success  \n");

    printf("delete files, and transfer again with replace parameter \n");
    if (gfal_unlink(dst_uri) != 0) {
        gfal_posix_check_error();
        g_assert_not_reached();
    }

    printf(" try re-do transfer \n");
    if (gfalt_copy_file(handle, params, src_uri, dst_uri, &tmp_err) != 0) {
        printf(" error while the file transfer %d : %s.\n", tmp_err->code, tmp_err->message);
        g_assert_not_reached();
    }
    else
        printf(" third transfer is a success  \n");

    printf("remove replace flag and try again \n");
    gfalt_set_replace_existing_file(params, FALSE, &tmp_err);
    if (gfalt_copy_file(handle, params, src_uri, dst_uri, &tmp_err) != 0) {
        printf(" transfer failed, like wanted , %d  \n",
                gfal_posix_code_error());
        g_assert( tmp_err->code == EEXIST);
        g_clear_error(&tmp_err);
    }
    else {
        printf(" copy should fail fatal \n");
        g_assert_not_reached();
    }

    if (gfal2_unlink(handle, src_uri, &tmp_err) != 0) {
        gfal_posix_check_error();
        g_assert_not_reached();
    }
    if (gfal2_unlink(handle, dst_uri, &tmp_err) != 0) {
        gfal_posix_check_error();
        g_assert_not_reached();
    }

    gfalt_params_handle_delete(params, &tmp_err);
    gfal2_context_free(handle);
    return 0;
}

