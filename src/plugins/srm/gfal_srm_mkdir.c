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
 * @file gfal_srm_mkdir.c
 * @brief file for the mkdir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 19/05/2011
 * */

#include "gfal_srm.h"
#include "gfal_srm_namespace.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_endpoint.h"
#include <common/gfal_common_err_helpers.h>

int gfal_mkdir_srmv2_internal(gfal_srmv2_opt* opts, char* endpoint, const char* path, mode_t mode, GError** err){
	struct srm_mkdir_input mkdir_input;
    srm_context_t context;
	int res = -1;
	GError* tmp_err=NULL;
	char errbuf[GFAL_ERRMSG_LEN]={0};

	errno =0;	
    if(  (context =  gfal_srm_ifce_context_setup(opts->handle, endpoint, errbuf, GFAL_ERRMSG_LEN, &tmp_err)) != NULL){
        mkdir_input.dir_name = (char*) path;
        res  = gfal_srm_external_call.srm_mkdir(context, &mkdir_input);

        if(res <0){
            gfal_srm_report_error(errbuf, &tmp_err);
            res = -1;
        }
        gfal_srm_ifce_context_release(context);
    }
    G_RETURN_ERR(res, tmp_err, err);
}

int gfal_srm_mkdir_recG(plugin_handle ch, const char* surl, mode_t mode, GError** err){
	int ret = -1;
	char full_endpoint[GFAL_URL_MAX_LEN];
	GError* tmp_err=NULL;
	enum gfal_srm_proto srm_types;
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;
	
	gfal_log(GFAL_VERBOSE_TRACE, "  ->  [gfal_srm_mkdir_rec] ");
	ret =gfal_srm_determine_endpoint(opts, surl, full_endpoint, GFAL_URL_MAX_LEN, &srm_types,   &tmp_err);
	if(ret >=0){
		if (srm_types == PROTO_SRMv2){			// check the proto version
            struct stat stat_dir;
            gfal_log(GFAL_VERBOSE_VERBOSE, "   [gfal_srm_mkdir_rec] check if directory %s already exist...", surl);
            if( gfal_srm_statG(ch, surl, &stat_dir, &tmp_err) == 0 && S_ISDIR(stat_dir.st_mode) ){
                ret =0;
            }else{
                g_clear_error(&tmp_err);
                gfal_log(GFAL_VERBOSE_VERBOSE, "   [gfal_srm_mkdir_rec] try to create directory %s", surl);
                if( (ret= gfal_mkdir_srmv2_internal(opts, full_endpoint, (char*)surl, mode, &tmp_err)) !=0){
                    if(tmp_err && tmp_err->code == EEXIST){
                        g_clear_error(&tmp_err);
                        ret = 0;
                    }
                }
            }
	
		} else if(srm_types == PROTO_SRM){
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT, __func__,
                    "support for SRMv1 is removed in 2.0, failure");
			ret =  -1;
		} else{
		    gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT, __func__,
		            "unknow version of the protocol SRM , failure ");
			ret=-1;
		}

		gfal_log(GFAL_VERBOSE_TRACE, "   [gfal_srm_mkdir_rec] <-");
	}
	G_RETURN_ERR(ret, tmp_err, err);
}


int gfal_srm_mkdirG(plugin_handle ch, const char* surl, mode_t mode, gboolean pflag, GError** err){
	int ret = -1;
	char full_endpoint[GFAL_URL_MAX_LEN];
	GError* tmp_err=NULL;
	enum gfal_srm_proto srm_types;
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    if(pflag){ // pflag set : behavior similar to mkdir -p requested
        ret = gfal_srm_mkdir_recG(ch, surl, mode, &tmp_err);
    }else{
	
        gfal_log(GFAL_VERBOSE_TRACE, "  ->  [gfal_srm_mkdirG] ");
        ret =gfal_srm_determine_endpoint(opts, surl, full_endpoint, GFAL_URL_MAX_LEN, &srm_types,   &tmp_err);
        if(ret >=0){
            if (srm_types == PROTO_SRMv2){			// check the proto version
                gfal_log(GFAL_VERBOSE_VERBOSE, "   [gfal_srm_mkdirG] try to create directory %s", surl);
                // verify if directory already exist
                struct stat st;
                if( gfal_statG_srmv2_internal(opts, &st, full_endpoint, (char*) surl, &tmp_err) !=0){
                    g_clear_error(&tmp_err);
                    ret= gfal_mkdir_srmv2_internal(opts, full_endpoint, (char*)surl, mode, &tmp_err);	// execute the SRMv2 access test
                }else{
                    gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EEXIST, __func__, "directory already exist");
                    ret = -1;
                }



            } else if(srm_types == PROTO_SRM){
                gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT, __func__,
                        "support for SRMv1 is removed in 2.0, failure");
                ret =  -1;
            } else{
                gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(),EPROTONOSUPPORT, __func__,
                        "unknow version of the protocol SRM , failure ");
                ret=-1;
            }

        gfal_log(GFAL_VERBOSE_TRACE, "   [gfal_srm_mkdirG] <-");
        }
    }
	G_RETURN_ERR(ret, tmp_err, err);
}
