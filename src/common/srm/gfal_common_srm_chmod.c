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
 * @file gfal_common_srm_chmod.c
 * @brief file for the change permission management
 * @author Devresse Adrien
 * @date 06/07/2011
 * */
 
 #define _GNU_SOURCE 

#include <regex.h>
#include <time.h> 
 
#include "gfal_common_srm.h"
#include <common/gfal_common_internal.h>
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_plugin.h>
#include "gfal_common_srm_internal_layer.h"
#include "gfal_common_srm_chmod.h"
#include "gfal_common_srm_endpoint.h"
#include "gfal_common_srm_internal_ls.h"

/*
 * 
 *  convert a mode_t to a TPermissionMode, right dec and mask are used to get the good oct (mode & mask) >> right_dec
 *  WARNING : hard conversion mode, subject to problem if the TPermissionMode declaration begin to change !
 */
static TPermissionMode gfal_srmv2_mode_t_to_TPermissionMode(mode_t mode, mode_t mask,  mode_t right_dec){
	return ( (mode & mask) >> right_dec);
}

/*
 * Do a translation of a chmod right to a srm right 
 * */
static void gfal_srmv2_configure_set_permission(gfal_srmv2_opt* opts, const char* surl,  mode_t mode, struct srm_setpermission_input* perms_input){
	memset(perms_input, 0, sizeof(struct srm_setpermission_input));
	perms_input->surl = (char*)surl;
	perms_input->permission_type =SRM_PERMISSION_CHANGE;
	perms_input->owner_permission = gfal_srmv2_mode_t_to_TPermissionMode(mode, 00700, 6);
	perms_input->other_permission= gfal_srmv2_mode_t_to_TPermissionMode(mode, 007, 0);
}

static int gfal_srmv2_chmod_internal(gfal_srmv2_opt* opts, char* endpoint, const char* path, mode_t mode, GError** err){
	g_return_val_err_if_fail(opts && endpoint && path,-1,err,"[gfal_srmv2_chmod_internal] invalid args ");
			
	GError* tmp_err=NULL;
	struct srm_context context;
	int ret=0;
	struct srm_setpermission_input perms_input;
	const int err_size = 2048;
	
	char errbuf[err_size] ; *errbuf='\0';
		
	// set the structures datafields	
	gfal_srmv2_configure_set_permission(opts, path, mode, &perms_input);

    gfal_srm_ifce_context_init(&context, opts->handle, endpoint,
                                  errbuf, GFAL_ERRMSG_LEN, &tmp_err);
	

    if( (ret = gfal_srm_external_call.srm_setpermission(&context , &perms_input)) < 0){
		gfal_srm_report_error(errbuf, &tmp_err);
	} else{
    	 ret = 0;
	}
    G_RETURN_ERR(ret, tmp_err, err);
}

int	gfal_srm_chmodG(plugin_handle ch, const char * path , mode_t mode, GError** err){
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;
	GError* tmp_err=NULL;
	int ret=-1;	
	char full_endpoint[GFAL_URL_MAX_LEN];
	enum gfal_srm_proto srm_types;

    gfal_srm_cache_stat_remove(ch, path);
	if((gfal_srm_determine_endpoint(opts, path, full_endpoint, GFAL_URL_MAX_LEN, &srm_types, &tmp_err)) == 0){		// check & get endpoint										
		gfal_log(GFAL_VERBOSE_NORMAL, "[gfal_srm_chmodG] endpoint %s", full_endpoint);

		if (srm_types == PROTO_SRMv2){
			ret = gfal_srmv2_chmod_internal(opts, full_endpoint, path, mode, &tmp_err);
		} else if(srm_types == PROTO_SRM){
			g_set_error(&tmp_err,0, EPROTONOSUPPORT, "support for SRMv1 is removed in gfal 2.0, failure");
		} else{
			g_set_error(&tmp_err,0,EPROTONOSUPPORT, "Unknow SRM protocol, failure ");
		}		
	}

	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	else 
		errno =0;
	return ret;			
}
