#include <gtest/gtest.h>
#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>
#include <utils/exceptions/gerror_to_cpp.h>


class AccessTest: public testing::Test {
public:
    static const char* root;

    char surl[2048];
    gfal2_context_t context;

    AccessTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~AccessTest() {
        gfal2_context_free(context);
    }

    virtual void SetUp() {
        GError* error = NULL;
        generate_random_uri(root, "access_test", surl, sizeof(surl));
        int ret = gfal2_mkdir(context, surl, 0775, &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }

    virtual void TearDown() {
        GError* error = NULL;
        gfal2_rmdir(context, surl, &error);
        g_clear_error(&error);
    }
};
const char* AccessTest::root;


TEST_F(AccessTest, SimpleAccess)
{

}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing base url and/or modes\n");
        printf("\t%s [options] srm://host/base/path/\n", argv[0]);
        return 1;
    }

    AccessTest::root = argv[1];

//    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);

    return RUN_ALL_TESTS();
}
