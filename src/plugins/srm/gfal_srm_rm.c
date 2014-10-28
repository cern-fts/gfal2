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
#include "gfal_srm_endpoint.h"


static int gfal_srm_rm_srmv2_isdir(srm_context_t context, const char* surl)
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


static int gfal_srm_rm_srmv2_internal(srm_context_t context, int nbfiles, const char* const * surls,
        GError** errors)
{
    struct srm_rm_input input;
    struct srm_rm_output output;
    int ret = -1, i;

    input.nbfiles = nbfiles;
    input.surls = (char**)(surls);

    ret = gfal_srm_external_call.srm_rm(context, &input, &output);

    if (ret == nbfiles) {
        ret = 0;
        struct srmv2_filestatus* statuses = output.statuses;

        for (i = 0; i < nbfiles; ++i) {
            int err_code = statuses[i].status;
            if (err_code != 0) {
                ret -= 1;
                // DPM returns an EINVAL when srm_rm is called over a directory
                // Check if the file is actually a directory, and override the return
                // code with EISDIR in that case
                if (err_code == EINVAL && gfal_srm_rm_srmv2_isdir(context, surls[i]))
                    err_code = EISDIR;

                if (statuses[i].explanation)
                    gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(), err_code, __func__,
                            "error reported from srm_ifce, %s", statuses[i].explanation);
                else
                    gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(), err_code, __func__,
                            "error reported from srm_ifce, without explanation!");
            }
        }

        gfal_srm_external_call.srm_srm2__TReturnStatus_delete(
                output.retstatus);
        gfal_srm_external_call.srm_srmv2_filestatus_delete(output.statuses,
                nbfiles);
    }
    else {
        gfal_srm_report_error(context->errbuf, &errors[0]);
        for (i = 1; i < nbfiles; ++i)
            errors[i] = g_error_copy(errors[0]);
        ret = -1;
    }

    return ret;
}


/**
 *
 * bindings of the unlink plugin call
 */
int gfal_srm_unlink_listG(plugin_handle ch, int nbfiles, const char* const* paths, GError** err)
{
    GError *tmp_err = NULL;
    int ret = -1, i;

    if (!err)
        return -1;

    if (!ch || nbfiles < 0 || paths == NULL || *paths == NULL) {
        gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EINVAL, __func__, "incorrect args");
    }
    else {
        gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;
        srm_context_t context = gfal_srm_ifce_easy_context(opts, paths[0], &tmp_err);
        if (context) {
            for(i = 0; i < nbfiles; ++i)
                gfal_srm_cache_stat_remove(ch, paths[i]);

            ret = gfal_srm_rm_srmv2_internal(context, nbfiles, paths, err);
        }
    }

    if (tmp_err) {
        for (i = 1; i < nbfiles; ++i)
            err[i] = g_error_copy(err[0]);
    }
    return ret;
}


int gfal_srm_unlinkG(plugin_handle ch, const char * path, GError** err)
{
    int ret;
    GError *tmp_err = NULL;
    const char* paths[1] = {path};

    ret = gfal_srm_unlink_listG(ch, 1, paths, &tmp_err);

    if (ret != 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}
