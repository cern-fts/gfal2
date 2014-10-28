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
  @file gfal_common_lfc_open.c
  @brief lfc implementation for open/read/write/close
  @author Adrien Devresse
  @date 29/04/2011
 */

#include <regex.h>

#include "gfal_lfc.h"
#include "lfc_ifce_ng.h"


/*
 * open function for the srm  plugin
 */
gfal_file_handle lfc_openG(plugin_handle ch, const char* path, int flag, mode_t mode, GError** err){
	gfal2_context_t handle = ((struct lfc_ops*)ch)->handle;
	GError* tmp_err=NULL;
	gfal_file_handle res=NULL;
	gfal_log(GFAL_VERBOSE_TRACE, "  %s ->",__func__);

	char** surls = lfc_getSURLG(ch, path, &tmp_err);
	if(surls != 0 && tmp_err == NULL){
		char** p = surls;
		while( *p != NULL){
			gfal_log(GFAL_VERBOSE_VERBOSE, " LFC resolution %s -> %s ", path, *p);
			res = gfal_plugin_openG(handle, *p, flag, mode, &tmp_err);
			if(res || ( tmp_err && tmp_err->code!=ECOMM))
				break;
			p++;
		}
	}
	g_strfreev(surls);
	if(tmp_err)
		gfal2_propagate_prefixed_error(err, tmp_err, __func__);
	return res;

}
