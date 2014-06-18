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
 * @file gfal_srm_chmod.c
 * @brief file for the change permission management
 * @author Devresse Adrien
 * @date 06/07/2011
 * */
 
  

#include <regex.h>
#include <time.h> 
 
#include "gfal_srm.h"
#include <common/gfal_common_internal.h>
#include <common/gfal_common_err_helpers.h>
#include <common/gfal_common_plugin.h>
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_namespace.h"
#include "gfal_srm_endpoint.h"
#include "gfal_srm_internal_ls.h"

/*
 * 
 *  convert a mode_t to a TPermissionMode, right dec and mask are used to get the good oct (mode & mask) >> right_dec
 *  WARNING : hard conversion mode, subject to problem if the TPermissionMode declaration begin to change !
 */
static TPermissionMode gfal_srmv2_mode_t_to_TPermissionMode(mode_t mode, mode_t mask,  mode_t right_dec)
{
	return ( (mode & mask) >> right_dec);
}

/*
 * Do a translation of a chmod right to a srm right 
 * */
static void gfal_srmv2_configure_set_permission(const char* surl,  mode_t mode, struct srm_setpermission_input* perms_input)
{
	memset(perms_input, 0, sizeof(struct srm_setpermission_input));
	perms_input->surl = (char*)surl;
	perms_input->permission_type =SRM_PERMISSION_CHANGE;
	perms_input->owner_permission = gfal_srmv2_mode_t_to_TPermissionMode(mode, 00700, 6);
	perms_input->other_permission= gfal_srmv2_mode_t_to_TPermissionMode(mode, 007, 0);
}


static int gfal_srmv2_chmod_internal(srm_context_t context, const char* path, mode_t mode, GError** err)
{
	g_return_val_err_if_fail(context && path,-1,err,"[gfal_srmv2_chmod_internal] invalid args ");
			
	GError* tmp_err=NULL;
	int ret=0;
	struct srm_setpermission_input perms_input;
		
	// set the structures datafields	
	gfal_srmv2_configure_set_permission(path, mode, &perms_input);

    if( (ret = gfal_srm_external_call.srm_setpermission(context , &perms_input)) < 0) {
        gfal_srm_report_error(context->errbuf, &tmp_err);
    }
    else {
        ret = 0;
    }

    G_RETURN_ERR(ret, tmp_err, err);
}


int	gfal_srm_chmodG(plugin_handle ch, const char * path , mode_t mode, GError** err)
{
    g_return_val_err_if_fail(ch && path, EINVAL, err, "[gfal_srm_chmodG] Invalid value handle and/or surl");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    int ret = -1;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, path, &tmp_err);
    if (context != NULL) {
        gfal_srm_cache_stat_remove(ch, path);
        ret = gfal_srmv2_chmod_internal(context, path, mode, &tmp_err);
        gfal_srm_ifce_context_release(context);
    }

    if (ret != 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}
