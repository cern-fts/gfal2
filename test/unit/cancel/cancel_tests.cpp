/*
 * Copyright (c) CERN 2013-2017
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
    if (tmp_err) {
        printf("%d %s\n", tmp_err->code, tmp_err->message);
    }
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


