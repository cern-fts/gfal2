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

#include <regex.h>
#include <time.h>

#include "gfal_srm.h"
#include "gfal_srm_endpoint.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_request.h"



static int gfal_srmv2_bring_online_internal(srm_context_t context, gfal_srmv2_opt* opts,
        int nbfiles, const char* const* surl, time_t pintime, time_t timeout,
        char* token, size_t tsize, int async, GError** errors)
{
    struct srm_bringonline_input  input;
    struct srm_bringonline_output output;
    gfal_srm_params_t             params = gfal_srm_params_new(opts);
    int i;

    memset(&output, 0, sizeof(output));

    srm_set_desired_request_time(context, timeout);

    input.nbfiles        = nbfiles;
    input.surls          = (char**)surl;
    input.desiredpintime = pintime;
    input.protocols      = gfal_srm_params_get_protocols(params);
    input.spacetokendesc = gfal_srm_params_get_spacetoken(params);

    if (input.spacetokendesc)
        gfal2_log(G_LOG_LEVEL_DEBUG, "Bringonline with spacetoken %s", input.spacetokendesc);

    int ret;
    if (async)
        ret = gfal_srm_external_call.srm_bring_online_async(context, &input, &output);
    else
        ret = gfal_srm_external_call.srm_bring_online(context, &input, &output);


    if (ret < 0) {
        GError *tmp_err = NULL;
        gfal_srm_report_error(context->errbuf, &tmp_err);
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }

    if (output.token)
        g_strlcpy(token, output.token, tsize);
    else
        token[0] = '\0';

    gfal2_log(G_LOG_LEVEL_MESSAGE, "Got BRINGONLINE token %s", token);

    int nterminal = 0;
    for (i = 0; i < nbfiles; ++i) {
        switch (output.filestatuses[i].status) {
            case 0:
                ++nterminal;
                break;
            case EAGAIN:
                break;
            default:
                gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(),
                            output.filestatuses[i].status, __func__,
                            "error on the bring online request: %s ",
                            output.filestatuses[i].explanation);
                ++nterminal;
                break;
        }
    }
    gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(output.filestatuses, ret);
    gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);
    free(output.token);

    gfal_srm_params_free(params);

    // Return 1 if all are already terminal
    return nterminal == nbfiles;
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
    gfal_srm_ifce_easy_context_release(opts, context);

    if (tmp_err) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }
    return ret;
}


int gfal_srmv2_bring_online_listG(plugin_handle ch, int nbfiles, const char* const* surls,
        time_t pintime, time_t timeout, char* token, size_t tsize,
        int async, GError** errors)
{
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, *surls, &tmp_err);
    if (context == NULL) {
        int i;
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }

    int ret = gfal_srmv2_bring_online_internal(context, opts, nbfiles, surls,
                pintime, timeout, token, tsize, async, errors);
    gfal_srm_ifce_easy_context_release(opts, context);
    return ret;
}


static int gfal_srmv2_bring_online_poll_internal(srm_context_t context,
		int nbfiles, const char* const* surls, const char* token, GError ** errors)
{
    struct srm_bringonline_input  input;
    struct srm_bringonline_output output;
    int i;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.nbfiles = nbfiles;
    input.surls   = (char**)surls;
    output.token  = (char*)token;

    int ret = gfal_srm_external_call.srm_bring_online_status(context, &input, &output);
    if (ret < 0) {
        GError *tmp_err = NULL;
        gfal_srm_report_error(context->errbuf, &tmp_err);
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }

    int nterminal = 0;
    for (i = 0; i < nbfiles; ++i) {
        switch(output.filestatuses[i].status) {
            case 0:
                ++nterminal;
                break;
            case EAGAIN:
                gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(),
                                EAGAIN, __func__,
                                "still queued: %s ",
                                output.filestatuses[i].explanation);
                break;
            default:
                gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(),
                        output.filestatuses[i].status, __func__,
                        "error on the bring online request: %s ", output.filestatuses[i].explanation);
                ++nterminal;
        }
    }

    gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(output.filestatuses, ret);
    gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);

    // Return will be 1 if all files are terminal
    return nterminal == nbfiles;
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
    gfal_srm_ifce_easy_context_release(opts, context);

    if (tmp_err) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }

    return ret;
}



int gfal_srmv2_bring_online_poll_listG(plugin_handle ch, int nbfiles,
        const char* const * surls, const char* token, GError** errors)
{
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, *surls, &tmp_err);
    if (context == NULL) {
        int i;
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }

    int ret = gfal_srmv2_bring_online_poll_internal(context, nbfiles, surls, token, errors);
    gfal_srm_ifce_easy_context_release(opts, context);
    return ret;
}



static int gfal_srmv2_release_file_internal(srm_context_t context, gfal_srmv2_opt* opts,
        int nbfiles, const char* const* surl, const char* token, GError** errors)
{
    struct srm_releasefiles_input input;
    struct srmv2_filestatus      *statuses;
    int i;

    if (token)
        gfal2_log(G_LOG_LEVEL_INFO, "Release file with token %s", token);
    else
        gfal2_log(G_LOG_LEVEL_INFO, "Release file without token");

    // Perform
    input.nbfiles  = nbfiles;
    input.reqtoken = NULL;
    input.surls    = (char**)surl;
    if(token)
        input.reqtoken = (char*)token;

    int ret = gfal_srm_external_call.srm_release_files(context, &input, &statuses);

    if (ret < 0) {
        GError *tmp_err = NULL;
        gfal_srm_report_error(context->errbuf, &tmp_err);
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }

    for (i = 0; i < nbfiles; ++i) {
        if (statuses[i].status != 0) {
            gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(), statuses[i].status, __func__,
                    "error on the release request : %s ", statuses[0].explanation);
        }
    }
    gfal_srm_external_call.srm_srmv2_filestatus_delete(statuses, 1);
    return 0;
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
    gfal_srm_ifce_easy_context_release(opts, context);

    if (tmp_err) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }

    return ret;
}



int gfal_srmv2_release_file_listG(plugin_handle ch, int nbfiles, const char* const* surls,
        const char* token, GError** errors)
{
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, *surls, &tmp_err);
    if (context == NULL) {
        int i;
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }

    int ret = gfal_srmv2_release_file_internal(context, opts, nbfiles, surls, token, errors);
    gfal_srm_ifce_easy_context_release(opts, context);
    return ret;
}


static int gfal_srmv2_abort_files_internal(srm_context_t context, gfal_srmv2_opt* opts,
        int nbfiles, const char* const* surl, const char* token, GError** errors)
{
    struct srm_abort_files_input input;
    struct srmv2_filestatus      *statuses;
    GError                       *tmp_err = NULL;
    int i;

    if (token)
        gfal2_log(G_LOG_LEVEL_INFO, "Abort file with token %s", token);
    else
        gfal2_log(G_LOG_LEVEL_INFO, "Abort file without token");

    // Perform
    input.nbfiles  = nbfiles;
    input.reqtoken = NULL;
    input.surls    = (char**)surl;
    if(token)
        input.reqtoken = (char*)token;

    int ret = gfal_srm_external_call.srm_abort_files(context, &input, &statuses);

    if (ret < 0) {
        gfal_srm_report_error(context->errbuf, &tmp_err);
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
    }
    else {
        ret = 0;
        for (i = 0; i < nbfiles; ++i) {
            if (statuses[i].status != 0) {
                gfal2_set_error(&(errors[i]), gfal2_get_plugin_srm_quark(),
                        statuses[i].status, __func__,
                        "error on the abort request : %s ", statuses[i].explanation);
                ret -= 1;
            }
        }
        gfal_srm_external_call.srm_srmv2_filestatus_delete(statuses, 1);
    }

    return ret;
}

int gfal_srm2_abort_filesG(plugin_handle ch, int nbfiles, const char* const* surls, const char* token, GError ** errors)
{
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, *surls, &tmp_err);
    if (context == NULL) {
        int i;
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }

    int ret = gfal_srmv2_abort_files_internal(context, opts, nbfiles, surls, token, errors);
    gfal_srm_ifce_easy_context_release(opts, context);
    return ret;
}
