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


/*
 * @file gfal_common_srm_copy.c
 * @brief file for the third party transfer implementation
 * @author Devresse Adrien
 * */

#include <common/gfal_types.h>
#include <common/gfal_common_errverbose.h>
#include <transfer/gfal_transfer.h>

#include "gfal_common_srm_getput.h"


int srm_plugin_get_3rdparty(plugin_handle handle, const char * surl, char* buff, size_t s_buff, GError ** err){
	return gfal_srm_get_rd3_turl(handle, surl, buff , s_buff, NULL,  err);
}

int srm_plugin_put_3rdparty(plugin_handle handle, const char * surl, char* buff, size_t s_buff, char** reqtoken, GError ** err){
	return gfal_srm_put_rd3_turl(handle, surl, buff , s_buff, reqtoken,  err);
}



int plugin_filecopy(plugin_handle handle, gfal_context_t context,
					gfalt_params_t params, 
					const char* src, const char* dst, GError ** err){
	g_return_val_err_if_fail( handle != NULL && src != NULL
			&& dst != NULL , -1, err, "[plugin_filecopy][gridftp] einval params");	
	
	gfal_log(GFAL_VERBOSE_TRACE, "  -> [srm_plugin_filecopy] ");
	GError * tmp_err=NULL;
	int res = -1;
	char buff_turl_src[GFAL_URL_MAX_LEN];
	char buff_turl_dst[GFAL_URL_MAX_LEN];
	char* reqtoken = NULL;
	
	if( (res = srm_plugin_get_3rdparty(handle, src, buff_turl_src, GFAL_URL_MAX_LEN, &tmp_err)) ==0 ){ // do the first resolution
		gfal_log(GFAL_VERBOSE_TRACE, "    surl -> turl src resolution : %s -> %s", src, buff_turl_src);	
		
		if( ( res = srm_plugin_put_3rdparty(handle, dst, buff_turl_dst, GFAL_URL_MAX_LEN, &reqtoken, &tmp_err) ) ==0){
			gfal_log(GFAL_VERBOSE_TRACE, "    surl -> turl dst resolution : %s -> %s", dst, buff_turl_dst);			
					
			res = gfalt_copy_file(context, params, buff_turl_src, buff_turl_dst, &tmp_err);
			if( res == 0){
				gfal_log(GFAL_VERBOSE_TRACE, "  transfer executed, execute srm put done");				
				res= gfal_srm_putdone_simple(handle, dst, reqtoken, &tmp_err);
			}
		}
	}

	gfal_log(GFAL_VERBOSE_TRACE, " [srm_plugin_filecopy] <-");	
	G_RETURN_ERR(res, tmp_err, err);		
}

