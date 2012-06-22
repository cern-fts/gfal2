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


#include "tests_params.h"
#include <cstdlib>

#include <transfer/gfal_transfer_types_internal.h>
#include <transfer/gfal_transfer_plugins.h>
#include <common/gfal_types.h>
#include <glibmm.h>
#include <cgreen/cgreen.h>	
#include <glib.h>


using namespace Gfal::Transfer;


int get_locked_errcode(){
	return EBUSY;
}

void create_params(){
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


void test_timeout_c(){
	GError * tmp_err=NULL;
	gfalt_params_t p = gfalt_params_handle_new(&tmp_err);
	assert_true_with_message( p != NULL && tmp_err==NULL, "bad initialization ");
	long res = gfalt_get_timeout(p, &tmp_err);
	assert_true_with_message( res == GFALT_DEFAULT_TRANSFERT_TIMEOUT && tmp_err==NULL, "bad timeout value %ld %ld ", res, tmp_err);
	guint64  r = (guint64) rand();
	gfalt_set_timeout(p, r, &tmp_err);
	assert_true_with_message( gfalt_get_timeout(p, &tmp_err)  == r, "bad timeout get ");	
	gfalt_params_handle_delete(p,NULL);
}


void test_nbstreams_c(){
	GError * tmp_err=NULL;
	gfalt_params_t p = gfalt_params_handle_new(&tmp_err);
	assert_true_with_message( p != NULL && tmp_err==NULL, "bad initialization ");
	long res = gfalt_get_nbstreams(p, &tmp_err);
	assert_true_with_message( res == GFALT_DEFAULT_NB_STREAM && tmp_err==NULL, "bad nbstreams value %ld %ld ", res, tmp_err);
	long  r = rand();
	gfalt_set_nbstreams(p, r, &tmp_err);
	assert_true_with_message( gfalt_get_nbstreams(p, &tmp_err)  == r, "bad timeout get ");	
	gfalt_params_handle_delete(p,NULL);
}




