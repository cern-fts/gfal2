#include <gfal_api.h>
#include <gtest/gtest.h>
#include <common/gfal_gtest_asserts.h>
#include <utils/exceptions/gerror_to_cpp.h>


class ConfigFixture: public testing::Test {
protected:
    gfal2_context_t context;

public:
    ConfigFixture() {
        GError* error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    ~ConfigFixture() {
        gfal2_context_free(context);
    }
};

// Regression test for DMC-833
TEST_F(ConfigFixture, ClientData)
{
    GError* error = NULL;
    int ret = 0;

    ret = gfal2_add_client_info(context, "TEST", "VALUE", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_add_client_info(context, "TEST2", "VALUE2", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_add_client_info(context, "TEST", "REPLACED", &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfal2_get_client_info_count(context, &error);
    EXPECT_EQ(ret, 2);

    const char *value;
    ret = gfal2_get_client_info_value(context, "TEST", &value, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    EXPECT_STRCASEEQ(value, "REPLACED");
}
