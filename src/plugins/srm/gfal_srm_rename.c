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
 * @file gfal_srm_rename.c
 * @brief file for the rename function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 24/0/2013
 * */
#include "gfal_srm.h"
#include "gfal_srm_endpoint.h"
#include "gfal_srm_internal_ls.h"
#include "gfal_srm_rename.h"
#include <common/gfal_common_internal.h>
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_plugin.h>
#include <dirent.h>


int gfal_srm_rename_internal_srmv2(gfal_srmv2_opt* opts, char* endpoint,
                                   const char* src, const char* dst, GError** err)
{
    GError*       tmp_err = NULL;
    char          errbuf[GFAL_ERRMSG_LEN]={0};
    int           ret = -1;
    srm_context_t context;

    context = gfal_srm_ifce_context_setup(opts->handle, endpoint, errbuf, GFAL_ERRMSG_LEN, &tmp_err);
    if (context != NULL) {
        struct srm_mv_input input;

        input.from = (char*)src;
        input.to   = (char*)dst;

        ret = gfal_srm_external_call.srm_mv(context, &input);

        if (ret != 0) {
            gfal_srm_report_error(errbuf, &tmp_err);
            ret = -1;
        }

        gfal_srm_ifce_context_release(context);
    }
    G_RETURN_ERR(ret, tmp_err, err);
}



int gfal_srm_rename_internal(gfal_srmv2_opt* opts, const char* src,
                             const char *dst, GError** err)
{
    GError* tmp_err = NULL;
    int ret = -1;

    char full_endpoint[GFAL_URL_MAX_LEN];
    enum gfal_srm_proto srm_types;

    if ((gfal_srm_determine_endpoint(opts, src, full_endpoint,
                                     GFAL_URL_MAX_LEN, &srm_types, &tmp_err)) == 0) {
        gfal_log(GFAL_VERBOSE_NORMAL, "gfal_srm_rm_internal -> endpoint %s",
                full_endpoint);

        if (srm_types == PROTO_SRMv2) {
            ret = gfal_srm_rename_internal_srmv2(opts, full_endpoint, src, dst,
                                                 &tmp_err);
        }
        else if (srm_types == PROTO_SRM) {
            g_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT,
                       "support for SRMv1 is removed in gfal 2.0, failure");
        }
        else {
            g_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT,
                        "Unknown SRM protocol, failure ");
        }
    }

    if (tmp_err)
        g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
    return ret;
}



int gfal_srm_renameG(plugin_handle plugin_data, const char* oldurl,
                     const char* urlnew, GError** err)
{
    g_return_val_err_if_fail(plugin_data && oldurl && urlnew,
                             -1, err, "[gfal_srm_renameG] Incorrect args");

    GError*         tmp_err = NULL;
    gfal_srmv2_opt* opts    = (gfal_srmv2_opt*)plugin_data;

    // If we rename, it doesn't make sense to keep cached the entry
    gfal_srm_cache_stat_remove(plugin_data, oldurl);

    int ret = gfal_srm_rename_internal(opts, oldurl, urlnew, &tmp_err);
    if(tmp_err)
        g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
    return ret;
}
