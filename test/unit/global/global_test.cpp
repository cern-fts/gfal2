



#include <gfal_api.h>
#include <utils/uri_util.h>
#include <gtest/gtest.h>


TEST(gfalGlobal, test_verbose){
    gfal_set_verbose(GFAL_VERBOSE_DEBUG);
    int r = gfal_get_verbose();
    ASSERT_TRUE(r==GFAL_VERBOSE_DEBUG);
}

TEST(gfalGlobal, testLoad){
    GError* tmp_err=NULL;
    gfal2_context_t c = gfal2_context_new(&tmp_err);
    if (tmp_err)
        printf("%s\n", tmp_err->message);
    ASSERT_EQ((GError*)NULL, tmp_err);
    ASSERT_TRUE(c != NULL);
    gfal2_context_free(c);
    c = NULL;
    gfal2_context_free(c);
}


TEST(gfalGlobal, url_parsing){
    GError* tmp_err=NULL;
    const char* url = "gsiftp://dcache-door-desy09.desy.de:2811/pnfs/desy.de/dteam/gfal2-tests/testread0011";
    const char* bad_url= "bob the sponge:";
    char buffer[GFAL_URL_MAX_LEN]= {0};
    int ret = gfal_hostname_from_uri(url, buffer, GFAL_URL_MAX_LEN, &tmp_err);
    ASSERT_EQ(0, ret);
    ASSERT_STREQ("dcache-door-desy09.desy.de:2811", buffer);

    ret = gfal_hostname_from_uri(bad_url, buffer, GFAL_URL_MAX_LEN, &tmp_err);
    ASSERT_LT(0,ret);
    ASSERT_TRUE(tmp_err != NULL);

}



