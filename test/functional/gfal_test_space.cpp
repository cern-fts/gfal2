#include <gfal_api.h>
#include <gtest/gtest.h>
#include <utils/exceptions/gerror_to_cpp.h>
#include <common/gfal_gtest_asserts.h>


class SpaceTest: public testing::Test {
public:
    static const char* root;
    gfal2_context_t context;

    SpaceTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~SpaceTest() {
        gfal2_context_free(context);
    }
};

const char* SpaceTest::root;


TEST_F(SpaceTest, SpaceInAttrList)
{
    GError *error = NULL;
    char attrs[1024];

    int ret = gfal2_listxattr(context, root, attrs, sizeof(attrs), &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    for (int i = 0; i < ret;) {
        GTEST_LOG_(INFO) << (attrs + i);
        size_t attrlen = strlen(attrs + i);
        if (strcmp(attrs + i, GFAL_XATTR_SPACETOKEN) == 0) {
            SUCCEED();
            return;
        }
        i += attrlen + 1;
    }
    // Raise an error if we reach here
    FAIL();
}


TEST_F(SpaceTest, GetSpaceAttr)
{
    GError *error = NULL;
    char buffer[1024];

    int ret = gfal2_getxattr(context, root, GFAL_XATTR_SPACETOKEN, buffer, sizeof(buffer), &error);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    GTEST_LOG_(INFO) << buffer;
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing base url\n");
        printf("\t%s srm://host/base/path/\n", argv[0]);
        return 1;
    }

    SpaceTest::root = argv[1];

    return RUN_ALL_TESTS();
}
