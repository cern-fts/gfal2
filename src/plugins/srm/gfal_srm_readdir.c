/*
 * Copyright (c) CERN 2013-2015
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
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

#define GFAL_FILENAME_MAX FILENAME_MAX


#include <string.h>
#include <stdio.h>

#include "gfal_srm_namespace.h"
#include "gfal_srm_opendir.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_internal_ls.h"


/**
 * Casts a 64 bits stat into whatever stat type we are using in this compilation unit.
 * Intended for cases where we compile using 32 bits size types, which I don't believe to be
 * the case any more, but just in case...
 */
static void gfal_srm_stat64_to_stat(const struct stat64 *st64, struct stat *st)
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


/**
 * Converts a SRM status into a dirent + struct stat
 * Returns, for convenience, the same pointer passed as dir_ent
 */
static struct dirent *gfal_srm_readdir_convert_result(plugin_handle ch,
    const char *parent_surl, const struct srmv2_mdfilestatus *srm_status,
    struct dirent *dir_ent, struct stat *st, GError **err)
{
    char buff_surlfull[GFAL_URL_MAX_LEN];

    char *p = strrchr(srm_status->surl, '/');
    if (p != NULL) {
        g_strlcpy(buff_surlfull, parent_surl, GFAL_URL_MAX_LEN);
        g_strlcat(buff_surlfull, p, GFAL_URL_MAX_LEN);

        dir_ent->d_reclen = g_strlcpy(dir_ent->d_name, p + 1, GFAL_URL_MAX_LEN);
    }
    else {
        g_strlcpy(buff_surlfull, parent_surl, GFAL_URL_MAX_LEN);
        g_strlcat(buff_surlfull, "/", GFAL_URL_MAX_LEN);
        g_strlcat(buff_surlfull, p, GFAL_URL_MAX_LEN);

        dir_ent->d_reclen = g_strlcpy(dir_ent->d_name, srm_status->surl, GFAL_URL_MAX_LEN);
    }

    if (S_ISDIR(srm_status->stat.st_mode))
        dir_ent->d_type = DT_DIR;
    else if (S_ISLNK(srm_status->stat.st_mode))
        dir_ent->d_type = DT_LNK;
    else
        dir_ent->d_type = DT_REG;

    gfal_srm_stat64_to_stat(&srm_status->stat, st);
    // Stores cache information
    gfal_srm_cache_stat_add(ch, buff_surlfull, st, &srm_status->locality);

    return dir_ent;
}


/**
 * Wraps the actual call to srm-ifce
 */
static int gfal_srm_readdir_internal(plugin_handle ch,
    gfal_srm_opendir_handle oh, GError **err)
{
    g_return_val_err_if_fail(ch && oh, -1, err, "[gfal_srmv2_opendir_internal] invaldi args");
    GError *tmp_err = NULL;
    int resu = -1;
    struct srm_ls_input input;
    struct srm_ls_output output;
    struct srmv2_mdfilestatus *srmv2_mdstatuses = NULL;
    int ret = -1;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(ch, oh->surl, &tmp_err);
    if (!easy) {
        G_RETURN_ERR(resu, tmp_err, err);
    }
    char *tab_surl[] = {easy->path, NULL};

    input.nbfiles = 1;
    input.surls = tab_surl;
    input.numlevels = 1;
    input.count = oh->chunk_size;
    // Mind that srm_ls may - or may not - modify the value pointed by input.offset
    int offset_buffer = oh->chunk_offset;
    input.offset = &offset_buffer;

    oh->response_index = 0;
    ret = gfal_srm_external_call.srm_ls(easy->srm_context, &input, &output);

    if (ret >= 0) {
        srmv2_mdstatuses = output.statuses;
        if (srmv2_mdstatuses[0].status != 0) {
            gfal2_set_error(err, gfal2_get_plugin_srm_quark(),
                srmv2_mdstatuses->status, __func__,
                "Error reported from srm_ifce : %d %s",
                srmv2_mdstatuses->status, srmv2_mdstatuses->explanation);
            resu = -1;

        }
        else {
            oh->srm_file_statuses = &srmv2_mdstatuses[0];
            resu = 0;
        }
    }
    else {
        gfal_srm_report_error(easy->srm_context->errbuf, &tmp_err);
        resu = -1;
    }
    gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);

    gfal_srm_ifce_easy_context_release(ch, easy);
    G_RETURN_ERR(resu, tmp_err, err);
}


/**
 * Wraps the SRM request.
 * Request each chunks, then iterates through the responses as readdir is called
 */
static struct dirent *gfal_srm_readdir_pipeline(plugin_handle ch,
    gfal_srm_opendir_handle oh, struct stat *st, GError **err)
{
    GError *tmp_err = NULL;

    // Nothing yet, so get the bulk
    if (oh->srm_file_statuses == NULL) {
        gfal_srm_readdir_internal(ch, oh, &tmp_err);
        if (tmp_err) {
            gfal2_propagate_prefixed_error(err, tmp_err, __func__);
            return NULL;
        }
    }

    // Empty directory
    if (oh->srm_file_statuses == NULL || oh->srm_file_statuses->nbsubpaths == 0) {
        return NULL;
    }

    // Done here
    if (oh->response_index >= oh->srm_file_statuses->nbsubpaths) {
        return NULL;
    }

    // Iterate and return statuses
    struct dirent *ret = gfal_srm_readdir_convert_result(ch, oh->surl,
        &oh->srm_file_statuses->subpaths[oh->response_index], &oh->dirent_buffer,
        st, &tmp_err);
    oh->response_index++;

    // If chunk listing, and the index passed the last entry in the buffer,
    // release, and prepare next bulk
    if (oh->is_chunked_listing && oh->response_index >= oh->chunk_size) {
        oh->chunk_offset += oh->chunk_size;
        gfal_srm_external_call.srm_srmv2_mdfilestatus_delete(oh->srm_file_statuses, 1);
        oh->srm_file_statuses = NULL;
    }

    return ret;
}


/**
 * Only read.
 * SRM listing returns the file stat anyway, so wrap Read + Stat and discard the stat
 */
struct dirent *gfal_srm_readdirG(plugin_handle ch, gfal_file_handle fh, GError **err)
{
    g_return_val_err_if_fail(ch && fh, NULL, err, "[gfal_srm_readdirG] Invalid args");
    struct stat _; // Ignore this
    return gfal_srm_readdirppG(ch, fh, &_, err);
}


/**
 * Read + Stat
 */
struct dirent *gfal_srm_readdirppG(plugin_handle ch,
    gfal_file_handle fh, struct stat *st, GError **err)
{
    g_return_val_err_if_fail(ch && fh, NULL, err, "[gfal_srm_readdirppG] Invalid args");
    GError *tmp_err = NULL;

    struct dirent *ret = NULL;
    gfal_srm_opendir_handle oh = (gfal_srm_opendir_handle) fh->fdesc;
    ret = gfal_srm_readdir_pipeline(ch, oh, st, &tmp_err);

    // Directory too big, so prepare to read in chunks and delegate
    if (tmp_err && tmp_err->code == EFBIG) {
        // If we already tried, abort!
        if (oh->is_chunked_listing) {
            gfal2_propagate_prefixed_error_extended(err, tmp_err, __func__,
                "EFBIG received when already trying chunk listing");
            return NULL;
        }
        // Prepare for chunk listing, and re-issue
        g_clear_error(&tmp_err);
        oh->is_chunked_listing = 1;
        oh->chunk_offset = 0;
        oh->chunk_size = 1000;
        oh->response_index = 0;

        gfal2_log(G_LOG_LEVEL_WARNING,
            "EFBIG while listing SRM directory, trying with chunk listing of size %d",
            oh->chunk_size);

        ret = gfal_srm_readdir_pipeline(ch, oh, st, &tmp_err);
        if (tmp_err)
            gfal2_propagate_prefixed_error_extended(err, tmp_err, __func__,
                "Failed when attempting chunk listing");
    }
        // Just an error
    else if (tmp_err) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    }
    return ret;
}
