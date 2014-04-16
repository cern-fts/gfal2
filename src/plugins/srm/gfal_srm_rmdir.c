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
 * @file gfal_srm_rmdir.c
 * @brief file for the rmdir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 24/05/2011
 * */
#include "gfal_srm.h"
#include "gfal_srm_namespace.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_endpoint.h"
#include "gfal_srm_internal_ls.h"

#include <common/gfal_constants.h>
#include <common/gfal_common_err_helpers.h>


int gfal_srmv2_rmdir_internal(gfal_srmv2_opt* opts, char* endpoint, const char* surl, GError** err){
    srm_context_t context;
	struct srm_rmdir_input rmdir_input;
	struct srm_rmdir_output rmdir_output;
	GError* tmp_err=NULL;
	char errbuf[GFAL_ERRMSG_LEN]={0};
	int ret = -1;
	

    if( (context =  gfal_srm_ifce_context_setup(opts->handle, endpoint, errbuf, GFAL_ERRMSG_LEN, &tmp_err)) != NULL){
        rmdir_input.recursive = 0;
        rmdir_input.surl = (char*)surl;

        if( gfal_srm_external_call.srm_rmdir(context, &rmdir_input, &rmdir_output) >=0){
            const int sav_errno = rmdir_output.statuses[0].status;
            if( sav_errno ){
                gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), sav_errno, __func__,
                        "Error report from the srm_ifce %s ", strerror(sav_errno));
                ret = -1;
            }else{
                ret =0;
            }
            gfal_srm_external_call.srm_srmv2_filestatus_delete(rmdir_output.statuses,1);
            gfal_srm_external_call.srm_srm2__TReturnStatus_delete (rmdir_output.retstatus);
        }else{
            gfal_srm_report_error(errbuf, &tmp_err);
            ret=-1;
        }
        gfal_srm_ifce_context_release(context);
    }
    G_RETURN_ERR(ret, tmp_err, err);
}


int gfal_srm_rmdirG(plugin_handle ch, const char* surl, GError** err){
	int ret = -1;
	char full_endpoint[GFAL_URL_MAX_LEN];
	GError* tmp_err=NULL;
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*)ch;
	enum gfal_srm_proto srm_type;
	gfal_log(GFAL_VERBOSE_VERBOSE, "  -> [gfal_srm_rmdirG]");	
		
    gfal_srm_cache_stat_remove(ch, surl);
	ret = gfal_srm_determine_endpoint(opts, surl, full_endpoint, GFAL_URL_MAX_LEN, &srm_type,  &tmp_err);
	if( ret >=0 ){
		if(srm_type == PROTO_SRMv2){
			struct stat st;
			gfal_log(GFAL_VERBOSE_VERBOSE, "   [gfal_srm_rmdirG] try to delete directory %s", surl);			
			if( (ret = gfal_statG_srmv2_internal(opts, &st, full_endpoint, (char*) surl, &tmp_err)) ==0){ // stat file in order to verify if directory or not
				
				if( S_ISDIR(st.st_mode) ){
					ret = gfal_srmv2_rmdir_internal(opts, full_endpoint, surl, &tmp_err);
				}else{
					ret = -1;
                    gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), ENOTDIR, __func__,
                            "This file is not a directory, impossible to use rmdir on it");
				}

			
			}				

		}else if (srm_type == PROTO_SRM){
		    gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT, __func__,
		            "support for SRMv1 is removed in 2.0, failure");
			ret = -1;
		}else {
		    gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT, __func__,
		            "unknow version of the protocol SRM , failure");
			ret = -1;			
		}
		
	}
	gfal_log(GFAL_VERBOSE_VERBOSE, "  [gfal_srm_rmdirG] <-");	
	
    G_RETURN_ERR(ret, tmp_err, err);
}




