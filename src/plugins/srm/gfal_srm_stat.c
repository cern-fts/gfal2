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
 * @file gfal_srm_stat.c
 * @brief file for the stat function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 16/05/2011
 * */
#include <common/gfal_constants.h>
#include <common/gfal_common_errverbose.h> 

#include "gfal_srm.h"
#include "gfal_srm_internal_ls.h"
#include "gfal_srm_access.h"
#include "gfal_srm_internal_layer.h" 
#include "gfal_srm_endpoint.h"



int gfal_statG_srmv2_internal(gfal_srmv2_opt* opts, struct stat* buf, const char* endpoint, const char* surl, GError** err){
	return gfal_statG_srmv2__generic_internal(opts, buf, endpoint, surl, err);
}

/*
 * stat call, for the srm interface stat and lstat are the same call !! the default behavior is similar to stat by default and ignore links
 * 
 * */
int gfal_srm_statG(plugin_handle ch, const char* surl, struct stat* buf, GError** err){
	g_return_val_err_if_fail( ch && surl && buf, -1, err, "[gfal_srm_statG] Invalid args in handle/surl/bugg");
	GError* tmp_err = NULL;
	int ret =-1;
	char full_endpoint[GFAL_URL_MAX_LEN];
	char key_buff[GFAL_URL_MAX_LEN];
	enum gfal_srm_proto srm_type;
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;
	gfal_srm_construct_key(surl, GFAL_SRM_LSTAT_PREFIX, key_buff, GFAL_URL_MAX_LEN);
	
	if (gsimplecache_take_one_kstr(opts->cache, key_buff, buf) ==0){
		gfal_log(GFAL_VERBOSE_DEBUG, " srm_statG -> value taken from the cache");
		ret = 0;
	}else{
		ret =gfal_srm_determine_endpoint(opts, surl, full_endpoint, GFAL_URL_MAX_LEN, &srm_type,   &tmp_err);
		if( ret >=0 ){
			if(srm_type == PROTO_SRMv2){
				gfal_log(GFAL_VERBOSE_VERBOSE, "   [gfal_srm_statG] try to stat file %s", surl);				
				ret = gfal_statG_srmv2_internal(opts, buf, full_endpoint, surl, &tmp_err);
                if( ret ==0){
                   gfal_log(GFAL_VERBOSE_TRACE, "   [gfal_srm_statG] store %s stat info in cache", surl);
                   gfal_srm_cache_stat_add(ch, surl, buf);
                }
			}else if (srm_type == PROTO_SRM){
				g_set_error(&tmp_err, 0, EPROTONOSUPPORT, "support for SRMv1 is removed in 2.0, failure");
				ret = -1;
			}else {
				g_set_error(&tmp_err, 0, EPROTONOSUPPORT, "Unknow version of the protocol SRM , failure");
				ret = -1;			
			}
			
		}
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}
