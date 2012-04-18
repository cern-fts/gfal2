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

/* unit test for common_errverbose */


#include <cgreen/cgreen.h>
#include "gfal_common_errverbose.h"


 	

void gfal2_test_verbose_set_get()
{
	gfal_set_verbose(GFAL_VERBOSE_DEBUG);
	int r = gfal_get_verbose();
	assert_true_with_message(r==GFAL_VERBOSE_DEBUG, " verbose set /get failed, must be the same value");
}
