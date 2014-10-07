/*
* Copyright @ Members of the EMI Collaboration, 2013.
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
 * @file gfal_srm_bringonline.c
 * @brief brings online functions layer from srm
 * @author Devresse Adrien, Alejandro Álvarez Ayllón
 * @version 2.0
 * @date 19/12/2011
 * */



#include <common/gfal_common_internal.h>
#include <common/gfal_common_err_helpers.h>
#include <common/gfal_common_plugin.h>
#include <regex.h>
#include <time.h>

#include "gfal_srm.h"
#include "gfal_srm_endpoint.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_request.h"



static int gfal_srmv2_bring_online_internal(srm_context_t context, gfal_srmv2_opt* opts,
        int nbfiles, const char* const* surl, time_t pintime, time_t timeout,
        char* token, size_t tsize, int async, GError** err)
{
    struct srm_bringonline_input  input;
    struct srm_bringonline_output output;
    GError                       *tmp_err = NULL;
    gfal_srm_params_t             params = gfal_srm_params_new(opts, &tmp_err);
    int                           status = 0;

    memset(&output, 0, sizeof(output));

    if (params != NULL) {
        context->timeout      = timeout;
        context->timeout_conn = timeout;
        context->timeout_ops  = timeout;

        input.nbfiles        = nbfiles;
        input.surls          = (char**)surl;
        input.desiredpintime = pintime;
        input.protocols      = gfal_srm_params_get_protocols(params);
        input.spacetokendesc = gfal_srm_params_get_spacetoken(params);

        if (input.spacetokendesc)
            gfal_log(GFAL_VERBOSE_DEBUG, "Bringonline with spacetoken %s", input.spacetokendesc);

        int ret = 0;

        if (async)
            ret = gfal_srm_external_call.srm_bring_online_async(context, &input, &output);
        else
            ret = gfal_srm_external_call.srm_bring_online(context, &input, &output);

        if (ret < 0) {
            gfal_srm_report_error(context->errbuf, &tmp_err);
        }
        else {
            status = output.filestatuses[0].status;
            switch (status) {
                case 0:
                case EAGAIN:
                    if (output.token)
                        g_strlcpy(token, output.token, tsize);
                    else
                        token[0] = '\0';
                    break;
                default:
                    gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(),
                                output.filestatuses[0].status, __func__,
                                "error on the bring online request : %s ",
                                output.filestatuses[0].explanation);
                    break;
            }
        }
        gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(output.filestatuses, ret);
        gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);
        free(output.token);
    }
    gfal_srm_params_free(params);

    if (tmp_err != NULL) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }
    else {
        // Return 1 if already pinned
        return status == 0;
    }
}



int gfal_srmv2_bring_onlineG(plugin_handle ch, const char* surl,
        time_t pintime, time_t timeout, char* token, size_t tsize,
        int async, GError** err)
{
    g_return_val_err_if_fail(ch && surl && token, EINVAL, err, "[gfal_srmv2_bring_onlineG] Invalid value handle and/or surl");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    int ret = -1;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (context != NULL) {
        ret = gfal_srmv2_bring_online_internal(context, opts, 1, &surl,
                    pintime, timeout, token, tsize, async, &tmp_err);
    }

    if (ret < 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}


int gfal_srmv2_bring_online_listG(plugin_handle ch, int nbfiles, const char* const* surls,
        time_t pintime, time_t timeout, char* token, size_t tsize,
        int async, GError** err)
{
    g_return_val_err_if_fail(ch && surls && *surls && token, EINVAL, err, "[gfal_srmv2_bring_onlineG] Invalid value handle and/or surl");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    int ret = -1;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, *surls, &tmp_err);
    if (context != NULL) {
        ret = gfal_srmv2_bring_online_internal(context, opts, nbfiles, surls,
                    pintime, timeout, token, tsize, async, &tmp_err);
    }

    if (ret < 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}



static int gfal_srmv2_bring_online_poll_internal(srm_context_t context,
		int nbfiles, const char* const* surls, const char* token, GError ** err)
{
    struct srm_bringonline_input  input;
    struct srm_bringonline_output output;
    GError                       *tmp_err = NULL;
    int                           status = 0;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.nbfiles = nbfiles;
    input.surls   = (char**)surls;
    output.token  = (char*)token;

    int ret = gfal_srm_external_call.srm_bring_online_status(context, &input, &output);
    if (ret < 0) {
        gfal_srm_report_error(context->errbuf, &tmp_err);
    }
    else {
        status = output.filestatuses[0].status;
        switch (status) {
            case 0:
            case EAGAIN:
                break;
            default:
                gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(),
                            output.filestatuses[0].status, __func__,
                            "error on the bring online request : %s ",
                            output.filestatuses[0].explanation);
                break;
        }
    }
    gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(output.filestatuses, ret);
    gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);

    if (tmp_err != NULL) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }
    else {
        // Return will be 1 if the file is already online
        return status == 0;
    }
}



int gfal_srmv2_bring_online_pollG(plugin_handle ch, const char* surl,
                                  const char* token, GError** err)
{
    g_return_val_err_if_fail(ch && surl && token, EINVAL, err, "[gfal_srmv2_bring_online_pollG] Invalid value handle and, surl or token");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    int ret = -1;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (context != NULL) {
        ret = gfal_srmv2_bring_online_poll_internal(context, 1, &surl, token, &tmp_err);
    }

    if (ret < 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}



int gfal_srmv2_bring_online_poll_listG(plugin_handle ch, int nbfiles, const char* const* surls,
                                  const char* token, GError** err)
{
    g_return_val_err_if_fail(ch && surls && *surls && token, EINVAL, err, "[gfal_srmv2_bring_online_pollG] Invalid value handle and, surl or token");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    int ret = -1;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, *surls, &tmp_err);
    if (context != NULL) {
        ret = gfal_srmv2_bring_online_poll_internal(context, nbfiles, surls, token, &tmp_err);
    }

    if (ret < 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}



static int gfal_srmv2_release_file_internal(srm_context_t context, gfal_srmv2_opt* opts,
        int nbfiles, const char* const* surl, const char* token, GError** err)
{
    struct srm_releasefiles_input input;
    struct srmv2_filestatus      *statuses;
    GError                       *tmp_err = NULL;
    gfal_srm_params_t             params = gfal_srm_params_new(opts, &tmp_err);

    if (params != NULL) {
          if (token)
              gfal_log(GFAL_VERBOSE_VERBOSE, "Release file with token %s", token);
          else
              gfal_log(GFAL_VERBOSE_VERBOSE, "Release file without token");

          // Perform
          input.nbfiles  = nbfiles;
          input.reqtoken = NULL;
          input.surls    = (char**)surl;
          if(token)
              input.reqtoken = (char*)token;

          int ret = gfal_srm_external_call.srm_release_files(context, &input, &statuses);

          if (ret < 0) {
              gfal_srm_report_error(context->errbuf, &tmp_err);
          }
          else {
              if (statuses[0].status != 0) {
                  gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(),
                              statuses[0].status, __func__,
                              "error on the release request : %s ",
                              statuses[0].explanation);
              }
              gfal_srm_external_call.srm_srmv2_filestatus_delete(statuses, 1);
          }
    }

    if (tmp_err != NULL) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }
    else {
        return 0;
    }
}



int gfal_srmv2_release_fileG(plugin_handle ch, const char* surl,
        const char* token, GError** err)
{
    g_return_val_err_if_fail(ch && surl && token, EINVAL, err, "[gfal_srmv2_release_fileG] Invalid value handle, surl or token");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    int ret = -1;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (context != NULL) {
        ret = gfal_srmv2_release_file_internal(context, opts, 1, &surl, token, &tmp_err);
    }

    if (ret < 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}



int gfal_srmv2_release_file_listG(plugin_handle ch, int nbfiles, const char* const* surls,
        const char* token, GError** err)
{
    g_return_val_err_if_fail(ch && surls && *surls && token, EINVAL, err, "[gfal_srmv2_release_fileG] Invalid value handle, surl or token");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    int ret = -1;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, *surls, &tmp_err);
    if (context != NULL) {
        ret = gfal_srmv2_release_file_internal(context, opts, nbfiles, surls, token, &tmp_err);
    }

    if (ret < 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}

static int gfal_srmv2_abort_files_internal(srm_context_t context, gfal_srmv2_opt* opts,
        int nbfiles, const char* const* surl, const char* token, GError** err)
{
    struct srm_abort_files_input input;
    struct srmv2_filestatus      *statuses;
    GError                       *tmp_err = NULL;
    gfal_srm_params_t             params = gfal_srm_params_new(opts, &tmp_err);

    if (params != NULL) {
          if (token)
              gfal_log(GFAL_VERBOSE_VERBOSE, "Abort file with token %s", token);
          else
              gfal_log(GFAL_VERBOSE_VERBOSE, "Abort file without token");

          // Perform
          input.nbfiles  = nbfiles;
          input.reqtoken = NULL;
          input.surls    = (char**)surl;
          if(token)
          input.reqtoken = (char*)token;

          int ret = gfal_srm_external_call.srm_abort_files(context, &input, &statuses);

          if (ret < 0) {
              gfal_srm_report_error(context->errbuf, &tmp_err);
          }
          else {
              if (statuses[0].status != 0) {
                  gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(),
                              statuses[0].status, __func__,
                              "error on the release request : %s ",
                              statuses[0].explanation);
              }
              gfal_srm_external_call.srm_srmv2_filestatus_delete(statuses, 1);
          }
    }

    if (tmp_err != NULL) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }
    else {
        return 0;
    }
}

int gfal_srm2_abort_filesG(plugin_handle ch, int nbfiles, const char* const* surls, const char* token, GError ** err)
{
    g_return_val_err_if_fail(ch && surls && *surls && token, EINVAL, err, "[gfal_srmv2_release_fileG] Invalid value handle, surl or token");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    int ret = -1;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, *surls, &tmp_err);
    if (context != NULL) {
        ret = gfal_srmv2_abort_files_internal(context, opts, nbfiles, surls, token, &tmp_err);
    }

    if (ret < 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}
