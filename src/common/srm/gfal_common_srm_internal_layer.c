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
 * @file gfal_common_srm_internal_layer.c
 * @brief file for the srm external function mapping for mocking purpose
 * @author Devresse Adrien
 * @version 2.0
 * @date 09/06/2011
 * */

const char * srm_config_group= "SRM PLUGIN";
const char * srm_config_transfer_checksum= "COPY_CHECKSUM_TYPE";
const char * srm_ops_timeout_key= "OPERATION_TIMEOUT";
const char * srm_conn_timeout_key= "CONN_TIMEOUT";

#include "gfal_common_srm_internal_layer.h"
// hotfix for the old srm lib 
void disable_srm_srmv2_pinfilestatus_delete(struct srmv2_pinfilestatus*  srmv2_pinstatuses, int n){}
void disable_srm_srmv2_mdfilestatus_delete(struct srmv2_mdfilestatus* mdfilestatus, int n){}
void disable_srm_srmv2_filestatus_delete(struct srmv2_filestatus*  srmv2_statuses, int n){}
void disable_srm_srm2__TReturnStatus_delete(struct srm2__TReturnStatus* status){}


struct _gfal_srm_external_call gfal_srm_external_call = { 
	.srm_context_init = &srm_context_init,
	.srm_ls = &srm_ls,
	.srm_rmdir = &srm_rmdir,
	.srm_mkdir = &srm_mkdir,
	.srm_getpermission = &srm_getpermission,
	.srm_check_permission = &srm_check_permission,
	.srm_srmv2_pinfilestatus_delete = &disable_srm_srmv2_pinfilestatus_delete,
	.srm_srmv2_mdfilestatus_delete = &disable_srm_srmv2_mdfilestatus_delete,
	.srm_srmv2_filestatus_delete = &disable_srm_srmv2_filestatus_delete,
	.srm_srm2__TReturnStatus_delete = &disable_srm_srm2__TReturnStatus_delete,
	.srm_prepare_to_get= &srm_prepare_to_get,
	.srm_prepare_to_put= &srm_prepare_to_put,
	.srm_put_done = &srm_put_done,
	.srm_setpermission= &srm_setpermission,
    .srm_rm = &srm_rm,
    .srm_set_timeout_connect = &srm_set_timeout_connect
};


int gfal_srm_ifce_context_init(struct srm_context* context, gfal_context_t handle, const char* endpoint,
                                char* errbuff, size_t s_errbuff, GError** err){
    gint timeout;
    GError * tmp_err=NULL;
    gfal_srm_external_call.srm_context_init(context, (char*) endpoint, errbuff, s_errbuff, gfal_get_verbose());
    timeout = gfal2_get_opt_integer(handle, srm_config_group, srm_ops_timeout_key, &tmp_err);
    if(!tmp_err){
        gfal_log(GFAL_VERBOSE_DEBUG, " SRM operation timeout %d", timeout);
        context->timeout = timeout;

        timeout = gfal2_get_opt_integer(handle, srm_config_group, srm_conn_timeout_key, &tmp_err);
        if(!tmp_err){
            gfal_log(GFAL_VERBOSE_DEBUG, " SRM connexion timeout %d", timeout);
            gfal_srm_external_call.srm_set_timeout_connect(timeout);
        }
    }
    if(!tmp_err)
        return 0;
    G_RETURN_ERR(-1, tmp_err, err);
}


int gfal_srm_ifce_context_deinit(struct srm_context* context){
    return 0;
}

