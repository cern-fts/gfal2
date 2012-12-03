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
 * @file gfal_common.c
 * @brief file for the gfal's plugin unlink function
 * @author Devresse Adrien
 * @version 2.0
 * @date 12/06/2011
 * */
 
 #define _GNU_SOURCE 

#include <regex.h>
#include <time.h> 

#include "gfal_common_srm.h"
#include "gfal_common_srm_internal_layer.h" 
#include "gfal_common_srm_internal_ls.h"
#include <common/gfal_common_internal.h>
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_plugin.h>
#include "gfal_common_srm_endpoint.h"


static int gfal_srm_rm_srmv2_internal(gfal_srmv2_opt* opts, const char* full_endpoint, char** surls, GError** err){
	GError* tmp_err=NULL;
	struct srm_context context;
	struct srm_rm_input input;
	struct srm_rm_output output;
	const int nb_request=1;
	char errbuf[GFAL_ERRMSG_LEN]={0};
	int i;
	int ret=-1;


    gfal_srm_ifce_context_init(&context, opts->handle, full_endpoint,
                                  errbuf, GFAL_ERRMSG_LEN, &tmp_err);

	input.nbfiles = nb_request;
	input.surls = surls;

	ret = gfal_srm_external_call.srm_rm(&context,&input, &output);	
	
	if(ret == nb_request){
		ret =0;
		struct srmv2_filestatus* statuses = output.statuses;
		for(i=0; i < nb_request;++i){
			if(statuses[i].status!=0){
				if(statuses[i].explanation != NULL)
					g_set_error(&tmp_err, 0, statuses[i].status," error reported from srm_ifce, %s ", statuses[i].explanation );
				else
					g_set_error(&tmp_err, 0, EINVAL ," error reported from srm_ifce with corrputed memory ! ");
				ret = -1;
				break;
			}
		}
			
		gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);
		gfal_srm_external_call.srm_srmv2_filestatus_delete(output.statuses, ret);
	}else{
		gfal_srm_report_error(errbuf, &tmp_err);
		ret= -1;		
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
	
	
	
}

int gfal_srm_rm_internal(gfal_srmv2_opt* opts, char** surls, GError** err){
	GError* tmp_err=NULL;
	int ret=-1;	

	char full_endpoint[GFAL_URL_MAX_LEN];
	enum gfal_srm_proto srm_types;
	if((gfal_srm_determine_endpoint(opts, *surls, full_endpoint, GFAL_URL_MAX_LEN, &srm_types, &tmp_err)) == 0){		// check & get endpoint										
		gfal_log(GFAL_VERBOSE_NORMAL, "gfal_srm_rm_internal -> endpoint %s", full_endpoint);

		if (srm_types == PROTO_SRMv2){
				ret= gfal_srm_rm_srmv2_internal(opts, full_endpoint, surls, &tmp_err);	
		} else if(srm_types == PROTO_SRM){
			g_set_error(&tmp_err,0, EPROTONOSUPPORT, "support for SRMv1 is removed in gfal 2.0, failure");
		} else{
			g_set_error(&tmp_err,0,EPROTONOSUPPORT, "Unknow SRM protocol, failure ");
		}		
	}

	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}


/**
 * 
 * bindings of the unlink plugin call
*/
int gfal_srm_unlinkG(plugin_handle ch, const char * path, GError** err){
	g_return_val_err_if_fail( ch && path, -1, err, "[gfal_srm_unlinkG] incorrects args");
	GError* tmp_err=NULL;
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*)ch;
	
	char* surls[] = { (char*) path, NULL};
    gfal_srm_cache_stat_remove(ch, path);
	int ret = gfal_srm_rm_internal(opts, surls, &tmp_err);
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;		
}




