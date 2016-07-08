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

#include <string.h>
#include "gfal_srm_namespace.h"
#include "gfal_srm_internal_layer.h"


static int gfal_access_srmv2_internal(srm_context_t context, const char *surl, int mode, GError **err)
{
    GError *tmp_err = NULL;
    struct srm_checkpermission_input checkpermission_input;
    struct srmv2_filestatus *resu;
    int ret = -1;
    char *tab_surl[] = {(char *) surl, NULL};

    checkpermission_input.nbfiles = 1;
    checkpermission_input.amode = mode;
    checkpermission_input.surls = tab_surl;

    ret = gfal_srm_external_call.srm_check_permission(context, &checkpermission_input, &resu);
    if (ret != 1) {
        gfal_srm_report_error(context->errbuf, &tmp_err);
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }

    if (resu[0].status) {
        if (strnlen(resu[0].surl, GFAL_URL_MAX_LEN) >= GFAL_URL_MAX_LEN ||
            strnlen(resu[0].explanation, GFAL_URL_MAX_LEN) >= GFAL_URL_MAX_LEN) {
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(),
                resu[0].status, __func__, "Memory corruption in the libgfal_srm_ifce answer, fatal");
        }
        else {
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(),
                resu[0].status, __func__,
                "Error %d : %s , file %s: %s", resu[0].status,
                strerror(resu[0].status), resu[0].surl, resu[0].explanation);
        }
        ret = -1;
    }
        // resu[0].status == 0 is success
    else {
        ret = 0;
        errno = 0;
    }

    gfal_srm_external_call.srm_srmv2_filestatus_delete(resu, 1);

    G_RETURN_ERR(ret, tmp_err, err);
}


/*
 * @brief access method for SRMv2
 * check the right for a given SRM url, work only for SRMv2, V1 deprecated.
 * @param ch the handle of the plugin
 * @param surl srm url of a given file
 * @param mode access mode to check
 * @param err : GError error reprot system
 * @warning : not safe, surl must be verified
 */
int gfal_srm_accessG(plugin_handle ch, const char *surl, int mode, GError **err)
{
    g_return_val_err_if_fail(ch && surl, EINVAL, err, "[gfal_srm_accessG] Invalid value handle and/or surl");
    GError *tmp_err = NULL;
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;

    int ret = -1;

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (easy != NULL) {
        ret = gfal_access_srmv2_internal(easy->srm_context, easy->path, mode, &tmp_err);
    }
    gfal_srm_ifce_easy_context_release(opts, easy);

    if (ret != 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}
