/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "tests_srm.h"
#include <cgreen/cgreen.h>
#include <glib.h>
#include <common/gfal_types.h>
#include <common/gfal_common_internal.h>
#include <common/srm/gfal_common_srm_url_check.h>
#include <common/srm/gfal_common_srm.h>






void srm_url_check(){
	GError * tmp_err=NULL;
	gfal_handle handle = gfal_initG(&tmp_err);
	assert_true_with_message(tmp_err== NULL && handle ," initialize gfal failed ");
	
	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
	gboolean res = plugin_url_check2(&opts, NULL, NULL, GFAL_FILE_COPY);
	assert_true_with_message(res == FALSE ," bad url");
	res = plugin_url_check2(&opts, "srm://blabla.com/tata", "srm://blabla.com/toto", GFAL_FILE_COPY);
	assert_true_with_message(res == TRUE ,"good url");
	res = plugin_url_check2(&opts, NULL , "srm://blabla.com/toto", GFAL_FILE_COPY);
	assert_true_with_message(res == FALSE ," one bad url");
	res = plugin_url_check2(&opts, "fsdfds", "srm://blabla.com/toto", GFAL_FILE_COPY);
	assert_true_with_message(res == FALSE ,"bad protocol format");
	res = plugin_url_check2(&opts, "srm://blabla.com/toto", "dsffds", GFAL_FILE_COPY);
	assert_true_with_message(res == FALSE ,"bad protocol format 2");
	res = plugin_url_check2(&opts, "sr", "", GFAL_FILE_COPY);
	assert_true_with_message(res == FALSE ,"short url test");	
	gfal_handle_freeG(handle);
}


