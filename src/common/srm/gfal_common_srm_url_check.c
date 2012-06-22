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
#define _GNU_SOURCE
 
#include <string.h>

#include "gfal_common_srm_url_check.h"

#include <common/gfal_types.h>
#include <common/gfal_common_errverbose.h>
#include <transfer/gfal_transfer.h>

const char * surl_prefix = GFAL_PREFIX_SRM;


gboolean srm_check_url(const char * surl){
	gboolean res =FALSE;
	const size_t prefix_len = strlen(surl_prefix) ;
	size_t surl_len = strnlen(surl, GFAL_URL_MAX_LEN);
	if( ( surl_len < GFAL_URL_MAX_LEN )
			&& (strncmp(surl, surl_prefix, prefix_len) == 0)){
		res = TRUE;
	}
	return res;
}


gboolean plugin_url_check2(plugin_handle handle, const char* src, const char* dst, gfal_url2_check type ){
	g_return_val_if_fail(handle != NULL,FALSE);
	gboolean res = FALSE;

	
	if( src != NULL && dst != NULL){
		if( type == GFAL_FILE_COPY 
			&& (srm_check_url(src)
				||  srm_check_url(dst)
				)
			){
				res= TRUE;
			}
		
	}
	return res;
}
