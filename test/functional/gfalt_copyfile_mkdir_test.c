/**
 * Compile command : gcc -o gfalt_copyfile gfalt_copyfile.c `pkg-config --libs --cflags gfal_transfer`
 */

#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <transfer/gfal_transfer.h>

#include <common/gfal_lib_test.h>

static int test_copy_nested(const char* root_dir,
        const char* src_uri,
        gfal2_context_t handle, gfalt_params_t params)
{
    char first_level[2048];
    char second_level[2048];
    char third_level[2048];
    char dst_uri[2048];

    generate_random_uri(root_dir, "generate_folder", first_level, 2048);
    generate_random_uri(first_level, "generate_folder2", second_level, 2048);
    generate_random_uri(second_level, "generate_folder3", third_level, 2048);
    generate_random_uri(third_level, "generate_dest_file", dst_uri, 2048);

    GError *tmp_err = NULL;
    printf(" begin to copyfile to child dir  from %s to %s", src_uri, dst_uri);
    int ret = gfalt_copy_file(handle, params, src_uri, dst_uri, &tmp_err);

    g_assert(gfal_unlink(dst_uri) == 0);
    g_assert(gfal_rmdir(third_level) == 0);
    g_assert(gfal_rmdir(second_level) == 0);
    g_assert(gfal_rmdir(first_level) == 0);

    if (ret != 0) {
        printf(" error while the file transfer %d : %s.\n", tmp_err->code, tmp_err->message);
        return -1;
    }
    else
        printf(" transfer sucessfull ! \n");

    return 0;
}

static int test_copy_root(const char* root_dir,
        const char* src_uri,
        gfal2_context_t handle, gfalt_params_t params)
{
    char dst_uri[2048];

    generate_random_uri(root_dir, "simple_file_at_root", dst_uri, 2048);

    GError *tmp_err = NULL;
    printf(" begin to copyfile to root dir from %s to %s", src_uri, dst_uri);
    int ret = gfalt_copy_file(handle, params, src_uri, dst_uri, &tmp_err);

    g_assert(gfal_unlink(dst_uri) == 0);

    if (ret != 0) {
        printf(" error while the file transfer %d : %s.\n", tmp_err->code, tmp_err->message);
        return -1;
    }
    else
        printf(" transfer sucessfull ! \n");
    return 0;
}


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
    gfal2_context_t handle;
    GError *tmp_err = NULL;
    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);
    if ((handle = gfal2_context_new(&tmp_err)) == NULL ) {
        printf(" bad initialization %d : %s.\n", tmp_err->code,
                tmp_err->message);
        return -1;
    }

    gfalt_params_t params = gfalt_params_handle_new(NULL);

    int res_params = gfalt_set_create_parent_dir(params, TRUE, &tmp_err);
    g_assert(res_params == 0 && tmp_err==NULL);
    g_assert(gfalt_get_create_parent_dir(params, &tmp_err) == TRUE && tmp_err == NULL);

    // create source if not there
    char src_uri[2048];
    generate_random_uri(src_root, "copyfile_mkdir_source", src_uri, 2048);
    if (generate_file_if_not_exists(handle, src_uri, "file:///etc/hosts", &tmp_err) != 0) {
        fprintf(stderr, "Could not generate the source: %s", tmp_err->message);
        g_assert_not_reached();
    }

    int ret;
    ret = test_copy_nested(dst_root, src_uri, handle, params);
    ret += test_copy_root(dst_root, src_uri, handle, params);

    g_assert(gfal_unlink(src_uri) == 0);

    gfalt_params_handle_delete(params, NULL);
    gfal2_context_free(handle);
    return ret;
}
