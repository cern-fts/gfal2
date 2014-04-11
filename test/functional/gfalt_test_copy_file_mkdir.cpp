/**
 * Compile command : gcc -o gfalt_copyfile gfalt_copyfile.c `pkg-config --libs --cflags gfal_transfer`
 */
#include <gtest/gtest.h>

#include <gfal_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils/exceptions/gerror_to_cpp.h>
#include <transfer/gfal_transfer.h>

#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>

#include <list>


class CopyTestMkdir: public testing::Test {
public:
    static const char* source_root;
    static const char* destination_root;

    char source[2048];
    char destination[2048];
    gfal2_context_t handle;
    gfalt_params_t params;

    std::list<std::string> directories;

    CopyTestMkdir() {
        GError *error = NULL;
        handle =  gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
        params = gfalt_params_handle_new(NULL);
        gfalt_set_create_parent_dir(params, TRUE, NULL);
    }

    virtual ~CopyTestMkdir() {
        gfal2_context_free(handle);
        gfalt_params_handle_delete(params, NULL);
    }

    virtual void SetUp() {
        generate_random_uri(source_root, "copyfile_replace_source", source, 2048);

        RecordProperty("Source", source);
        RecordProperty("Destination", source);

        GError* error = NULL;
        int ret = generate_file_if_not_exists(handle, source, "file:///etc/hosts", &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }

    virtual void TearDown() {
        gfal_unlink(source);
        gfal_unlink(destination);

        std::list<std::string>::const_iterator i;
        for (i = directories.begin(); i != directories.end(); ++i)
            gfal_rmdir(i->c_str());
        directories.clear();
    }
};

const char* CopyTestMkdir::source_root;
const char* CopyTestMkdir::destination_root;


TEST_F(CopyTestMkdir, CopyNested)
{
    char first_level[2048];
    char second_level[2048];
    char third_level[2048];

    generate_random_uri(destination_root, "generate_folder", first_level, 2048);
    generate_random_uri(first_level, "generate_folder2", second_level, 2048);
    generate_random_uri(second_level, "generate_folder3", third_level, 2048);
    generate_random_uri(third_level, "generate_dest_file", destination, 2048);

    directories.push_front(first_level);
    directories.push_front(second_level);
    directories.push_front(third_level);

    GError *error = NULL;
    int ret = 0;

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
}


TEST_F(CopyTestMkdir, CopyAtRoot)
{
    generate_random_uri(destination_root, "simple_file_at_root", destination, 2048);

    GError *error = NULL;
    int ret = 0;

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing source and destination base urls\n");
        printf("\t%s [options] srm://host/base/path/ srm://destination/base/path/\n", argv[0]);
        return 1;
    }

    CopyTestMkdir::source_root = argv[1];
    CopyTestMkdir::destination_root = argv[2];

//    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);

    return RUN_ALL_TESTS();
}
