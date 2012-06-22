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
 * @brief file for the open/read/write srm
 * @author Devresse Adrien
 * @date 06/07/2011
 * */
 
 #define _GNU_SOURCE 

#include <regex.h>
#include <time.h> 
#include <fcntl.h> 
#include <stdlib.h>
#include <common/gfal_common_internal.h>
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_filedescriptor.h>
#include "gfal_common_srm_internal_layer.h"
#include "gfal_common_srm.h"


typedef struct _gfal_srm_handle_open{
	gfal_file_handle internal_handle;
	char surl[GFAL_URL_MAX_LEN];
	srm_req_type req_type;
	char* reqtoken;
}*gfal_srm_handle_open;

static gfal_file_handle gfal_srm_file_handle_create(gfal_file_handle fh, char* surl, char* reqtoken, srm_req_type req_type){
	if(fh== NULL)
		return NULL;
	gfal_srm_handle_open sh = g_new(struct _gfal_srm_handle_open,1);
	sh->internal_handle = fh;
	g_strlcpy(sh->surl, surl, GFAL_URL_MAX_LEN);
	sh->reqtoken= reqtoken;
	sh->req_type = req_type;
	return gfal_file_handle_new(gfal_srm_getName(), sh);
}

static gfal_file_handle gfal_srm_file_handle_map(gfal_file_handle fh){
	return ((gfal_srm_handle_open) fh->fdesc)->internal_handle;
}

static void gfal_srm_file_handle_delete(gfal_file_handle fh){
	free(fh->fdesc);
	gfal_file_handle_delete(fh);
}

/*
 * open function for the srm  plugin
 */
gfal_file_handle gfal_srm_openG(plugin_handle ch, const char* path, int flag, mode_t mode, GError** err){
	gfal_file_handle ret = NULL;
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;	
	GError* tmp_err=NULL;
	char* p = (char*)path;
	char turl[GFAL_URL_MAX_LEN];
	char* reqtoken=NULL;
	srm_req_type req_type;
	int tmp_ret;
	gfal_log(GFAL_VERBOSE_TRACE, "  %s ->",__func__);
	
	if(flag & O_CREAT){ // create turl if file is not existing else get one for this file
		gfal_log(GFAL_VERBOSE_TRACE, "   SRM PUT mode",__func__);
		tmp_ret= gfal_srm_putTURLS_plugin(ch, p, turl, GFAL_URL_MAX_LEN, &reqtoken, &tmp_err);
		req_type= SRM_PUT;
	}else{
		gfal_log(GFAL_VERBOSE_TRACE, "   SRM GET mode",__func__);
		tmp_ret= gfal_srm_getTURLS_plugin(ch, p, turl, GFAL_URL_MAX_LEN, &reqtoken, &tmp_err);
		req_type= SRM_GET;
	}
	
	if(tmp_ret == 0){
		gfal_log(GFAL_VERBOSE_TRACE, "  SRM RESOLUTION : %s -> %s ", path, turl);
		ret = gfal_plugin_openG(opts->handle, turl, flag, mode, &tmp_err);
		ret= gfal_srm_file_handle_create(ret, p, reqtoken, req_type);
	}

	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
	
}

/*
 * read function for the srm  plugin
 */
ssize_t gfal_srm_readG(plugin_handle ch, gfal_file_handle fd, void* buff, size_t count, GError** err){
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;	
	GError* tmp_err=NULL;
	int ret =  gfal_plugin_readG(opts->handle, gfal_srm_file_handle_map(fd), buff, count, &tmp_err);	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;		
}

/*
 * pread function for the srm  plugin
 */
ssize_t gfal_srm_preadG(plugin_handle ch, gfal_file_handle fd, void* buff, size_t count, off_t offset, GError** err){
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;	
	GError* tmp_err=NULL;
	int ret =  gfal_plugin_preadG(opts->handle, gfal_srm_file_handle_map(fd), buff, count, offset, &tmp_err);	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;		
}

/*
 * write function for the srm  plugin
 */
ssize_t gfal_srm_writeG(plugin_handle ch, gfal_file_handle fd, const void* buff, size_t count, GError** err){
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;	
	GError* tmp_err=NULL;
	int ret = gfal_plugin_writeG(opts->handle, gfal_srm_file_handle_map(fd), (void* )buff, count, &tmp_err);	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}

/*
 * lseek function for the srm  plugin
 */
off_t gfal_srm_lseekG(plugin_handle ch, gfal_file_handle fd, off_t offset, int whence, GError** err){
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;	
	GError* tmp_err=NULL;
	int ret = gfal_plugin_lseekG(opts->handle, gfal_srm_file_handle_map(fd), offset, whence, &tmp_err);	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}

int gfal_srm_closeG(plugin_handle ch, gfal_file_handle fh, GError ** err){
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*)	ch;	
	GError* tmp_err=NULL;
	int ret = gfal_plugin_closeG(opts->handle, gfal_srm_file_handle_map(fh), &tmp_err);
	if(ret ==0){
		gfal_srm_handle_open sh = (gfal_srm_handle_open)fh->fdesc;
		char* surls[] = { sh->surl, NULL };
		if(sh->req_type == SRM_PUT)
			ret = gfal_srm_putdone(opts, surls, sh->reqtoken, &tmp_err); // end the transaction on the srm server in case of pu
		gfal_srm_file_handle_delete(fh);
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
		
	return ret;	
}


