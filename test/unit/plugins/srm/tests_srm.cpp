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


#include "tests_srm.h"
#include <gtest/gtest.h>
#include <gfal_api.h>

extern "C"{

#include <plugins/srm/gfal_srm_url_check.h>
#include <plugins/srm/gfal_srm.h>

}

TEST(gfalPlugin, srmTest){
	GError * tmp_err=NULL;
	gfal2_context_t handle = gfal2_context_new(&tmp_err);
    ASSERT_TRUE(tmp_err== NULL && handle);

	gfal_srmv2_opt opts;
	gfal_srm_opt_initG(&opts, handle);
    gboolean res = plugin_url_check2(&opts, handle, NULL, NULL, GFAL_FILE_COPY);
    ASSERT_TRUE(res == FALSE);
	res = plugin_url_check2(&opts, handle, "srm://blabla.com/tata", "srm://blabla.com/toto", GFAL_FILE_COPY);
    ASSERT_TRUE(res == TRUE);
	res = plugin_url_check2(&opts, handle, NULL , "srm://blabla.com/toto", GFAL_FILE_COPY);
    ASSERT_TRUE(res == FALSE);
	res = plugin_url_check2(&opts, handle, "fsdfds", "srm://blabla.com/toto", GFAL_FILE_COPY);
    ASSERT_TRUE(res == FALSE );
	res = plugin_url_check2(&opts, handle, "srm://blabla.com/toto", "dsffds", GFAL_FILE_COPY);
    ASSERT_TRUE(res == FALSE);
	res = plugin_url_check2(&opts, handle, "sr", "", GFAL_FILE_COPY);
    ASSERT_TRUE(res == FALSE);
    gfal2_context_free(handle);
}
