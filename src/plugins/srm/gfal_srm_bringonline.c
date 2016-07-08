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

#include <regex.h>

#include "gfal_srm.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_request.h"
#include "gfal_srm_url_check.h"


// Sadly, this is quite inefficient, but has to be done since there might be duplicated
// surls in the initial request (plus, the response order doesn't have to be the same
// as when requested)
static int gfal_srmv2_bring_online_internal_status_index(
    int nresponses,
    struct srm_bringonline_output *output,
    const char *surl)
{
    int i;
    for (i = 0; i < nresponses; ++i) {
        if (gfal2_srm_surl_cmp(output->filestatuses[i].surl, surl) == 0) {
            return i;
        }
    }
    return -1;
}


static int gfal_srmv2_bring_online_internal(srm_context_t context, gfal_srmv2_opt *opts,
    int nbfiles, const char *const *surl, time_t pintime, time_t timeout,
    char *token, size_t tsize, int async, GError **errors)
{
    struct srm_bringonline_input input;
    struct srm_bringonline_output output;
    gfal_srm_params_t params = gfal_srm_params_new(opts);
    int i;

    memset(&output, 0, sizeof(output));

    srm_set_desired_request_time(context, timeout);

    input.nbfiles = nbfiles;
    input.surls = (char **) surl;
    input.desiredpintime = pintime;
    input.protocols = gfal_srm_params_get_protocols(params);
    input.spacetokendesc = gfal_srm_params_get_spacetoken(params);

    if (input.spacetokendesc)
        gfal2_log(G_LOG_LEVEL_DEBUG, "Bringonline with spacetoken %s", input.spacetokendesc);

    int nresponses;
    if (async)
        nresponses = gfal_srm_external_call.srm_bring_online_async(context, &input, &output);
    else
        nresponses = gfal_srm_external_call.srm_bring_online(context, &input, &output);


    if (nresponses < 0) {
        GError *tmp_err = NULL;
        gfal_srm_report_error(context->errbuf, &tmp_err);
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }

    if (nresponses != nbfiles) {
        gfal2_log(G_LOG_LEVEL_DEBUG,
            "%d files in the request, %d in the response", nbfiles,
            nresponses);
    }

    if (output.token)
        g_strlcpy(token, output.token, tsize);
    else
        token[0] = '\0';

    gfal2_log(G_LOG_LEVEL_MESSAGE, "Got BRINGONLINE token %s", token);

    int nterminal = 0;
    for (i = 0; i < nbfiles; ++i) {
        int status_index = gfal_srmv2_bring_online_internal_status_index(nresponses, &output, surl[i]);
        if (status_index >= 0) {
            switch (output.filestatuses[status_index].status) {
                case 0:
                    ++nterminal;
                    break;
                case EAGAIN:
                    break;
                default:
                    gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(),
                        output.filestatuses[status_index].status, __func__,
                        "error on the bring online request: %s ",
                        output.filestatuses[status_index].explanation);
                    ++nterminal;
                    break;
            }
        } else {
            gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(),
                EPROTO, __func__, "missing surl on the response: %s", surl[i]);
            ++nterminal;
        }
    }
    gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(output.filestatuses, nresponses);
    gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);
    free(output.token);

    gfal_srm_params_free(params);

    // Return 1 if all are already terminal
    return nterminal == nbfiles;
}


int gfal_srmv2_bring_onlineG(plugin_handle ch, const char *surl,
    time_t pintime, time_t timeout, char *token, size_t tsize,
    int async, GError **err)
{
    g_return_val_err_if_fail(ch && surl && token, EINVAL, err,
        "[gfal_srmv2_bring_onlineG] Invalid value handle and/or surl");
    GError *tmp_err = NULL;
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;

    int ret = -1;

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (easy != NULL) {
        ret = gfal_srmv2_bring_online_internal(easy->srm_context, opts, 1, (const char *const *) &easy->path,
            pintime, timeout, token, tsize, async, &tmp_err);
    }
    gfal_srm_ifce_easy_context_release(opts, easy);

    if (tmp_err) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }
    return ret;
}


int gfal_srmv2_bring_online_listG(plugin_handle ch, int nbfiles, const char *const *surls,
    time_t pintime, time_t timeout, char *token, size_t tsize,
    int async, GError **errors)
{
    int i;
    GError *tmp_err = NULL;
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, *surls, &tmp_err);
    if (easy == NULL) {
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }

    char *decoded[nbfiles];
    for (i = 0; i < nbfiles; ++i) {
        decoded[i] = gfal2_srm_get_decoded_path(surls[i]);
    }

    int ret = gfal_srmv2_bring_online_internal(easy->srm_context, opts, nbfiles, (const char *const *) decoded,
        pintime, timeout, token, tsize, async, errors);
    gfal_srm_ifce_easy_context_release(opts, easy);

    for (i = 0; i < nbfiles; ++i) {
        g_free(decoded[i]);
    }

    return ret;
}


static int gfal_srmv2_bring_online_poll_internal(srm_context_t context,
    int nbfiles, const char *const *surls, const char *token, GError **errors)
{
    struct srm_bringonline_input input;
    struct srm_bringonline_output output;
    int i;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.nbfiles = nbfiles;
    input.surls = (char **) surls;
    output.token = (char *) token;

    int nresponses = gfal_srm_external_call.srm_bring_online_status(context, &input, &output);
    if (nresponses < 0) {
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
        int status_index = gfal_srmv2_bring_online_internal_status_index(nresponses, &output, surls[i]);
        if (status_index >= 0) {
            switch (output.filestatuses[status_index].status) {
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
                        output.filestatuses[status_index].status, __func__,
                        "error on the bring online request: %s ",
                        output.filestatuses[status_index].explanation);
                    ++nterminal;
                    break;
            }
        } else {
            gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(),
                EPROTO, __func__, "missing surl on the response: %s", surls[i]);
            ++nterminal;
        }
    }

    gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(output.filestatuses, nresponses);
    gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);

    // Return will be 1 if all files are terminal
    return nterminal == nbfiles;
}


int gfal_srmv2_bring_online_pollG(plugin_handle ch, const char *surl,
    const char *token, GError **err)
{
    g_return_val_err_if_fail(ch && surl && token, EINVAL, err,
        "[gfal_srmv2_bring_online_pollG] Invalid value handle and, surl or token");
    GError *tmp_err = NULL;
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;

    int ret = -1;

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (easy != NULL) {
        ret = gfal_srmv2_bring_online_poll_internal(easy->srm_context, 1, (const char *const *) &easy->path, token,
            &tmp_err);
    }
    gfal_srm_ifce_easy_context_release(opts, easy);

    if (tmp_err) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }

    return ret;
}


int gfal_srmv2_bring_online_poll_listG(plugin_handle ch, int nbfiles,
    const char *const *surls, const char *token, GError **errors)
{
    int i;
    GError *tmp_err = NULL;
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, *surls, &tmp_err);
    if (easy == NULL) {
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }

    char *decoded[nbfiles];
    for (i = 0; i < nbfiles; ++i) {
        decoded[i] = gfal2_srm_get_decoded_path(surls[i]);
    }

    int ret = gfal_srmv2_bring_online_poll_internal(easy->srm_context, nbfiles, (const char *const *) decoded,
        token, errors);
    gfal_srm_ifce_easy_context_release(opts, easy);

    for (i = 0; i < nbfiles; ++i) {
        g_free(decoded[i]);
    }

    return ret;
}


static int gfal_srmv2_release_file_internal(srm_context_t context, gfal_srmv2_opt *opts,
    int nbfiles, const char *const *surl, const char *token, GError **errors)
{
    struct srm_releasefiles_input input;
    struct srmv2_filestatus *statuses;
    int i;

    if (token)
        gfal2_log(G_LOG_LEVEL_MESSAGE, "Released file with token %s", token);
    else
        gfal2_log(G_LOG_LEVEL_MESSAGE, "Released file without token");

    // Perform
    input.nbfiles = nbfiles;
    input.reqtoken = NULL;
    input.surls = (char **) surl;
    if (token)
        input.reqtoken = (char *) token;

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


int gfal_srmv2_release_fileG(plugin_handle ch, const char *surl,
    const char *token, GError **err)
{
    g_return_val_err_if_fail(ch && surl && token, EINVAL, err,
        "[gfal_srmv2_release_fileG] Invalid value handle, surl or token");
    GError *tmp_err = NULL;
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;

    int ret = -1;

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (easy != NULL) {
        ret = gfal_srmv2_release_file_internal(easy->srm_context, opts, 1, (const char *const *) &easy->path, token,
            &tmp_err);
    }
    gfal_srm_ifce_easy_context_release(opts, easy);

    if (tmp_err) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }

    return ret;
}


int gfal_srmv2_release_file_listG(plugin_handle ch, int nbfiles, const char *const *surls,
    const char *token, GError **errors)
{
    int i;
    GError *tmp_err = NULL;
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surls[0], &tmp_err);
    if (easy == NULL) {
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }

    char *decoded[nbfiles];
    for (i = 0; i < nbfiles; ++i) {
        decoded[i] = gfal2_srm_get_decoded_path(surls[i]);
    }

    int ret = gfal_srmv2_release_file_internal(easy->srm_context, opts, nbfiles, (const char *const *) decoded,
        token, errors);
    gfal_srm_ifce_easy_context_release(opts, easy);

    for (i = 0; i < nbfiles; ++i) {
        g_free(decoded[i]);
    }

    return ret;
}


static int gfal_srmv2_abort_files_internal(srm_context_t context, gfal_srmv2_opt *opts,
    int nbfiles, const char *const *surl, const char *token, GError **errors)
{
    struct srm_abort_files_input input;
    struct srmv2_filestatus *statuses;
    GError *tmp_err = NULL;
    int i;

    if (token)
        gfal2_log(G_LOG_LEVEL_MESSAGE, "Abort file with token %s", token);
    else
        gfal2_log(G_LOG_LEVEL_MESSAGE, "Abort file without token");

    // Perform
    input.nbfiles = nbfiles;
    input.reqtoken = NULL;
    input.surls = (char **) surl;
    if (token)
        input.reqtoken = (char *) token;

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

int gfal_srm2_abort_filesG(plugin_handle ch, int nbfiles, const char *const *surls, const char *token, GError **errors)
{
    int i;

    GError *tmp_err = NULL;
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surls[0], &tmp_err);
    if (easy == NULL) {
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }

    char *decoded[nbfiles];
    for (i = 0; i < nbfiles; ++i) {
        decoded[i] = gfal2_srm_get_decoded_path(surls[i]);
    }

    int ret = gfal_srmv2_abort_files_internal(easy->srm_context, opts, nbfiles, (const char *const *) decoded,
        token, errors);
    gfal_srm_ifce_easy_context_release(opts, easy);
    for (i = 0; i < nbfiles; ++i) {
        g_free(decoded[i]);
    }

    return ret;
}
