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

#include <glib.h>
#include <cstdlib>
#include <global/gfal_global.h>
#include <common/gfal_common_internal.h>
#include <transfer/gfal_transfer_types_internal.h>


extern "C" {
	
using namespace Gfal;
using namespace Gfal::Transfer;

gfal_context_t gfal_context_new(GError ** err){
	GError * tmp_err=NULL;
	gfal_context_t c = NULL;
	CPP_GERROR_TRY
		c = reinterpret_cast<gfal_context_t>(new FileCopy(new CoreLayer()));	
	CPP_GERROR_CATCH(&tmp_err);
	G_RETURN_ERR(c, tmp_err, err);
} 
	

void gfal_context_free(gfal_context_t context){
	delete (reinterpret_cast<FileCopy*>(context));
}
	
}
