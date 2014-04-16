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
 * @brief file for the gfal's plugin unlink function
 * @author Devresse Adrien
 * @version 2.0
 * @date 12/06/2011
 * */

#include <regex.h>
#include <time.h> 

#include "gfal_srm.h"
#include "gfal_srm_internal_layer.h" 
#include "gfal_srm_internal_ls.h"
#include <common/gfal_common_internal.h>
#include <common/gfal_common_err_helpers.h>
#include <common/gfal_common_plugin.h>
#include "gfal_srm_endpoint.h"


static int gfal_srm_rm_srmv2_isdir(srm_context_t context,
        const char* full_endpoint, const char* surl)
{
    struct srm_ls_input input;
    struct srm_ls_output output;

    input.nbfiles = 1;
    input.surls = (char**)(&surl);
    input.numlevels = 0;
    input.offset = 0;
    input.count = 0;

    if (gfal_srm_external_call.srm_ls(context, &input, &output) < 0)
        return 0;

    int isdir = S_ISDIR(output.statuses->stat.st_mode);

    gfal_srm_external_call.srm_srmv2_mdfilestatus_delete(output.statuses, 1);
    gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);

    return isdir;
}


static int gfal_srm_rm_srmv2_internal(gfal_srmv2_opt* opts,
        const char* full_endpoint, const char* surl, GError** err)
{
    GError* tmp_err = NULL;
    srm_context_t context;
    struct srm_rm_input input;
    struct srm_rm_output output;
    char errbuf[GFAL_ERRMSG_LEN] = { 0 };
    int ret = -1;

    if ((context = gfal_srm_ifce_context_setup(opts->handle, full_endpoint,
            errbuf, GFAL_ERRMSG_LEN, &tmp_err)) != NULL ) {
        input.nbfiles = 1;
        input.surls = (char**)(&surl);

        ret = gfal_srm_external_call.srm_rm(context, &input, &output);

        if (ret == 1) {
            ret = 0;
            struct srmv2_filestatus* statuses = output.statuses;
            if (statuses[0].status != 0) {
                if (statuses[0].explanation != NULL ) {
                    // DPM returns an EINVAL when srm_rm is called over a directory
                    // Check if the file is actually a directory, and override the return
                    // code with EISDIR in that case
                    int err_code = statuses[0].status;
                    if (err_code == EINVAL &&
                        gfal_srm_rm_srmv2_isdir(context, full_endpoint, surl)) {
                        err_code = EISDIR;
                    }
                    gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(),
                                err_code, __func__,
                                "error reported from srm_ifce, %s ",
                                statuses[0].explanation);
                }
                else {
                    gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(),
                                EINVAL, __func__,
                               "error reported from srm_ifce with corrputed memory ! ");
                }
                ret = -1;
            }

            gfal_srm_external_call.srm_srm2__TReturnStatus_delete(
                    output.retstatus);
            gfal_srm_external_call.srm_srmv2_filestatus_delete(output.statuses,
                    ret);
        }
        else {
            gfal_srm_report_error(errbuf, &tmp_err);
            ret = -1;
        }
        gfal_srm_ifce_context_release(context);
    }
    G_RETURN_ERR(ret, tmp_err, err);
}


static int gfal_srm_rm_internal(gfal_srmv2_opt* opts, const char* surl, GError** err)
{
    GError* tmp_err = NULL;
    int ret = -1;

    char full_endpoint[GFAL_URL_MAX_LEN];
    enum gfal_srm_proto srm_types;
    if ((gfal_srm_determine_endpoint(opts, surl, full_endpoint,
            GFAL_URL_MAX_LEN, &srm_types, &tmp_err)) == 0) { // check & get endpoint
        gfal_log(GFAL_VERBOSE_NORMAL, "gfal_srm_rm_internal -> endpoint %s",
                full_endpoint);

        if (srm_types == PROTO_SRMv2) {
            ret = gfal_srm_rm_srmv2_internal(opts, full_endpoint, surl,
                    &tmp_err);
        }
        else if (srm_types == PROTO_SRM) {
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT, __func__,
                    "support for SRMv1 is removed in gfal 2.0, failure");
        }
        else {
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT, __func__,
                    "Unknow SRM protocol, failure ");
        }
    }

    if (tmp_err)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    return ret;
}

/**
 * 
 * bindings of the unlink plugin call
 */
int gfal_srm_unlinkG(plugin_handle ch, const char * path, GError** err)
{
    g_return_val_err_if_fail( ch && path, -1, err,
            "[gfal_srm_unlinkG] incorrects args");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    gfal_srm_cache_stat_remove(ch, path);
    int ret = gfal_srm_rm_internal(opts, path, &tmp_err);
    if (tmp_err)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    return ret;
}
