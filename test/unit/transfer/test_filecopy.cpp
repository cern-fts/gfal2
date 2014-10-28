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

#include <gfal_api.h>
#include <gtest/gtest.h>
#include "test_filecopy.h"


void test_filecopy_instance_c(){
	setenv("GFAL_PLUGIN_LIST", "libgfal_plugin_gridftp.so", TRUE);
	GError* tmp_err= NULL;
	gfal2_context_t c = gfal2_context_new(&tmp_err);
    ASSERT_TRUE(c != NULL && tmp_err==NULL);
	gfal2_context_free(c);
}


void test_filecopy_instance_c_problem(){
	setenv("GFAL_PLUGIN_DIR", "/tmp/blablafalse", TRUE);
	GError* tmp_err= NULL;
	gfal2_context_t c = gfal2_context_new(&tmp_err);
    ASSERT_TRUE(c == NULL && tmp_err!=NULL);
	printf("%d %s\n", tmp_err->code, tmp_err->message);
}
