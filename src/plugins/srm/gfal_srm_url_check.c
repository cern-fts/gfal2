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

 
#include <string.h>

#include "gfal_srm_url_check.h"

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

gboolean srm_check_url_transport_compatible(plugin_handle handle, const char* url){
        gfal_srmv2_opt* opts = (gfal_srmv2_opt*) handle;
        char** p_proto = srm_get_3rdparty_turls_sup_protocol(opts->handle);
        while(*p_proto != NULL){
            const int proto_len = strlen(*p_proto);
            if(strncmp(url, *p_proto, proto_len) == 0)
                return TRUE;
            ++p_proto;
        }
        return FALSE;
}


gboolean plugin_url_check2(plugin_handle handle, const char* src, const char* dst, gfal_url2_check type ){
    g_return_val_if_fail(handle != NULL && src != NULL && dst != NULL,FALSE);
	gboolean res = FALSE;
    gboolean src_srm = srm_check_url(src);
    gboolean dst_srm = srm_check_url(dst);
    gboolean src_3rd = srm_check_url_transport_compatible(handle, src);
    gboolean dst_3rd = srm_check_url_transport_compatible(handle, dst);

	if( src != NULL && dst != NULL){
		if( type == GFAL_FILE_COPY 
            && (    ( src_srm && dst_srm)
                 || ( src_srm && dst_3rd)
                 || ( dst_srm && src_3rd)
               )
			){
				res= TRUE;
			}
		
	}
	return res;
}
