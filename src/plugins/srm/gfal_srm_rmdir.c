/*
 * Copyright (c) CERN 2013-2017
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

#include "gfal_srm.h"
#include "gfal_srm_namespace.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_endpoint.h"
#include "gfal_srm_internal_ls.h"


static int gfal_srmv2_rmdir_internal(srm_context_t context, const char *surl, GError **err)
{
    struct srm_rmdir_input rmdir_input;
    struct srm_rmdir_output rmdir_output;
    GError *tmp_err = NULL;
    int ret = -1;

    rmdir_input.recursive = 0;
    rmdir_input.surl = (char *) surl;

    if (gfal_srm_external_call.srm_rmdir(context, &rmdir_input, &rmdir_output) >= 0) {
        const int sav_errno = rmdir_output.statuses[0].status;
        if (sav_errno) {
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), sav_errno,
                __func__, "Error report from the srm_ifce %s ", strerror(sav_errno));
            ret = -1;
        }
        else {
            ret = 0;
        }
        gfal_srm_external_call.srm_srmv2_filestatus_delete(rmdir_output.statuses, 1);
        gfal_srm_external_call.srm_srm2__TReturnStatus_delete(rmdir_output.retstatus);
    }
    else {
        gfal_srm_report_error(context->errbuf, &tmp_err);
        ret = -1;
    }

    G_RETURN_ERR(ret, tmp_err, err);
}


int gfal_srm_rmdirG(plugin_handle ch, const char *surl, GError **err)
{
    g_return_val_err_if_fail(ch && surl, EINVAL, err, "[gfal_srm_rmdirG] Invalid value handle and/or surl");
    GError *tmp_err = NULL;
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;

    int ret = -1;

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (easy != NULL) {
        struct stat st;
        gfal2_log(G_LOG_LEVEL_DEBUG, "   [gfal_srm_rmdirG] try to delete directory %s", surl);
        ret = gfal_statG_srmv2_internal(easy->srm_context, &st, NULL, easy->path, &tmp_err);
        if (ret == 0) {
            if (S_ISDIR(st.st_mode)) {
                gfal_srm_cache_stat_remove(ch, surl); // invalidate cache entry
                ret = gfal_srmv2_rmdir_internal(easy->srm_context, easy->path, &tmp_err);
            }
            else {
                ret = -1;
                gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), ENOTDIR, __func__,
                    "This file is not a directory, impossible to use rmdir on it");
            }
        }
    }
    gfal_srm_ifce_easy_context_release(opts, easy);

    if (ret != 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}
