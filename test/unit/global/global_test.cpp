



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
    ASSERT_EQ(tmp_err, (GError*)NULL);
    ASSERT_TRUE(c != NULL);
    gfal2_context_free(c);
    c = NULL;
    gfal2_context_free(c);
}


