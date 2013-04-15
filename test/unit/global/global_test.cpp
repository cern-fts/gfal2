



#include <gfal_api.h>
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


