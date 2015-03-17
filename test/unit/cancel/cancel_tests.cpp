#include <gfal_api.h>
#include <gtest/gtest.h>


TEST(gfalCancel, test_cancel_simple){
    GError* tmp_err=NULL;
    gfal2_context_t c = gfal2_context_new(&tmp_err);
    ASSERT_TRUE(c != NULL);
    gfal2_cancel(c);
    gfal2_cancel(c);
    gfal2_cancel(c);
    gfal2_context_free(c);
}

void gfal_cancel_hook_cb_s(gfal2_context_t context, void* userdata){
    int* p = (int*)userdata;
    *p += 1;
    (void) gfal2_version();

}

TEST(gfalGlobal, testCancelCallback){
    GError* tmp_err=NULL;
    gfal2_context_t c = gfal2_context_new(&tmp_err);
    ASSERT_TRUE(c != NULL);

    int i=0,res ;
    // add callback7
    gfal_cancel_token_t tok = gfal2_register_cancel_callback(c, &gfal_cancel_hook_cb_s, &i);
    ASSERT_TRUE( tok != NULL);
    res= gfal2_cancel(c);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(i, 1);
    gfal2_remove_cancel_callback(c, tok);
    res= gfal2_cancel(c);
    ASSERT_EQ(res, 0);
    ASSERT_EQ(i, 1);

    res= gfal2_cancel(c);
    ASSERT_EQ(res, 0);
    gfal2_context_free(c);
}


