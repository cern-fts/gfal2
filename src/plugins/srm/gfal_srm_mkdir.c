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

#include "gfal_srm.h"
#include "gfal_srm_namespace.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_endpoint.h"


static int gfal_mkdir_srmv2_internal(srm_context_t context, const char* path, mode_t mode, GError** err)
{
	struct srm_mkdir_input mkdir_input;
	int res = -1;
	GError* tmp_err=NULL;

	errno = 0;
    mkdir_input.dir_name = (char*) path;
    res  = gfal_srm_external_call.srm_mkdir(context, &mkdir_input);

    if (res < 0) {
        gfal_srm_report_error(context->errbuf, &tmp_err);
        res = -1;
    }
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal_srm_mkdir_recG(plugin_handle ch, const char* surl, mode_t mode, GError** err)
{
    g_return_val_err_if_fail(ch && surl, EINVAL, err, "[gfal_srm_mkdir_recG] Invalid value handle and/or surl");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    int ret = -1;

    gfal2_log(G_LOG_LEVEL_DEBUG, "  ->  [gfal_srm_mkdir_recG] ");
    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (easy != NULL) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "   [gfal_srm_mkdir_recG] try to create directory %s", surl);
        struct stat st;

        ret = gfal_statG_srmv2_internal(easy->srm_context, &st, NULL, easy->path, &tmp_err);
        if (ret == 0) {
            if (!S_ISDIR(st.st_mode)) {
                gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), ENOTDIR, __func__, "%s it is a file", surl);
                ret = -1;
            }
            else {
                ret = 0;
            }
        }
        else {
            g_clear_error(&tmp_err);
            ret = gfal_mkdir_srmv2_internal(easy->srm_context, easy->path, mode, &tmp_err);
            if (ret < 0 && tmp_err->code == EEXIST) {
                ret = 0;
                g_clear_error(&tmp_err);
            }
        }
    }
    gfal_srm_ifce_easy_context_release(opts, easy);
    gfal2_log(G_LOG_LEVEL_DEBUG, "   [gfal_srm_mkdir_recG] <-");
    G_RETURN_ERR(ret, tmp_err, err);
}


int gfal_srm_mkdirG(plugin_handle ch, const char* surl, mode_t mode, gboolean pflag, GError** err)
{
    g_return_val_err_if_fail(ch && surl, EINVAL, err, "[gfal_srm_mkdirG] Invalid value handle and/or surl");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    int ret = -1;

    if(pflag) { // pflag set : behavior similar to mkdir -p requested
        ret = gfal_srm_mkdir_recG(ch, surl, mode, &tmp_err);
    }
    else {
        gfal2_log(G_LOG_LEVEL_DEBUG, "  ->  [gfal_srm_mkdirG] ");
        gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
        if (easy != NULL) {
            gfal2_log(G_LOG_LEVEL_DEBUG, "   [gfal_srm_mkdirG] try to create directory %s", surl);
            struct stat st;

            ret = gfal_statG_srmv2_internal(easy->srm_context, &st, NULL, easy->path, &tmp_err);
            if (ret == 0) {
                gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EEXIST, __func__, "directory already exist");
                ret = -1;
            }
            else {
                g_clear_error(&tmp_err);
                ret = gfal_mkdir_srmv2_internal(easy->srm_context, easy->path, mode, &tmp_err);
            }
        }
        gfal_srm_ifce_easy_context_release(opts, easy);
        gfal2_log(G_LOG_LEVEL_DEBUG, "   [gfal_srm_mkdirG] <-");
    }

    G_RETURN_ERR(ret, tmp_err, err);
}
