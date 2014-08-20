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

/*
 * @file gfal_posix_readdir.c
 * @brief file for the readdir posix func
 * @author Devresse Adrien
 * @version 2.0
 * @date 27/05/2011
 * */

#include <stdlib.h>
#include <glib.h>
#include <dirent.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>


#include "gfal_posix_api.h"
#include "gfal_posix_internal.h"
#include <common/gfal_common_filedescriptor.h>
#include <common/gfal_common_dir_handle.h>


/*
 * Implementation of the readdir functions
 *
 */
inline struct dirent* gfal_posix_internal_readdir(DIR* dir){
	 GError* tmp_err=NULL;
	 gfal2_context_t handle;
	 struct dirent* res= NULL;

	if((handle = gfal_posix_instance()) == NULL){
		errno = EIO;
		return NULL;
	}

    res = gfal2_readdir(handle, dir, &tmp_err);
	if(tmp_err){
		gfal_posix_register_internal_error(handle, "[gfal_readdir]", tmp_err);
		errno = tmp_err->code;
	}
	return res;
}


