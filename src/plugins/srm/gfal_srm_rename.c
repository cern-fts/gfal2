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
#include "gfal_srm_namespace.h"
#include <common/gfal_common_internal.h>
#include <common/gfal_common_err_helpers.h>
#include <common/gfal_common_plugin.h>
#include <dirent.h>


static int gfal_srm_rename_internal_srmv2(srm_context_t context,
        const char* src, const char* dst, GError** err)
{
    GError*       tmp_err = NULL;
    int           ret = -1;
    struct srm_mv_input input;

    input.from = (char*)src;
    input.to   = (char*)dst;

    ret = gfal_srm_external_call.srm_mv(context, &input);

    if (ret != 0) {
        gfal_srm_report_error(context->errbuf, &tmp_err);
        ret = -1;
    }

    G_RETURN_ERR(ret, tmp_err, err);
}


int gfal_srm_renameG(plugin_handle plugin_data, const char* oldurl,
                     const char* urlnew, GError** err)
{
    g_return_val_err_if_fail(plugin_data && oldurl && urlnew, EINVAL, err, "[gfal_srm_renameG] Invalid value handle and/or surl");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) plugin_data;

    int ret = -1;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, oldurl, &tmp_err);
    if (context != NULL) {
        gfal_srm_cache_stat_remove(plugin_data, oldurl);
        ret = gfal_srm_rename_internal_srmv2(context, oldurl, urlnew, &tmp_err);
    }

    if (ret != 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}
