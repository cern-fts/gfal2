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
 * @file gfal_srm_access.c
 * @brief file for the access function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 05/05/2011
 * */
#include <string.h>
#include "gfal_srm_access.h"
#include <common/gfal_constants.h>
#include <common/gfal_common_err_helpers.h>
#include <common/gfal_common_plugin.h>
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_endpoint.h"


int gfal_access_srmv2_internal(gfal_srmv2_opt*  opts, char* endpoint, const char* surl, int mode,  GError** err){
	GError* tmp_err=NULL;
    srm_context_t context;
	struct srm_checkpermission_input checkpermission_input;
	struct srmv2_filestatus *resu;
	const int nb_request=1;
	char errbuf[GFAL_ERRMSG_LEN];
	int i;
	int ret=-1;
	char* tab_surl[] = { (char*)surl, NULL};



    if ( (context =  gfal_srm_ifce_context_setup(opts->handle, endpoint, errbuf, GFAL_ERRMSG_LEN, &tmp_err)) != NULL){
        checkpermission_input.nbfiles = nb_request;
        checkpermission_input.amode = mode;
        checkpermission_input.surls = tab_surl;

        ret = gfal_srm_external_call.srm_check_permission(context,&checkpermission_input, &resu);
        if(ret != nb_request){
            gfal_srm_report_error(errbuf, &tmp_err);
            gfal_srm_ifce_context_release(context);
            return -1;
        }
        for(i=0; i< nb_request; ++i){
            if( resu[i].status ){
                if( strnlen(resu[i].surl, GFAL_URL_MAX_LEN) >= GFAL_URL_MAX_LEN || strnlen(resu[i].explanation, GFAL_URL_MAX_LEN) >= GFAL_URL_MAX_LEN){
                    g_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), resu[i].status, " Memory corruption in the libgfal_srm_ifce answer, fatal");
                }else{
                    g_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), resu[i].status, "Error %d : %s  \
    , file %s: %s", resu[i].status, strerror(resu[i].status), resu[i].surl, resu[i].explanation);
                }
                ret= -1;
                break;
            }
            ret = 0;
        }
        errno = 0;
        //g_printerr(" resu : %d , status %d, strerror : %s, explanation : %s \n", ret, resu[0].status, strerror(resu[0].status), resu[0].explanation);
        gfal_srm_external_call.srm_srmv2_filestatus_delete(resu, nb_request);
    }
    gfal_srm_ifce_context_release(context);
    G_RETURN_ERR(ret, tmp_err,err);
}


/*
 * @brief access method for SRMv2
 * check the right for a given SRM url, work only for SRMv2, V1 deprecated.
 * @param ch the handle of the plugin
 * @param surl srm url of a given file
 * @param mode access mode to check
 * @param err : GError error reprot system
 * @warning : not safe, surl must be verified
 */ 
int gfal_srm_accessG(plugin_handle ch, const char* surl, int mode, GError** err){			// execute an access method on a srm url
	g_return_val_err_if_fail(ch && surl, EINVAL, err, "[gfal_srm_accessG] Invalid value handle and/or surl");
	GError* tmp_err=NULL;
	gfal_srmv2_opt* opts= (gfal_srmv2_opt*) ch;
	int ret=-1;
	char full_endpoint[GFAL_URL_MAX_LEN];
	enum gfal_srm_proto srm_types;
	ret =gfal_srm_determine_endpoint(opts, surl, full_endpoint, GFAL_URL_MAX_LEN, &srm_types,  &tmp_err); // get the associated endpoint
	if( ret != 0){		// check & get endpoint										
		g_propagate_prefixed_error(err,tmp_err, "[%s]", __func__);
		return -1;
	}
	
	if (srm_types == PROTO_SRMv2){			// check the proto version
		ret= gfal_access_srmv2_internal(opts, full_endpoint, surl, mode,&tmp_err);	// execute the SRMv2 access test
		if(tmp_err)
			g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	} else if(srm_types == PROTO_SRM){
            g_set_error(err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT, "[%s] support for SRMv1 is removed in 2.0, failure", __func__);
			ret =  -1;
	} else{
        g_set_error(err, gfal2_get_plugin_srm_quark(),EPROTONOSUPPORT, "[%s] Unknow version of the protocol SRM , failure ", __func__);
		ret=-1;
	}
	return ret;
}
