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
 * @file gfal_srm_readdir.c
 * @brief file for the readdir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 14/06/2011
 * */
#define GFAL_FILENAME_MAX FILENAME_MAX


#include <string.h>
#include <stdio.h>

#include "gfal_srm_namespace.h"
#include "gfal_srm_opendir.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_internal_ls.h"

static void gfal_srm_stat64_to_stat(const struct stat64* st64, struct stat *st)
{
    if (sizeof(struct stat64) == sizeof(struct stat))
        memcpy(st, st64, sizeof(*st));
    else {
        st->st_dev = (dev_t) st64->st_dev;
        st->st_ino = (ino_t) st64->st_ino;
        st->st_mode = (mode_t) st64->st_mode;
        st->st_nlink = (nlink_t) st64->st_nlink;
        st->st_uid = (uid_t) st64->st_uid;
        st->st_gid = (gid_t) st64->st_gid;
        st->st_rdev = (dev_t) st64->st_rdev;
        st->st_size = (off_t) st64->st_size;
        st->st_blksize = (blkcnt_t) st64->st_blksize;
        st->st_blocks = (blkcnt_t) st64->st_blocks;
        st->st_atime = (time_t) st64->st_atime;
        st->st_mtime = (time_t) st64->st_mtime;
        st->st_ctime = (time_t) st64->st_ctime;
    }
}


inline static struct dirent* gfal_srm_readdir_convert_result(plugin_handle ch,
        const char* surl, struct srmv2_mdfilestatus * statuses,
        struct dirent* output, struct stat* st, GError ** err)
{
    struct dirent* resu = NULL;
    resu = output;
    char buff_surlfull[GFAL_URL_MAX_LEN];
    char* p = strrchr(statuses->surl, '/'); // keep only the file name + /
    if (p != NULL) {
        g_strlcpy(buff_surlfull, surl, GFAL_URL_MAX_LEN);
        g_strlcat(buff_surlfull, p, GFAL_URL_MAX_LEN);
        resu->d_reclen = g_strlcpy(resu->d_name, p + 1, GFAL_URL_MAX_LEN); // without '/'

        if (S_ISDIR(statuses->stat.st_mode))
            resu->d_type = DT_DIR;
        else if (S_ISLNK(statuses->stat.st_mode))
            resu->d_type = DT_LNK;
        else
            resu->d_type = DT_REG;

        gfal_srm_stat64_to_stat(&statuses->stat, st);
        gfal_srm_cache_stat_add(ch, buff_surlfull, st, &statuses->locality);
    }
    else
        g_strlcpy(resu->d_name, statuses->surl, GFAL_URL_MAX_LEN);
    return resu;
}


static int gfal_srm_readdir_internal(plugin_handle ch,
        gfal_srm_opendir_handle oh, GError** err)
{
	g_return_val_err_if_fail(ch && oh, -1, err, "[gfal_srmv2_opendir_internal] invaldi args");
	GError* tmp_err = NULL;
	int resu = -1;
    srm_context_t context;
	struct srm_ls_input input;
	struct srm_ls_output output;
	struct srmv2_mdfilestatus *srmv2_mdstatuses=NULL;
	int ret =-1;
	char* tab_surl[] = { (char*) oh->surl, NULL};

	memset(&input, 0, sizeof(input));
	memset(&output, 0, sizeof(output));

	context = gfal_srm_ifce_easy_context(ch, oh->surl, &tmp_err);
	if (!context) {
	    G_RETURN_ERR(resu, tmp_err, err);
	}

    input.nbfiles = 1;
    input.surls = tab_surl;
    input.numlevels = 1;
    input.count = oh->max_count - oh->count;
    input.offset = &oh->slice_offset;

    oh->slice_index = 0;

    /*
     * Mind that srm_ls will modify the value pointed by input.offset, so even if it has some
     * value, it will be reset to the offset of the next chunk if any!
     * Why is it called input then? I don't know.
     */
    ret = gfal_srm_external_call.srm_ls(context, &input, &output);

    if(ret >=0){
        srmv2_mdstatuses = output.statuses;
        if(srmv2_mdstatuses[0].status != 0){
            gfal2_set_error(err, gfal2_get_plugin_srm_quark(), srmv2_mdstatuses->status, __func__,
                    "Error reported from srm_ifce : %d %s",
                    srmv2_mdstatuses->status, srmv2_mdstatuses->explanation);
            resu = -1;

        }else {
            oh->srm_ls_resu = &srmv2_mdstatuses[0];
            //cache system
            resu = 0;
        }
    }else{
        gfal_srm_report_error(context->errbuf, &tmp_err);
        resu=-1;
    }
    gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);

    gfal_srm_ifce_easy_context_release(ch, context);
    G_RETURN_ERR(resu, tmp_err, err);
}


static struct dirent* gfal_srm_readdir_pipeline(plugin_handle ch,
        gfal_srm_opendir_handle oh, struct stat* st, GError** err)
{
    struct dirent* ret = NULL;
    GError* tmp_err = NULL;

    if (oh->srm_ls_resu == NULL ) {
        gfal_srm_readdir_internal(ch, oh, &tmp_err);
        if (tmp_err && tmp_err->code == EINVAL) { // fix in the case of short size SRMLs support, ( dcap )
            g_clear_error(&tmp_err);
            oh->max_count = 1000;
            gfal_srm_readdir_internal(ch, oh, &tmp_err);
        }
    }
    else if (oh->slice_index >= oh->srm_ls_resu->nbsubpaths) {
        return NULL ; // limited mode in order to not overload the srm server ( slow )
    }

    // Empty directory
    if (oh->srm_ls_resu == NULL || oh->srm_ls_resu->nbsubpaths == 0) {
        return NULL;
    }

    if (!tmp_err) {
        ret = gfal_srm_readdir_convert_result(ch, oh->surl,
                &oh->srm_ls_resu->subpaths[oh->slice_index], &oh->current_readdir,
                st,
                &tmp_err);

        oh->count++;
        oh->slice_index++;
    }
    else
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    return ret;
}


struct dirent* gfal_srm_readdirG(plugin_handle ch, gfal_file_handle fh, GError** err)
{
	g_return_val_err_if_fail( ch && fh, NULL, err, "[gfal_srm_readdirG] Invalid args");
	struct stat _;
	return gfal_srm_readdirppG(ch, fh, &_, err);
}


struct dirent* gfal_srm_readdirppG(plugin_handle ch,
        gfal_file_handle fh, struct stat* st, GError** err)
{
    g_return_val_err_if_fail( ch && fh, NULL, err, "[gfal_srm_readdirppG] Invalid args");
    GError* tmp_err = NULL;

    struct dirent* ret = NULL;
    gfal_srm_opendir_handle oh = (gfal_srm_opendir_handle) fh->fdesc;
    ret = gfal_srm_readdir_pipeline(ch, oh, st, &tmp_err);

    if(tmp_err)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    return ret;
}
