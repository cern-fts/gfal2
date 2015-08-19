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
const char * srm_config_group= "SRM PLUGIN";
const char * srm_config_transfer_checksum= "COPY_CHECKSUM_TYPE";
const char * srm_ops_timeout_key= "OPERATION_TIMEOUT";
const char * srm_conn_timeout_key= "CONN_TIMEOUT";
const char * srm_desired_request_lifetime = "REQUEST_LIFETIME";
const char * srm_config_turl_protocols= "TURL_PROTOCOLS";
const char * srm_config_3rd_party_turl_protocols= "TURL_3RD_PARTY_PROTOCOLS";
const char * srm_config_keep_alive = "KEEP_ALIVE";
const char * srm_spacetokendesc = "SPACETOKENDESC";

#include "gfal_srm_internal_layer.h"
// hotfix for the old srm lib
void disable_srm_srmv2_pinfilestatus_delete(struct srmv2_pinfilestatus*  srmv2_pinstatuses, int n){}
void disable_srm_srmv2_mdfilestatus_delete(struct srmv2_mdfilestatus* mdfilestatus, int n){}
void disable_srm_srmv2_filestatus_delete(struct srmv2_filestatus*  srmv2_statuses, int n){}
void disable_srm_srm2__TReturnStatus_delete(struct srm2__TReturnStatus* status){}


struct _gfal_srm_external_call gfal_srm_external_call = {
    .srm_ls = &srm_ls,
    .srm_rmdir = &srm_rmdir,
    .srm_mkdir = &srm_mkdir,
    .srm_getpermission = &srm_getpermission,
    .srm_check_permission = &srm_check_permission,
    .srm_srmv2_pinfilestatus_delete = &srm_srmv2_pinfilestatus_delete,
    .srm_srmv2_mdfilestatus_delete = &srm_srmv2_mdfilestatus_delete,
    .srm_srmv2_filestatus_delete = &srm_srmv2_filestatus_delete,
    .srm_srm2__TReturnStatus_delete = &srm_srm2__TReturnStatus_delete,
    .srm_prepare_to_get= &srm_prepare_to_get,
    .srm_prepare_to_put= &srm_prepare_to_put,
    .srm_put_done = &srm_put_done,
    .srm_setpermission= &srm_setpermission,
    .srm_rm = &srm_rm,
    .srm_set_timeout_connect = &srm_set_timeout_connect,
    .srm_bring_online = &srm_bring_online,
    .srm_bring_online_async = &srm_bring_online_async,
    .srm_bring_online_status = &srm_status_of_bring_online_async,
    .srm_release_files = &srm_release_files,
    .srm_mv = &srm_mv,
    .srm_abort_request = &srm_abort_request,
    .srm_getspacetokens = &srm_getspacetokens,
    .srm_getspacemd =  &srm_getspacemd,
    .srm_abort_files = &srm_abort_files,
    .srm_xping = &srm_xping
};


static srm_context_t gfal_srm_ifce_context_setup(gfal2_context_t handle,
        const char* endpoint, const char* ucert, const char* ukey,
        char* errbuff, size_t s_errbuff, GError** err)
{
    gint timeout;
    srm_context_t context = NULL;
    GError* tmp_err = NULL;

    const gboolean keep_alive = gfal2_get_opt_boolean_with_default(handle,
            srm_config_group, srm_config_keep_alive, FALSE);
    gfal2_log(G_LOG_LEVEL_DEBUG, " SRM connection keep-alive %d", keep_alive);

    context = srm_context_new2(endpoint, errbuff, s_errbuff,
            gfal2_log_get_level() >= G_LOG_LEVEL_DEBUG, keep_alive);

    if (context != NULL) {
        timeout = gfal2_get_opt_integer_with_default(handle, srm_config_group,
                srm_ops_timeout_key, 180);
        gfal2_log(G_LOG_LEVEL_DEBUG, " SRM operation timeout %d", timeout);
        context->timeout = timeout;
        context->timeout_ops = timeout;

        timeout = gfal2_get_opt_integer_with_default(handle, srm_config_group,
                srm_conn_timeout_key, 60);
        gfal2_log(G_LOG_LEVEL_DEBUG, " SRM connection timeout %d", timeout);
        context->timeout_conn = timeout;

        if (ucert) {
            gfal2_log(G_LOG_LEVEL_DEBUG, " SRM using certificate %s", ucert);
            if (ukey)
                gfal2_log(G_LOG_LEVEL_DEBUG, " SRM using private key %s", ukey);
            srm_set_credentials(context, ucert, ukey);
        }

        // User agent
        const char *agent, *agent_version;
        gfal2_get_user_agent(handle, &agent, &agent_version);
        if (agent) {
            srm_set_user_agent(context, "%s/%s gfal2/%s", agent, agent_version, gfal2_version());
        }
        else {
            srm_set_user_agent(context, "gfal2/%s", gfal2_version());
        }

        // Client information
        char* client_info = gfal2_get_client_info_string(handle);
        if (client_info) {
            srm_set_http_header(context, "ClientInfo", client_info);
        }
        g_free(client_info);
    }
    else {
        gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EINVAL,
                __func__, "Impossible to create srm context");
    }

    G_RETURN_ERR(context, tmp_err, err);
}


static int is_same_context(gfal_srmv2_opt* opts, const char* endpoint, const char* ucert, const char* ukey)
{
    if (strcmp(opts->endpoint, endpoint) != 0)
        return 0;
    if ((ucert && strcmp(opts->x509_ucert, ucert) != 0) || (!ucert && opts->x509_ucert[0]))
        return 0;
    if ((ukey && strcmp(opts->x509_ukey, ukey) != 0) || (!ukey && opts->x509_ukey[0]))
        return 0;
    return 1;
}


srm_context_t gfal_srm_ifce_easy_context(gfal_srmv2_opt* opts,
        const char* surl, GError** err)
{
    GError* nested_error = NULL;
    char full_endpoint[GFAL_URL_MAX_LEN];
    enum gfal_srm_proto srm_types;

    if (gfal_srm_determine_endpoint(opts, surl, full_endpoint, sizeof(full_endpoint), &srm_types, &nested_error) < 0) {
        gfal2_propagate_prefixed_error(err, nested_error, __func__);
        return NULL;
    }

    gchar* ucert = gfal2_get_opt_string(opts->handle, "X509", "CERT", NULL);
    gchar* ukey = gfal2_get_opt_string(opts->handle, "X509", "KEY", NULL);

    g_static_rec_mutex_lock(&opts->srm_context_mutex);

    // Try with existing one
    if (opts->srm_context) {
        if (is_same_context(opts, full_endpoint, ucert, ukey)) {
            gfal2_log(G_LOG_LEVEL_INFO, "SRM context recycled for %s", full_endpoint);
        }
        else {
            gfal2_log(G_LOG_LEVEL_INFO, "SRM context invalidated for %s", full_endpoint);
            srm_context_free(opts->srm_context);
            opts->srm_context = NULL;
        }
    }
    else {
        gfal2_log(G_LOG_LEVEL_INFO, "SRM context not available");
    }

    // Instantiate if we haven't got any
    if (opts->srm_context == NULL) {
        switch (srm_types) {
            case PROTO_SRMv2:
                opts->srm_context = gfal_srm_ifce_context_setup(opts->handle, full_endpoint,
                                ucert, ukey,
                                opts->srm_ifce_error_buffer, sizeof(opts->srm_ifce_error_buffer),
                                &nested_error);
                if (nested_error)
                    gfal2_propagate_prefixed_error(err, nested_error, __func__);
                break;
            case PROTO_SRM:
                gfal2_set_error(err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT,
                                __func__, "SRM v1 is not supported, failure");
                break;
            default:
                gfal2_set_error(err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT,
                                __func__, "Unknow version of the protocol SRM, failure");
                break;
        }
    }

    // Configure
    if (opts->srm_context) {
        g_strlcpy(opts->endpoint, full_endpoint, GFAL_URL_MAX_LEN);
        if (ucert) {
            g_strlcpy(opts->x509_ucert, ucert, GFAL_URL_MAX_LEN);
        }
        if (ukey) {
            g_strlcpy(opts->x509_ukey, ukey, sizeof(opts->x509_ukey));
        }

        time_t request_lifetime = gfal2_get_opt_integer_with_default(opts->handle,
                srm_config_group, srm_desired_request_lifetime, 3600);
        srm_set_desired_request_time(opts->srm_context, request_lifetime);
    }
    else {
        g_static_rec_mutex_unlock(&opts->srm_context_mutex);
    }

    g_free(ucert);
    g_free(ukey);

    return opts->srm_context;
}


void gfal_srm_ifce_easy_context_release(gfal_srmv2_opt* opts,
        srm_context_t context)
{
    if (opts && context)
        g_static_rec_mutex_unlock(&opts->srm_context_mutex);
}
