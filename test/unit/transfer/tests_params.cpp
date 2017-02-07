/*
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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

#include <gtest/gtest.h>
#include <cstdlib>
#include <gfal_api.h>


TEST(gfalTransfer, testparam){
	GError * tmp_err=NULL;
	gfalt_params_t  p = gfalt_params_handle_new(&tmp_err);
	g_assert(p != NULL);
	g_assert(tmp_err == NULL);
    gfalt_params_t  p2 = gfalt_params_handle_copy(p, &tmp_err);
    g_assert(p2 != NULL);
    g_assert(tmp_err == NULL);
	gfalt_params_handle_delete(NULL, &tmp_err);
	g_assert(tmp_err == NULL);
	gfalt_params_handle_delete(p,&tmp_err);
    g_assert(tmp_err == NULL);
    gfalt_params_handle_delete(p2,&tmp_err);
    g_assert(tmp_err == NULL);
}


TEST(gfalTransfer, testtimeout){
	GError * tmp_err=NULL;
	gfalt_params_t p = gfalt_params_handle_new(&tmp_err);
    ASSERT_TRUE( p != NULL && tmp_err==NULL);
	long res = gfalt_get_timeout(p, &tmp_err);
    ASSERT_TRUE(tmp_err==NULL);
	guint64  r = (guint64) rand();
	gfalt_set_timeout(p, r, &tmp_err);
    ASSERT_TRUE( gfalt_get_timeout(p, &tmp_err)  == r);
	gfalt_params_handle_delete(p,NULL);
}


TEST(gfalTransfer, testnbstream){
	GError * tmp_err=NULL;
	gfalt_params_t p = gfalt_params_handle_new(&tmp_err);
    ASSERT_TRUE( p != NULL && tmp_err==NULL);
	long res = gfalt_get_nbstreams(p, &tmp_err);
    ASSERT_TRUE(tmp_err==NULL);
	long  r = rand();
	gfalt_set_nbstreams(p, r, &tmp_err);
    ASSERT_TRUE( gfalt_get_nbstreams(p, &tmp_err)  == r);
	gfalt_params_handle_delete(p,NULL);
}

TEST(gfalTransfer, testlocaltransfer){
    GError * tmp_err=NULL;
    gfalt_params_t p = gfalt_params_handle_new(&tmp_err);
    ASSERT_TRUE( p != NULL && tmp_err==NULL);
    gboolean res = gfalt_get_local_transfer_perm(p, &tmp_err);
    ASSERT_TRUE( res == TRUE && tmp_err==NULL);

    int ret = gfalt_set_local_transfer_perm(p, FALSE, &tmp_err);
    res = gfalt_get_local_transfer_perm(p, &tmp_err);
    ASSERT_TRUE( res == FALSE && ret == FALSE && tmp_err==NULL);
    gfalt_params_handle_delete(p,NULL);
}
