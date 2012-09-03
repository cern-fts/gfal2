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

 * @file gfal_common_srm_internal_ls.c
 * @brief srm ls operation concentrator
 * @author Devresse Adrien
 * @version 2.0
 * @date 21/12/2011
 * */

#define _GNU_SOURCE

#include <common/gfal_constants.h>
#include <common/gfal_common_errverbose.h> 

#include "gfal_common_srm_internal_ls.h"
#include "gfal_common_srm_endpoint.h"

/*
 * clear memory used by the internal srm_ifce items
 * required by the old design of srm_ifce
 * 
 * */
void gfal_srm_ls_memory_management(struct srm_ls_input* input, struct srm_ls_output* output){
	if(input){
		// nothing
		
	}
	if(output){
		gfal_srm_external_call.srm_srmv2_mdfilestatus_delete(output->statuses, 1);
		gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output->retstatus);		
		
	}	
}

/*
 *  concentrate the srm_ls logical in one point for stat, readdir, status, and access
 * 
 * */
int gfal_srm_ls_internal(gfal_srmv2_opt* opts, const char* endpoint, 
						 struct srm_ls_input* input, struct srm_ls_output* output, 
						 GError** err){
	GError* tmp_err=NULL;
	struct srm_context context;
	
	char errbuf[GFAL_ERRMSG_LEN]={0};	
	int ret;							

    gfal_srm_ifce_context_init(&context, opts->handle, endpoint,
                                  errbuf, GFAL_ERRMSG_LEN, &tmp_err);	// init context
		
	
	if( (ret = gfal_srm_external_call.srm_ls(&context, input, output) ) < 0){
		gfal_srm_report_error(errbuf, &tmp_err);
		ret = -1;			
	}
								
	G_RETURN_ERR(ret, tmp_err, err);										
}


int gfal_statG_srmv2__generic_internal(	gfal_srmv2_opt* opts, struct stat* buf, 
										const char* endpoint, const char* surl, 
										GError** err){
	g_return_val_err_if_fail( opts && endpoint && surl 
								 && buf && (sizeof(struct stat) == sizeof(struct stat64)),
                                -1, err, "[gfal_statG_srmv2_generic_internal] Invalid args handle/endpoint or invalid stat struct size");
	GError* tmp_err=NULL;
	struct srm_ls_input input;
	struct srm_ls_output output;
	struct srmv2_mdfilestatus *srmv2_mdstatuses=NULL;
	const int nb_request=1;	
	int ret=-1;
	char* tab_surl[] = { (char*)surl, NULL};

	input.nbfiles = nb_request;
	input.surls = tab_surl;
	input.numlevels = 0;
	input.offset = 0;
	input.count = 0;
	
	ret = gfal_srm_ls_internal(opts, endpoint, &input, &output, &tmp_err);

	if(ret >=0){
		srmv2_mdstatuses = output.statuses;
		if(srmv2_mdstatuses->status != 0){
			g_set_error(&tmp_err, 0, srmv2_mdstatuses->status, "Error reported from srm_ifce : %d %s", 
							srmv2_mdstatuses->status, srmv2_mdstatuses->explanation);
			ret = -1;
		} else {
			memcpy(buf, &(srmv2_mdstatuses->stat), sizeof(struct stat));
			errno =0;
			ret =0;
		}
	}
	gfal_srm_ls_memory_management(&input, &output);
	
	G_RETURN_ERR(ret, tmp_err, err);
}


int gfal_Locality_srmv2_generic_internal(	gfal_srmv2_opt* opts, 
										const char* endpoint, const char* surl, TFileLocality* loc,
										GError** err){
	g_return_val_err_if_fail( opts && endpoint && surl && loc,
								-1, err, "[gfal_statG_srmv2_generic_internal] Invalid args handle/endpoint or invalid stat struct size");
	GError* tmp_err=NULL;
	struct srm_ls_input input;
	struct srm_ls_output output;
	struct srmv2_mdfilestatus *srmv2_mdstatuses=NULL;
	const int nb_request=1;	
	int ret=-1;
	char* tab_surl[] = { (char*)surl, NULL};

	input.nbfiles = nb_request;
	input.surls = tab_surl;
	input.numlevels = 0;
	input.offset = 0;
	input.count = 0;
	
	ret = gfal_srm_ls_internal(opts, endpoint, &input, &output, &tmp_err);

	if(ret >=0){
		srmv2_mdstatuses = output.statuses;
		if(srmv2_mdstatuses->status != 0){
			g_set_error(&tmp_err, 0, srmv2_mdstatuses->status, "Error  srm_ifce : %d %s", 
							srmv2_mdstatuses->status, srmv2_mdstatuses->explanation);
			ret = -1;
		} else {
			*loc = srmv2_mdstatuses->locality;
			ret = 0;
			errno =0;
		}
	}
	gfal_srm_ls_memory_management(&input, &output);
	
	G_RETURN_ERR(ret, tmp_err, err);
}						
