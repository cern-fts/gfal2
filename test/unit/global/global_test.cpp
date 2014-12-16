#include <gfal_api.h>
#include <gfal_plugins_api.h>
#include <utils/uri/gfal_uri.h>
#include <gtest/gtest.h>


TEST(gfalGlobal, testVerbose){
    gfal_set_verbose(GFAL_VERBOSE_DEBUG);
    int r = gfal_get_verbose();
    ASSERT_EQ(GFAL_VERBOSE_DEBUG, r);
}


TEST(gfalGlobal, testLoad){
    GError* tmp_err = NULL;
    gfal2_context_t c = gfal2_context_new(&tmp_err);
    if (tmp_err)
        printf("%s\n", tmp_err->message);
    ASSERT_EQ(NULL, tmp_err);
    ASSERT_NE((void*)NULL, c);
    gfal2_context_free(c);
    c = NULL;
    gfal2_context_free(c);
}


TEST(gfalGlobal, urlParsing){
    GError* tmp_err = NULL;
    const char* url = "gsiftp://dcache-door-desy09.desy.de:2811/pnfs/desy.de/dteam/gfal2-tests/testread0011";
    const char* bad_url = "bob the sponge:";
    char buffer[GFAL_URL_MAX_LEN] = { 0 };
    int ret = gfal_hostname_from_uri(url, buffer, GFAL_URL_MAX_LEN, &tmp_err);
    ASSERT_EQ(0, ret);
    ASSERT_STREQ("dcache-door-desy09.desy.de:2811", buffer);

    ret = gfal_hostname_from_uri(bad_url, buffer, GFAL_URL_MAX_LEN, &tmp_err);
    ASSERT_GT(0, ret);
    ASSERT_NE((void*)NULL, tmp_err);
}

static gboolean test_plugin_url(plugin_handle plugin_data, const char* url,
        plugin_mode operation, GError** err)
{
    return strncmp(url, "test://", 7) == 0 && operation == GFAL_PLUGIN_STAT;
}


static int test_plugin_stat(plugin_handle plugin_data , const char* url, struct stat *buf, GError** err)
{
    buf->st_mode = 12345;
    return 0;
}


TEST(gfalGlobal, loadPlugin)
{
    GError* tmp_err = NULL;
    gfal2_context_t c = gfal2_context_new(&tmp_err);
    ASSERT_NE((void*)NULL, c);

    gfal_plugin_interface test_plugin;
    memset(&test_plugin, 0, sizeof(test_plugin));

    test_plugin.check_plugin_url = test_plugin_url;
    test_plugin.statG = test_plugin_stat;

    int ret = gfal2_register_plugin(c, &test_plugin, &tmp_err);
    if (tmp_err)
        printf("%s\n", tmp_err->message);
    ASSERT_EQ(0, ret);

    struct stat st;
    ret = gfal2_stat(c, "test://blah", &st, &tmp_err);
    ASSERT_EQ(0, ret);

    ASSERT_EQ(12345, st.st_mode);

    gfal2_context_free(c);
}
