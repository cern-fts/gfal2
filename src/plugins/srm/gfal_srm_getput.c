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

#include <regex.h>
#include <time.h>
#include <stdio.h>

#include "gfal_srm.h"
#include "gfal_srm_request.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_endpoint.h"
#include "gfal_srm_getput.h"


// Make sure the TURL returned by the endpoint is one of the requested protocols
static int validate_turls(int n_results, gfal_srm_result **resu,
    gfal_srm_params_t params, GError **tmp_err)
{
    int failed = 0;
    int n_protocols = g_strv_length(params->protocols);
    int i, j;

    for (i = 0; i < n_results && !failed; ++i) {
        const char *turl = (*resu)[i].turl;

        if (turl[0] == '/') {
            failed = -1;
            gfal2_set_error(tmp_err, gfal2_get_plugin_srm_quark(), EBADMSG, __func__,
                "A turl can not start with /");
            break;
        }

        // If error is set, skip the check
        if ((*resu)[i].err_code != 0)
            continue;

        // Check the turl protocol is in the request list
        int matching_protocol = 0;
        for (j = 0; j < n_protocols; ++j) {
            size_t proto_len = strlen(params->protocols[j]);
            if (strncmp(params->protocols[j], turl, proto_len) == 0 && turl[proto_len] == ':') {
                matching_protocol = 1;
                break;
            }
        }

        // If no matching protocol, fail already
        if (!matching_protocol) {
            failed = -1;
            gfal2_set_error(tmp_err, gfal2_get_plugin_srm_quark(), EBADMSG, __func__,
                "The SRM endpoint returned a protocol that wasn't requested: %s",
                turl);
        }
    }
    // Didn't match, so free and set an error
    if (failed) {
        free(*resu);
        *resu = NULL;
    }
    return failed;
}


static int gfal_srm_convert_filestatuses_to_srm_result(struct srmv2_pinfilestatus *statuses, char *reqtoken, int n,
    gfal_srm_result **resu, GError **err)
{
    g_return_val_err_if_fail(statuses && n && resu, -1, err,
        "[gfal_srm_convert_filestatuses_to_srm_result] args invalids");
    *resu = calloc(n, sizeof(gfal_srm_result));
    int i = 0;
    for (i = 0; i < n; ++i) {
        if (statuses[i].turl)
            g_strlcpy((*resu)[i].turl, statuses[i].turl, GFAL_URL_MAX_LEN);
        if (statuses[i].explanation)
            g_strlcpy((*resu)[i].err_str, statuses[i].explanation, GFAL_URL_MAX_LEN);
        (*resu)[i].err_code = statuses[i].status;
        (*resu)[i].reqtoken = g_strdup(reqtoken);
    }
    return 0;
}


int gfal_srmv2_get_global(gfal_srmv2_opt *opts, gfal_srm_params_t params, srm_context_t context,
    struct srm_preparetoget_input *input, gfal_srm_result **resu, GError **err)
{
    g_return_val_err_if_fail(opts != NULL && input != NULL && resu != NULL, -1, err,
        "[gfal_srmv2_get_global] tab null ");

    GError *tmp_err = NULL;
    int ret = 0;
    struct srm_preparetoget_output preparetoget_output;

    memset(&preparetoget_output, 0, sizeof(preparetoget_output));

    ret = gfal_srm_external_call.srm_prepare_to_get(context, input, &preparetoget_output);
    if (ret < 0) {
        gfal_srm_report_error(context->errbuf, &tmp_err);
    } else {
        gfal2_log(G_LOG_LEVEL_MESSAGE, "Got GET token for %s: %s", input->surls[0], preparetoget_output.token);
        gfal_srm_convert_filestatuses_to_srm_result(preparetoget_output.filestatuses, preparetoget_output.token, ret,
            resu, &tmp_err);
    }

    if (preparetoget_output.filestatuses != NULL)
        gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(preparetoget_output.filestatuses, ret);
    if (preparetoget_output.retstatus != NULL)
        gfal_srm_external_call.srm_srm2__TReturnStatus_delete(preparetoget_output.retstatus);
    free(preparetoget_output.token);

    G_RETURN_ERR(ret, tmp_err, err);
}


int gfal_srmv2_put_global(gfal_srmv2_opt *opts, gfal_srm_params_t params, srm_context_t context,
    struct srm_preparetoput_input *input, gfal_srm_result **resu, GError **err)
{
    g_return_val_err_if_fail(opts != NULL && input != NULL && resu != NULL, -1, err,
        "[gfal_srmv2_put_global] tab null ");

    GError *tmp_err = NULL;
    int ret = 0;
    struct srm_preparetoput_output preparetoput_output;

    memset(&preparetoput_output, 0, sizeof(preparetoput_output));

    ret = gfal_srm_external_call.srm_prepare_to_put(context, input, &preparetoput_output);
    if (ret < 0) {
        gfal_srm_report_error(context->errbuf, &tmp_err);
    }
    else {
        gfal2_log(G_LOG_LEVEL_MESSAGE, "Got PUT token for %s: %s", input->surls[0], preparetoput_output.token);
        gfal_srm_convert_filestatuses_to_srm_result(preparetoput_output.filestatuses, preparetoput_output.token, ret,
            resu, &tmp_err);
    }

    if (preparetoput_output.filestatuses != NULL)
        gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(preparetoput_output.filestatuses, ret);
    if (preparetoput_output.retstatus != NULL)
        gfal_srm_external_call.srm_srm2__TReturnStatus_delete(preparetoput_output.retstatus);
    free(preparetoput_output.token);

    G_RETURN_ERR(ret, tmp_err, err);
}


//  @brief execute a srmv2 request sync "GET" on the srm_ifce
int gfal_srm_getTURLS_srmv2_internal(srm_context_t context, gfal_srmv2_opt *opts,
    gfal_srm_params_t params, char *surl, gfal_srm_result **resu, GError **err)
{
    g_return_val_err_if_fail(surl != NULL, -1, err, "[gfal_srmv2_getasync] tab null");

    GError *tmp_err = NULL;
    int ret = -1;
    struct srm_preparetoget_input preparetoget_input;

    // set the structures datafields
    preparetoget_input.desiredpintime = 0;
    preparetoget_input.nbfiles = 1;
    preparetoget_input.protocols = gfal_srm_params_get_protocols(params);
    preparetoget_input.spacetokendesc = gfal_srm_params_get_spacetoken(params);
    preparetoget_input.surls = &surl;

    ret = gfal_srmv2_get_global(opts, params, context, &preparetoget_input, resu, &tmp_err);
    G_RETURN_ERR(ret, tmp_err, err);
}


// execute a srmv2 request sync "PUT" on the srm_ifce
int gfal_srm_putTURLS_srmv2_internal(srm_context_t context, gfal_srmv2_opt *opts, gfal_srm_params_t params,
    char *surl, gfal_srm_result **resu, GError **err)
{
    g_return_val_err_if_fail(surl != NULL, -1, err, "[gfal_srm_putTURLS_srmv2_internal] GList passed null");

    GError *tmp_err = NULL;
    int ret = -1;
    struct srm_preparetoput_input preparetoput_input;

    SRM_LONG64 filesize = params->file_size;

    // set the structures datafields
    preparetoput_input.desiredpintime = 0;
    preparetoput_input.nbfiles = 1;
    preparetoput_input.protocols = gfal_srm_params_get_protocols(params);
    preparetoput_input.spacetokendesc = gfal_srm_params_get_spacetoken(params);
    preparetoput_input.surls = &surl;
    preparetoput_input.filesizes = &filesize;

    ret = gfal_srmv2_put_global(opts, params, context, &preparetoput_input, resu, &tmp_err);

    G_RETURN_ERR(ret, tmp_err, err);
}


// Internal function of gfal_srm_getTurls without argument check for internal usage
static int gfal_srm_mTURLS_internal(gfal_srmv2_opt *opts, gfal_srm_params_t params,
    srm_req_type req_type, const char *surl, gfal_srm_result **resu,
    GError **err)
{
    GError *tmp_err = NULL;
    int ret = -1;

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (easy != NULL) {
        if (req_type == SRM_GET)
            ret = gfal_srm_getTURLS_srmv2_internal(easy->srm_context, opts, params, easy->path, resu, &tmp_err);
        else
            ret = gfal_srm_putTURLS_srmv2_internal(easy->srm_context, opts, params, easy->path, resu, &tmp_err);
    }
    gfal_srm_ifce_easy_context_release(opts, easy);

    if (ret < 0) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    }
    else {
        if (validate_turls(1, resu, params, &tmp_err)) {
            gfal2_propagate_prefixed_error(err, tmp_err, __func__);
            ret = -1;
        }
    }

    return ret;
}

//  simple wrapper to getTURLs for the gfal_module layer
int gfal_srm_getTURLS_plugin(plugin_handle ch, const char *surl, char *buff_turl, int size_turl, char **reqtoken,
    GError **err)
{
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;
    gfal_srm_result *resu = NULL;
    GError *tmp_err = NULL;
    int ret = -1;

    gfal_srm_params_t params = gfal_srm_params_new(opts);
    if (params != NULL) {
        ret = gfal_srm_mTURLS_internal(opts, params, SRM_GET, surl, &resu, &tmp_err);
        if (ret > 0) {
            if (resu->err_code == 0) {
                g_strlcpy(buff_turl, resu->turl, size_turl);
                if (reqtoken)
                    *reqtoken = resu->reqtoken;
                ret = 0;
            } else {
                gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), resu->err_code, __func__,
                    "error on the turl request : %s ", resu->err_str);
                ret = -1;
                g_free(resu->reqtoken);
            }
            free(resu);
        }
        gfal_srm_params_free(params);
    }
    G_RETURN_ERR(ret, tmp_err, err);
}


//  special call for TURL resolution for checksum fallback solution
int gfal_srm_getTURL_checksum(plugin_handle ch, const char *surl, char *buff_turl, int size_turl, GError **err)
{
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;
    gfal_srm_result *resu = NULL;
    char **sup_protocols = NULL;
    GError *tmp_err = NULL;
    int ret = -1;

    gfal_srm_params_t params = gfal_srm_params_new(opts);
    if (params != NULL) {
    	sup_protocols = srm_get_3rdparty_turls_sup_protocol(opts->handle);
        gfal_srm_params_set_protocols(params, sup_protocols);
        ret = gfal_srm_mTURLS_internal(opts, params, SRM_GET, surl, &resu, &tmp_err);
        if (ret > 0) {
            if (resu->err_code == 0) {
                g_strlcpy(buff_turl, resu->turl, size_turl);
                ret = 0;

            } else {
                gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), resu->err_code, __func__,
                    "error on the turl %s request : %s ", resu->turl, resu->err_str);
                ret = -1;
            }
            free(resu);
        }
        gfal_srm_params_free(params);
    }
    G_RETURN_ERR(ret, tmp_err, err);
}

//  execute a get for thirdparty transfer turl
int gfal_srm_get_rd3_turl(plugin_handle ch, gfalt_params_t p, const char *surl, const char *other_surl,
    char *buff_turl, int size_turl,
    char *reqtoken, size_t size_reqtoken,
    GError **err)
{
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;
    gfal_srm_result *resu = NULL;
	char **sup_protocols = NULL;
    GError *tmp_err = NULL;
    int ret = -1;

    gfal_srm_params_t params = gfal_srm_params_new(opts);
    if (params != NULL) {
        gfal_srm_params_set_spacetoken(params, gfalt_get_src_spacetoken(p, NULL));
        sup_protocols = srm_get_3rdparty_turls_sup_protocol(opts->handle);
        reorder_rd3_sup_protocols(sup_protocols, surl, other_surl);
        gfal_srm_params_set_protocols(params, sup_protocols);

        ret = gfal_srm_mTURLS_internal(opts, params, SRM_GET, surl, &resu, &tmp_err);
        if (ret >= 0) {
            if (resu->err_code == 0) {
                g_strlcpy(buff_turl, resu->turl, size_turl);
                if (reqtoken)
                    g_strlcpy(reqtoken, resu->reqtoken, size_reqtoken);
                ret = 0;
            }
            else {
                gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(),
                    resu->err_code, __func__,
                    "error on the turl %s request : %s ", resu->turl, resu->err_str);
                ret = -1;
            }
            free(resu);
        }
        gfal_srm_params_free(params);
    }
    G_RETURN_ERR(ret, tmp_err, err);
}



//  execute a put for thirdparty transfer turl
int gfal_srm_put_rd3_turl(plugin_handle ch, gfalt_params_t p, const char *surl, const char *other_surl,
    size_t surl_file_size, char *buff_turl, int size_turl,
    char *reqtoken, size_t size_reqtoken,
    GError **err)
{
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;
    gfal_srm_result *resu = NULL;
	char **sup_protocols = NULL;
    GError *tmp_err = NULL;
    int ret = -1;

    gfal_srm_params_t params = gfal_srm_params_new(opts);
    if (params != NULL) {
        gfal_srm_params_set_spacetoken(params, gfalt_get_dst_spacetoken(p, NULL));
        sup_protocols = srm_get_3rdparty_turls_sup_protocol(opts->handle);
        reorder_rd3_sup_protocols(sup_protocols, surl, other_surl);
        gfal_srm_params_set_protocols(params, sup_protocols);
        gfal_srm_params_set_size(params, surl_file_size);

        ret = gfal_srm_mTURLS_internal(opts, params, SRM_PUT, surl, &resu, &tmp_err);
        if (ret >= 0) {
            if (resu->err_code == 0) {
                g_strlcpy(buff_turl, resu->turl, size_turl);
                if (reqtoken)
                    g_strlcpy(reqtoken, resu->reqtoken, size_reqtoken);
                ret = 0;
                free(resu);
            }
            else {
                gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(),
                    resu->err_code, __func__,
                    "error on the turl %s request : %s ", resu->turl, resu->err_str);
                ret = -1;
            }

        }
        gfal_srm_params_free(params);
    }

    G_RETURN_ERR(ret, tmp_err, err);
}


//  simple wrapper to putTURLs for the gfal_module layer
int gfal_srm_putTURLS_plugin(plugin_handle ch, const char *surl, char *buff_turl, int size_turl, char **reqtoken,
    GError **err)
{
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;
    gfal_srm_result *resu = NULL;
    GError *tmp_err = NULL;
    int ret = -1;

    gfal_srm_params_t params = gfal_srm_params_new(opts);
    if (params != NULL) {
        ret = gfal_srm_mTURLS_internal(opts, params, SRM_PUT, surl, &resu, &tmp_err);
        if (ret > 0) {
            if (resu->err_code == 0) {
                g_strlcpy(buff_turl, resu->turl, size_turl);
                if (reqtoken)
                    *reqtoken = resu->reqtoken;
                ret = 0;

            } else {
                gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), resu->err_code, __func__,
                    "error on the turl request : %s ", resu->err_str);
                ret = -1;
            }
            free(resu);
        }
        gfal_srm_params_free(params);
    }

    G_RETURN_ERR(ret, tmp_err, err);
}

// the surl protocol is considered in the first position of the supported protocols
int reorder_rd3_sup_protocols(char **sup_protocols, const char *surl, const char *other_surl)
{
    int n_protocols = g_strv_length(sup_protocols);
    int j;

    // Check the other_surl protocol is in the request list
    for (j = 0; j < n_protocols; ++j) {
    	size_t proto_len = strlen(sup_protocols[j]);
        if (strncmp(sup_protocols[j], other_surl, proto_len) == 0 && other_surl[proto_len] == ':') {
            g_strlcpy (sup_protocols[j], sup_protocols[0], strlen(sup_protocols[0])+1);
            g_strlcpy (sup_protocols[0], other_surl, proto_len+1);
            break;
        }
    }

    return 0;
}

// execute a srm put done on the specified surl and token, return 0 if success else -1 and errno is set
static int gfal_srm_putdone_srmv2_internal(srm_context_t context, char *surl, const char *token, GError **err)
{
    g_return_val_err_if_fail(surl != NULL, -1, err, "[gfal_srm_putdone_srmv2_internal] invalid args ");

    GError *tmp_err = NULL;
    int ret = 0;
    struct srm_putdone_input putdone_input;
    struct srmv2_filestatus *statuses;

    // set the structures datafields
    putdone_input.nbfiles = 1;
    putdone_input.reqtoken = (char *) token;
    putdone_input.surls = &surl;

    gfal2_log(G_LOG_LEVEL_DEBUG, "    [gfal_srm_putdone_srmv2_internal] start srm put done on %s", surl);
    ret = gfal_srm_external_call.srm_put_done(context, &putdone_input, &statuses);
    if (ret < 0) {
        gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), errno, __func__,
            "call to srm_ifce error: %s", context->errbuf);
    } else {
        ret = gfal_srm_convert_filestatuses_to_GError(statuses, ret, &tmp_err);
        gfal_srm_external_call.srm_srmv2_filestatus_delete(statuses, 1);
    }

    G_RETURN_ERR(ret, tmp_err, err);
}


int gfal_srm_putdone(gfal_srmv2_opt *opts, const char *surl, const char *token, GError **err)
{
    GError *tmp_err = NULL;
    int ret = -1;

    gfal2_log(G_LOG_LEVEL_DEBUG, "   -> [gfal_srm_putdone] ");

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (easy != NULL) {
        ret = gfal_srm_putdone_srmv2_internal(easy->srm_context, easy->path, token, &tmp_err);
    }
    gfal_srm_ifce_easy_context_release(opts, easy);

    if (ret < 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}


static int srmv2_abort_request_internal(srm_context_t context, const char *surl,
    const char *req_token, GError **err)
{
    GError *tmp_err = NULL;

    int ret = gfal_srm_external_call.srm_abort_request(context, (char *) req_token);
    if (ret < 0) {
        gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), errno, __func__,
            "SRMv2 abort request error : %s", context->errbuf);
    }

    G_RETURN_ERR(ret, tmp_err, err);
}


int srm_abort_request_plugin(plugin_handle *handle, const char *surl,
    const char *reqtoken, GError **err)
{
    g_return_val_err_if_fail(handle != NULL && reqtoken != NULL, -1, err,
        "[srm_abort_request_plugin] invalid values for token/handle");
    GError *tmp_err = NULL;
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) handle;
    int ret = -1;

    gfal2_log(G_LOG_LEVEL_DEBUG, "   -> [srm_abort_request] ");

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (easy != NULL) {
        ret = srmv2_abort_request_internal(easy->srm_context, easy->path, reqtoken, &tmp_err);
    }
    gfal_srm_ifce_easy_context_release(opts, easy);

    gfal2_log(G_LOG_LEVEL_DEBUG, " [srm_abort_request] <-");

    if (ret < 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}
