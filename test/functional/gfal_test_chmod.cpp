#include <gtest/gtest.h>
#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>
#include <utils/exceptions/gerror_to_cpp.h>


class ChmodTest: public testing::Test {
public:
    static const char* root;
    static int n_modes;
    static mode_t *modes;

    char surl[2048];
    gfal2_context_t context;

    ChmodTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~ChmodTest() {
        gfal2_context_free(context);
    }

    virtual void SetUp() {
        GError* error = NULL;
        generate_random_uri(root, "chmod_test", surl, sizeof(surl));
        int ret = gfal2_mkdir(context, surl, 0775, &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }

    virtual void TearDown() {
        GError* error = NULL;
        gfal2_chmod(context, surl, 0777, &error);
        g_clear_error(&error);
        gfal2_rmdir(context, surl, &error);
        g_clear_error(&error);
    }
};
const char* ChmodTest::root;
int ChmodTest::n_modes = 0;
mode_t* ChmodTest::modes = NULL;


TEST_F(ChmodTest, SimpleChmod)
{
    GError* error = NULL;
    struct stat st;

    int ret = gfal2_stat(context, surl, &st, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    mode_t original_mode = st.st_mode;
    int i;
    for (i = 0; i < n_modes; ++i) {
        ret = gfal2_chmod(context, surl, modes[i], &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

        ret = gfal2_stat(context, surl, &st, &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

        EXPECT_EQ(modes[i], st.st_mode & 0777);
    };

    ret = gfal2_chmod(context, surl, original_mode, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 3) {
        printf("Missing base url and/or modes\n");
        printf("\t%s [options] srm://host/base/path/ 0777 0555 0000....\n", argv[0]);
        return 1;
    }

    ChmodTest::root = argv[1];
    ChmodTest::n_modes = argc - 2;
    ChmodTest::modes = new mode_t[ChmodTest::n_modes];
    int i;
    for (i = 2; i < argc; ++i) {
        errno = 0;
        ChmodTest::modes[i - 2] = (mode_t)strtol(argv[i], NULL, 8);
        if (errno) {
            perror ("strtol");
            exit (1);
        }
    }

//    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);

    int result = RUN_ALL_TESTS();
    delete[] ChmodTest::modes;
    return result;
}
