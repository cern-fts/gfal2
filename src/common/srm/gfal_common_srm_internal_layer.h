#pragma once
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
 * @file gfal_common_srm_internal_layer.h
 * @brief header file for the srm external function mapping for mocking purpose
 * @author Devresse Adrien
 * @version 2.0
 * @date 09/06/2011
 * */

#include <gfal_srm_ifce.h> 
#include <gfal_srm_ifce_types.h> 
#include <glib.h>

#include <common/gfal_types.h>
#include <common/gfal_common_errverbose.h>
#include <config/gfal_config.h>
#include <transfer/gfal_transfer.h>

#include "gfal_common_srm_endpoint.h"
#include "gfal_common_srm.h"

extern const char * srm_config_group;
extern const char * srm_config_transfer_checksum;
extern const char * srm_config_turl_protocols;
extern const char * srm_config_3rd_party_turl_protocols;

// request type for surl <-> turl translation 
typedef enum _srm_req_type{
	SRM_GET,
	SRM_PUT
} srm_req_type;


/*
 * structure for mock abylity in the srm part
 *
 */
struct _gfal_srm_external_call{
	
	void (*srm_context_init)(struct srm_context *context,char *srm_endpoint,char *errbuf,int errbufsz,int verbose);
	
	int (*srm_ls)(struct srm_context *context,
		struct srm_ls_input *input,struct srm_ls_output *output);

	int (*srm_rm)(struct srm_context *context,
			struct srm_rm_input *input,struct srm_rm_output *output);
		
	int (*srm_rmdir)(struct srm_context *context,
		struct srm_rmdir_input *input,struct srm_rmdir_output *output);
		
	int (*srm_mkdir)(struct srm_context *context,
		struct srm_mkdir_input *input);
		
	int (*srm_getpermission) (struct srm_context *context,
		struct srm_getpermission_input *input,struct srm_getpermission_output *output);

	int (*srm_check_permission)(struct srm_context *context,
		struct srm_checkpermission_input *input,struct srmv2_filestatus **statuses);	

	int (*srm_prepare_to_get)(struct srm_context *context,
		struct srm_preparetoget_input *input,struct srm_preparetoget_output *output);
		
	void (*srm_srmv2_pinfilestatus_delete)(struct srmv2_pinfilestatus*  srmv2_pinstatuses, int n);
	
	void (*srm_srmv2_mdfilestatus_delete)(struct srmv2_mdfilestatus* mdfilestatus, int n);
	
	void (*srm_srmv2_filestatus_delete)(struct srmv2_filestatus*  srmv2_statuses, int n);
	
	void (*srm_srm2__TReturnStatus_delete)(struct srm2__TReturnStatus* status);
	
	int (*srm_prepare_to_put)(struct srm_context *context,
		struct srm_preparetoput_input *input,struct srm_preparetoput_output *output);

	int (*srm_put_done)(struct srm_context *context,
			struct srm_putdone_input *input, struct srmv2_filestatus **statuses);
			
	int (*srm_setpermission) (struct srm_context *context,
			struct srm_setpermission_input *input);

    void (*srm_set_timeout_connect) (int);
	
};

extern struct _gfal_srm_external_call gfal_srm_external_call;

int gfal_check_fullendpoint_in_surl(const char * surl, GError ** err);


gboolean gfal_srm_surl_group_checker(gfal_srmv2_opt* opts,char** surls, GError** err);

int gfal_srm_getTURLS_plugin(plugin_handle ch, const char* surl, char* buff_turl, int size_turl, char** reqtoken, GError** err);

int gfal_srm_putTURLS_plugin(plugin_handle ch, const char* surl, char* buff_turl, int size_turl, char** reqtoken, GError** err);

int gfal_srm_getTURLS(gfal_srmv2_opt* opts, char** surls, gfal_srm_result** resu,  GError** err);

int gfal_srm_putTURLS(gfal_srmv2_opt* opts , char** surls, gfal_srm_result** resu,  GError** err);

int gfal_srm_putdone(gfal_srmv2_opt* opts, char** surls, char* token,  GError** err);

int gfal_srm_unlinkG(plugin_handle ch, const char * path, GError** err);

void gfal_srm_report_error(char* errbuff, GError** err);


int gfal_srm_ifce_context_init(struct srm_context* context, gfal_context_t handle, const char* endpoint,
                                char* errbuff, size_t s_errbuff, GError** err);


srm_context_t gfal_srm_ifce_context_setup(gfal_context_t handle, const char* endpoint,
                                                char* errbuff, size_t s_errbuff, GError** err);

void gfal_srm_ifce_context_release(srm_context_t context);

int gfal_srm_ifce_context_deinit(struct srm_context* context);
