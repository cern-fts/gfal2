



#include <gfal_api.h>
#include <gtest/gtest.h>


TEST(gfalGlobal, test_verbose){
    gfal_set_verbose(GFAL_VERBOSE_DEBUG);
    int r = gfal_get_verbose();
    ASSERT_TRUE(r==GFAL_VERBOSE_DEBUG);
}



