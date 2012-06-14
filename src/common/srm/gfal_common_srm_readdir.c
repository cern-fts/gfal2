/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * @file gfal_common_srm_readdir.c
 * @brief file for the readdir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 14/06/2011
 * */
#define GFAL_FILENAME_MAX FILENAME_MAX
#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>

#include <common/gfal_common_errverbose.h>

#include "gfal_common_srm_readdir.h"
#include "gfal_common_srm_opendir.h" 
#include "gfal_common_srm_internal_layer.h"
 

inline static void gfal_srm_bufferize_request(plugin_handle ch, const char* surl, struct srmv2_mdfilestatus * statuses){
	char buff_key[GFAL_URL_MAX_LEN];	
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;
	gfal_srm_construct_key(surl, GFAL_SRM_LSTAT_PREFIX, buff_key, GFAL_URL_MAX_LEN);
	struct stat* st = g_new(struct stat, 1);
	if(sizeof(struct stat64) == sizeof(struct stat))
		memcpy(st, &statuses->stat, sizeof(struct stat));
	else{
		const struct stat64* stat_statuses = &statuses->stat;
		st->st_dev = (dev_t) stat_statuses->st_dev;
		st->st_ino = (ino_t) stat_statuses->st_ino;
		st->st_mode = (mode_t) stat_statuses->st_mode;
		st->st_nlink = (nlink_t) stat_statuses->st_nlink;
		st->st_uid = (uid_t) stat_statuses->st_uid;
		st->st_gid = (gid_t) stat_statuses->st_gid;
		st->st_rdev = (dev_t) stat_statuses->st_rdev;
		st->st_size = (off_t) stat_statuses->st_size;
		st->st_blksize = (blkcnt_t) stat_statuses->st_blksize;
		st->st_blocks = (blkcnt_t) stat_statuses->st_blocks;
		st->st_atime = (time_t) stat_statuses->st_atime;
		st->st_mtime = (time_t) stat_statuses->st_mtime;
		st->st_ctime = (time_t) stat_statuses->st_ctime;
	}
	gsimplecache_add_item_kstr(opts->cache, buff_key, st);
}

inline static struct dirent* gfal_srm_readdir_convert_result(plugin_handle ch, const char* surl, struct srmv2_mdfilestatus * statuses,  struct dirent* output, GError ** err){
	struct dirent* resu = NULL;
	resu = output;
	char buff_surlfull[GFAL_URL_MAX_LEN];
	char* p = strrchr(statuses->surl,'/'); // keep only the file name + /
	if(p!=NULL){
		g_strlcpy(buff_surlfull, surl, GFAL_URL_MAX_LEN);
		g_strlcat(buff_surlfull, p, GFAL_URL_MAX_LEN);	
		gfal_srm_bufferize_request(ch, buff_surlfull, statuses);
		g_strlcpy(resu->d_name, p+1, GFAL_URL_MAX_LEN);	// without '/'
	}
	else
		g_strlcpy(resu->d_name, statuses->surl, GFAL_URL_MAX_LEN);
	return resu;
}

int gfal_srm_readdir_internal(plugin_handle ch, gfal_srm_opendir_handle oh, int nb_files, GError** err){
	g_return_val_err_if_fail(ch && oh, -1, err, "[gfal_srmv2_opendir_internal] invaldi args");
	GError* tmp_err=NULL;
	int resu =-1;
	struct srm_context context;
	struct srm_ls_input input;
	struct srm_ls_output output;
	struct srmv2_mdfilestatus *srmv2_mdstatuses=NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*)ch;
	char errbuf[GFAL_ERRMSG_LEN]={0};
	int ret =-1;
	int offset = oh->dir_offset;
	char* tab_surl[] = { (char*) oh->surl, NULL};
	
    gfal_srm_ifce_context_init(&context, opts->handle, oh->endpoint,
                                  errbuf, GFAL_ERRMSG_LEN, &tmp_err);	// init context
	
	input.nbfiles = 1;
	input.surls = tab_surl;
	input.numlevels = 1;
	input.offset = &offset;
	input.count = nb_files;

	ret = gfal_srm_external_call.srm_ls(&context,&input,&output);					// execute ls

	if(ret >=0){
		srmv2_mdstatuses = output.statuses;
		if(srmv2_mdstatuses[0].status != 0){
			g_set_error(err, 0, srmv2_mdstatuses->status, "[%s] Error reported from srm_ifce : %d %s", __func__, 
						srmv2_mdstatuses->status, srmv2_mdstatuses->explanation);
			resu = -1;	

		}else {
			oh->resu_offset = oh->dir_offset;
			oh->srm_ls_resu = &srmv2_mdstatuses[0];
			//cache system
			resu = 0;
		}	
	}else{
		gfal_srm_report_error(errbuf, &tmp_err);
		resu=-1;
	}

	gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);
		
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return resu;	
}

struct dirent* gfal_srm_readdir_pipeline(plugin_handle ch, gfal_srm_opendir_handle oh, GError** err){
	struct dirent* ret = NULL;
	GError* tmp_err=NULL;
	int max_resu_per_req= 0;
	
	if(oh->srm_ls_resu == NULL){
		gfal_srm_readdir_internal(ch, oh, max_resu_per_req, &tmp_err);	
		if(tmp_err && tmp_err->code == EINVAL){// fix in the case of short size SRMLs support, ( dcap )
			g_clear_error(&tmp_err);
			gfal_srm_readdir_internal(ch, oh, 1000, &tmp_err);	
		}	
	}else if(oh->dir_offset >= (oh->resu_offset+ oh->srm_ls_resu->nbsubpaths) ){
		return NULL; // limited mode in order to not overload the srm server ( slow )
		/*
		gfal_srm_external_call.srm_srmv2_mdfilestatus_delete(oh->srm_ls_resu,1);	
		gfal_srm_readdir_internal(ch, oh, max_resu_per_req, &tmp_err);*/
	}
	if(!tmp_err){
		if(oh->srm_ls_resu->nbsubpaths == 0) // end of the list !!
			return NULL;
		const off_t myoffset = oh->dir_offset - oh->resu_offset;
		ret = gfal_srm_readdir_convert_result(ch, oh->surl, &oh->srm_ls_resu->subpaths[myoffset], &oh->current_readdir, &tmp_err);
		oh->dir_offset += 1;
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;	
}


struct dirent* gfal_srm_readdirG(plugin_handle ch, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail( ch && fh, NULL, err, "[gfal_srm_readdirG] Invalid args");
	GError* tmp_err=NULL;
	struct dirent* ret = NULL;
	if(fh != NULL){
		gfal_srm_opendir_handle oh = (gfal_srm_opendir_handle) fh->fdesc;
		ret = gfal_srm_readdir_pipeline(ch, oh, &tmp_err);		
	}else{
		g_set_error(&tmp_err, 0, EBADF, "bad dir descriptor");
		ret = NULL;
	}

	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return ret;
}
