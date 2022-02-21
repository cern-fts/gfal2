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

#include <checksums/checksums.h>
#include <uri/gfal2_uri.h>

#include "gfal_srm_getput.h"
#include "gfal_srm_namespace.h"
#include "gfal_srm_url_check.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_bringonline.h"

/** default buffer size for protocol schema */
#define GFAL_SCHEMA_MAX_LEN 64


GQuark srm_domain()
{
    return g_quark_from_static_string("SRM");
}

static GQuark gfal2_get_srm_get_quark()
{
    return g_quark_from_static_string("SRM:GET");
}

static GQuark gfal2_get_srm_put_quark()
{
    return g_quark_from_static_string("SRM:PUT");
}


int srm_plugin_delete_existing_copy(plugin_handle handle, gfalt_params_t params,
    const char *surl, GError **err)
{
    GError *tmp_err = NULL;
    int res = 0;
    const gboolean replace = gfalt_get_replace_existing_file(params, NULL);
    if (replace) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Trying to delete %s", surl);
        res = gfal_srm_unlinkG(handle, surl, &tmp_err);
        if (res == 0) {
            gfal2_log(G_LOG_LEVEL_DEBUG, "%s deleted with success", surl);
            plugin_trigger_event(params, srm_domain(), GFAL_EVENT_DESTINATION,
                GFAL_EVENT_OVERWRITE_DESTINATION,
                "Deleted %s", surl);
        }
        else if (tmp_err->code == ENOENT) {
            gfal2_log(G_LOG_LEVEL_MESSAGE, "%s doesn't exist, carry on", surl);
            g_clear_error(&tmp_err);
            tmp_err = NULL;
            res = 0;
        }
            // Workaround for BeStMan, which returns EINVAL instead of ENOENT
        else if (tmp_err->code == EINVAL) {
            gfal2_log(G_LOG_LEVEL_MESSAGE, "Got EINVAL removing %s. Assuming ENOENT (for BeStMan storages)", surl);
            g_clear_error(&tmp_err);
            tmp_err = NULL;
            res = 0;
        }
    }
    if (tmp_err)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    return res;
}

// create the parent directory
// return 0 if nothing or not requested
// return 1 if creation has been done
// return < 0 in case of error
int srm_plugin_create_parent_copy(plugin_handle handle, gfalt_params_t params,
    const char *surl, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    const gboolean create_parent = gfalt_get_create_parent_dir(params, NULL);
    if (create_parent) {
        char *path_dir = g_strdup(surl);
        char *p = path_dir + strlen(path_dir) - 1;
        while (*p == '/') { // remote trailing /
            *p = '\0';
            p--;
        }
        const unsigned int pref_len = GFAL_PREFIX_SRM_LEN;
        while (*p != '/' && (path_dir + pref_len) < p)
            p--;
        if ((path_dir + pref_len) < p) {
            *p = '\0';
            gfal2_log(G_LOG_LEVEL_DEBUG, " try to create parent dir : %s for %s", path_dir, surl);
            res = gfal_srm_mkdir_recG(handle, path_dir, 0755, &tmp_err);
            if (res == 0)
                gfal2_log(G_LOG_LEVEL_DEBUG, "parent path %s created with success", path_dir);
        }
        else {
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EINVAL, __func__,
                "Invalid srm url %s", surl);
            res = -1;
        }
        g_free(path_dir);
    }
    else {
        res = 0;
    }
    if (tmp_err)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    return res;
}


static int srm_plugin_prepare_dest_put(plugin_handle handle,
    gfal2_context_t context, gfalt_params_t params, const char *surl,
    GError **err)
{
    GError *tmp_err = NULL;
    int res;
    res = srm_plugin_delete_existing_copy(handle, params, surl, &tmp_err);
    if (res == 0) {
        res = srm_plugin_create_parent_copy(handle, params, surl, &tmp_err);
        if (res < 0)
            gfalt_propagate_prefixed_error(err, tmp_err, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_PARENT);
    }
    else {
        gfalt_propagate_prefixed_error(err, tmp_err, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_OVERWRITE);
    }

    return res;
}

// Returns 0 on success, < 0 on failure or > 0 if SURL not an SRM endpoint
static int srm_resolve_get_turl(plugin_handle handle, gfalt_params_t params, char **protocols,
    const char *surl, char *turl, size_t turl_size,
    char *token, size_t token_size,
    GError **err)
{
    GError* tmp_err = NULL;
    int res;

    if (srm_check_url(surl)) {
        res = gfal_srm_get_rd3_turl(handle, params, protocols, surl,
                                    turl, turl_size, token, token_size,
                                    &tmp_err);
    }
    else {
        g_strlcpy(turl, surl, turl_size);
        gfal2_log(G_LOG_LEVEL_DEBUG, "\t\tNo SRM resolution needed on %s", surl);
        token[0] = '\0';
        res = 1;
    }

    if (tmp_err)
        gfalt_propagate_prefixed_error(err, tmp_err, __func__, GFALT_ERROR_SOURCE, "SRM_GET_TURL");

    return res;
}

// Returns 0 on success, < 0 on failure or > 0 if SURL not an SRM endpoint
static int srm_resolve_put_turl(plugin_handle handle, gfal2_context_t context,
    gfalt_params_t params, char **protocols,
    const char *surl, off_t file_size_surl,
    char *turl, size_t turl_size,
    char *token, size_t token_size,
    GError **err)
{
    GError* tmp_err = NULL;
    int res;

    if (srm_check_url(surl)) {
        res = srm_plugin_prepare_dest_put(handle, context, params, surl, &tmp_err);

        if (res == 0) {
            res = gfal_srm_put_rd3_turl(handle, params, protocols, surl, file_size_surl,
                                        turl, turl_size, token, token_size,
                                        &tmp_err);

            if (res < 0) {
                gfalt_propagate_prefixed_error(err, tmp_err, __func__, GFALT_ERROR_DESTINATION, "SRM_PUT_TURL");
                return res;
            }
        }
    }
    else {
        g_strlcpy(turl, surl, turl_size);
        gfal2_log(G_LOG_LEVEL_DEBUG, "\t\tNo SRM resolution needed on %s", surl);
        token[0] = '\0';
        res = 1;
    }

    if (tmp_err != NULL)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return res;
}


static void srm_resolve_get_turl_log_event(gfalt_params_t params, const char* surl,
                                           const char* turl, const char* token)
{
    if (srm_check_url(surl)) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "\t\t[GET] SURL -> TURL resolution finished: %s -> %s (%s)",
                  surl, turl, token);
        plugin_trigger_event(params, gfal2_get_plugin_srm_quark(),
                             GFAL_EVENT_SOURCE, gfal2_get_srm_get_quark(),
                             "Got TURL %s => %s", surl, turl);
    }
}

static void srm_resolve_put_turl_log_event(gfalt_params_t params, const char* surl,
                                           const char* turl, const char* token)
{
    if (srm_check_url(surl)) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "\t\t[PUT] SURL -> TURL resolution finished: %s -> %s (%s)",
                  surl, turl, token);
        plugin_trigger_event(params, gfal2_get_plugin_srm_quark(),
                             GFAL_EVENT_DESTINATION, gfal2_get_srm_put_quark(),
                             "Got TURL %s => %s", surl, turl);
    }
}


static void srm_resolve_protocol_list_log(GLogLevelFlags loglevel, const char *msg, char **protocols)
{
    size_t n_protocols = g_strv_length(protocols);
    GString *msgline = g_string_new(msg);
    int i;

    for (i = 0; i < n_protocols; ++i) {
        if (i > 0) {
            g_string_append_c(msgline, ';');
        }

        g_string_append(msgline, protocols[i]);
    }

    gfal2_log(loglevel, "%s", msgline->str);
    g_string_free(msgline, TRUE);
}


// Return the configuration list of protocols.
// If one of the involved SURLs is not SRM, prioritize it.
static char** srm_resolve_protocol_list(plugin_handle ch, const char* source, const char* dest)
{
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;
    const char* non_srm_protocol = NULL;
    char** protocols = NULL;
    char* shift = NULL;
    size_t n_protocols;
    size_t non_srm_prot_len;
    gfal2_uri* parsed = NULL;
    GError* err = NULL;
    int i, j;

    protocols = srm_get_3rdparty_turls_sup_protocol(opts->handle);
    n_protocols = g_strv_length(protocols);
    srm_resolve_protocol_list_log(G_LOG_LEVEL_DEBUG, "Initial protocols for TURL resolution: ", protocols);

    if (!srm_check_url(source)) {
        non_srm_protocol = source;
    } else if (!srm_check_url(dest)) {
        non_srm_protocol = dest;
    }

    // Prioritize non-SRM protocol
    if (non_srm_protocol != NULL) {
        parsed = gfal2_parse_uri(non_srm_protocol, &err);

        if (err != NULL) {
            g_clear_error(&err);
            return protocols;
        }

        non_srm_protocol = parsed->scheme;

        // Treat "davs://" and "https:// as the same protocol
        if (strncmp(non_srm_protocol, "davs", 4) == 0) {
            non_srm_protocol = "https";
        }

        non_srm_prot_len = strlen(non_srm_protocol);

        // Check the compare_surl protocol is in the request list
        for (i = 1; i < n_protocols; i++) {
            if (strncmp(protocols[i], non_srm_protocol, non_srm_prot_len) == 0) {
                shift = protocols[i];

                for (j = i; j > 0; j--) {
                    protocols[j] = protocols[j - 1];
                }

                protocols[0] = shift;
                break;
            }
        }

        gfal2_free_uri(parsed);
    }

    srm_resolve_protocol_list_log(G_LOG_LEVEL_DEBUG, "Reordered protocols for TURL resolution: ", protocols);
    return protocols;
}

static int srm_get_checksum_config(gfal2_context_t context, gfalt_params_t params,
    gfalt_checksum_mode_t *mode, char *algorithm, size_t algorithm_size,
    char *user_checksum, size_t user_checksum_size,
    GError **err)
{
    *mode = gfalt_get_checksum(params, algorithm, algorithm_size, user_checksum, user_checksum_size, NULL);
    gboolean allow_empty_source = gfal2_get_opt_boolean(context, "SRM PLUGIN", "ALLOW_EMPTY_SOURCE_CHECKSUM", NULL);
    if (allow_empty_source) {
        *mode = *mode & ~GFALT_CHECKSUM_SOURCE;
    }

    if (algorithm[0] == '\0') {
        const char *configured;
        configured = gfal2_get_opt_string(context, srm_config_group, srm_config_transfer_checksum, err);
        if (configured != NULL)
            g_strlcpy(algorithm, configured, algorithm_size);
    }

    if (*err == NULL) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "\t\tChecksum check enabled: %d", *mode);
        if (*mode) {
            gfal2_log(G_LOG_LEVEL_DEBUG, "\t\tVerify source checksum: %d", (*mode & GFALT_CHECKSUM_SOURCE));
            gfal2_log(G_LOG_LEVEL_DEBUG, "\t\tChecksum algorithm: %s", algorithm);
            gfal2_log(G_LOG_LEVEL_DEBUG, "\t\tUser defined checksum: %s", user_checksum);
        }
        return 0;
    }
    else {
        return -1;
    }
}


static int srm_validate_source_checksum(plugin_handle handle, gfal2_context_t context,
    gfalt_params_t params, const char *src,
    gfalt_checksum_mode_t checksum_mode,
    const char *checksum_algorithm, const char *checksum_user,
    char *checksum_source, size_t checksum_source_size,
    GError **err)
{
    GError *tmp_err = NULL;

    plugin_trigger_event(params, srm_domain(), GFAL_EVENT_SOURCE,
        GFAL_EVENT_CHECKSUM_ENTER, "");

    int ret = 0;

    gboolean turl_fallback = (checksum_mode & GFALT_CHECKSUM_SOURCE);
    ret = gfal_srm_checksumG_fallback(handle, src, checksum_algorithm,
        checksum_source, checksum_source_size, 0, 0, turl_fallback, &tmp_err);

    if (ret != 0) {
        gfalt_propagate_prefixed_error(err, tmp_err, __func__, GFALT_ERROR_SOURCE, GFALT_ERROR_CHECKSUM);
    }
    else if (checksum_mode & GFALT_CHECKSUM_SOURCE && checksum_user[0]) {
        if (gfal_compare_checksums(checksum_source, checksum_user, checksum_source_size) != 0) {
            gfalt_set_error(err, gfal2_get_plugin_srm_quark(), EIO, __func__,
                GFALT_ERROR_SOURCE, GFALT_ERROR_CHECKSUM_MISMATCH,
                "User defined checksum and source checksum do not match %s != %s",
                checksum_user, checksum_source);
            ret = -1;
        }
    }

    plugin_trigger_event(params, srm_domain(), GFAL_EVENT_SOURCE,
        GFAL_EVENT_CHECKSUM_EXIT, "");

    return ret;
}


static int srm_validate_destination_checksum(plugin_handle handle, gfal2_context_t context,
    gfalt_params_t params, const char *dst,
    const char *checksum_algorithm, const char *checksum_user, const char *checksum_source,
    GError **err)
{
    GError *tmp_err = NULL;

    plugin_trigger_event(params, srm_domain(), GFAL_EVENT_DESTINATION,
        GFAL_EVENT_CHECKSUM_ENTER, "");

    int ret = 0;

    char checksum_destination[GFAL_URL_MAX_LEN] = {0};
    ret = gfal_srm_checksumG_fallback(handle, dst, checksum_algorithm,
        checksum_destination, sizeof(checksum_destination), 0, 0, TRUE,
        &tmp_err);

    if (ret == 0) {
        if (checksum_destination[0] != '\0') {
            if (checksum_source[0] != '\0' &&
                gfal_compare_checksums(checksum_source, checksum_destination, sizeof(checksum_destination)) != 0) {
                gfalt_set_error(err, gfal2_get_plugin_srm_quark(), EIO, __func__,
                    GFALT_ERROR_TRANSFER, GFALT_ERROR_CHECKSUM_MISMATCH,
                    "Source and destination checksums do not match %s != %s",
                    checksum_source, checksum_destination);
                ret = -1;
            }
            else if (checksum_user[0] != '\0' &&
                     gfal_compare_checksums(checksum_user, checksum_destination, sizeof(checksum_destination)) != 0) {
                gfalt_set_error(err, gfal2_get_plugin_srm_quark(), EIO, __func__,
                    GFALT_ERROR_TRANSFER, GFALT_ERROR_CHECKSUM_MISMATCH,
                    "User defined checksum and destination checksums do not match %s != %s",
                    checksum_user, checksum_destination);
                ret = -1;
            }
        }
        else {
            gfalt_set_error(err, gfal2_get_plugin_srm_quark(), EINVAL, __func__,
                GFALT_ERROR_DESTINATION, GFALT_ERROR_CHECKSUM,
                "Empty destination checksum");
            ret = -1;
        }
    }
    else {
        gfalt_propagate_prefixed_error(err, tmp_err, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_CHECKSUM);
    }

    plugin_trigger_event(params, srm_domain(), GFAL_EVENT_DESTINATION,
        GFAL_EVENT_CHECKSUM_EXIT, "");

    return ret;
}

static void srm_force_unlink(plugin_handle handle,
    gfal2_context_t context,
    const char *surl,
    GError **err)
{
    GError *unlink_err = NULL;
    gfal_srm_unlinkG(handle, surl, &unlink_err);
    if (unlink_err != NULL) {
        if (unlink_err->code != ENOENT) {
            gfal2_log(G_LOG_LEVEL_WARNING,
                "Got an error when removing the destination surl: %s",
                unlink_err->message);
        }
        else {
            gfal2_log(G_LOG_LEVEL_DEBUG, "Destination surl did not exist after abort");
        }
        g_error_free(unlink_err);
    }
    else {
        gfal2_log(G_LOG_LEVEL_MESSAGE, "Successfully removed destination surl after abort: %s", surl);
    }
}

static void srm_rollback_put(plugin_handle handle,
    gfal2_context_t context,
    const char *surl, const char *token,
    gboolean transfer_finished,
    GError **err)
{
    gfal2_log(G_LOG_LEVEL_MESSAGE, "Rolling back PUT");

    GError *abort_error = NULL;
    // If the transfer finished, or the destination is not an SRM
    // remove the destination
    if ((*err && (*err)->code != EEXIST) && (transfer_finished || !srm_check_url(surl))) {
        gfal2_unlink(context, surl, &abort_error);
        // It may not be there, so be gentle
        if (abort_error != NULL)
            g_error_free(abort_error);
    }
        // If the transfer is not finished, abort it
    else if (!transfer_finished && token[0] != '\0') {
        srm_abort_request_plugin(handle, surl, token, &abort_error);
        if (abort_error != NULL) {
            if (*err == NULL) {
                *err = abort_error;
            }
            else {
                gfal2_log(G_LOG_LEVEL_WARNING,
                    "Got an error when canceling the PUT request: %s",
                    abort_error->message);
                g_error_free(abort_error);
            }
        }
        // Some endpoints may not remove the file after an abort (i.e. Castor),
        // so do it ourselves if it is still there (see LCGUTIL-358)
        srm_force_unlink(handle, context, surl, err);
    }
}


static void srm_release_get(plugin_handle handle, const char *surl, const char *token,
    GError **err)
{
    gfal2_log(G_LOG_LEVEL_MESSAGE, "Rolling back GET");

    GError *release_error = NULL;
    gfal_srmv2_release_fileG(handle, surl, token, &release_error);
    if (release_error != NULL) {
        gfal2_log(G_LOG_LEVEL_WARNING,
            "Got an error when releasing the source file: %s",
            release_error->message);
        gfal2_log(G_LOG_LEVEL_WARNING,
            "It will be ignored!");
    }
}


static int srm_resolve_turls(plugin_handle handle, gfal2_context_t context,
    gfalt_params_t params,
    const char *source, char *turl_source, char *token_source,
    const char *dest, char *turl_destination, char *token_destination,
    GError **err)
{
    GError *tmp_err = NULL;
    char buffer[1024];
    struct stat stat_source;
    memset(&stat_source, 0, sizeof(stat_source));

    if (gfal2_stat(context, source, &stat_source, &tmp_err) != 0) {
        stat_source.st_size = 0;
        gfal2_log(G_LOG_LEVEL_DEBUG,
            "Fail to stat src SRM url %s to determine file size, try with file_size=0, error %s",
            source, tmp_err->message);
        g_clear_error(&tmp_err);
        tmp_err = NULL;
    }

    // Check if the source file is online in case the SRM_COPY_FAIL_NEARLINE is set
    gboolean fail_nearline = gfal2_get_opt_boolean_with_default(context, "SRM PLUGIN", "COPY_FAIL_NEARLINE", FALSE);

    if (fail_nearline && srm_check_url(source)) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Copy-fail-nearline: querying status first");
        ssize_t ret = gfal2_getxattr(context,  source, GFAL_XATTR_STATUS, buffer, sizeof(buffer), &tmp_err);

        if (ret > 0 && strlen(buffer) > 0 && tmp_err == NULL) {
            if (strncmp(buffer, GFAL_XATTR_STATUS_NEARLINE, sizeof(GFAL_XATTR_STATUS_NEARLINE)) == 0) {
                gfal2_log(G_LOG_LEVEL_DEBUG, "Copy-fail-nearline: The source file is not ONLINE");
                gfalt_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EINVAL, __func__,
                    GFALT_ERROR_SOURCE, "SRM_GET_TURL", "The source file is not ONLINE");
                gfal2_propagate_prefixed_error(err, tmp_err, __func__);
                return -1;
            }
        } else {
            if (tmp_err == NULL) {
                gfalt_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EINVAL, __func__,
                    GFALT_ERROR_SOURCE, "SRM_GET_TURL", "Error while checking if the source file is ONLINE");
            }
            gfal2_propagate_prefixed_error(err, tmp_err, __func__);
            return -1;
        }
    }

    // Try to match 3rd TURLs exhaustively
    // Algorithm:
    //   - Reorder the configured list of possible protocols
    //   - For each protocol in the list, try to get both getTURL and putTURL
    //   - Stop when successful or exhausted the protocol list

    char turl_source_resolved[GFAL_URL_MAX_LEN] = {0};
    char token_source_resolved[GFAL_URL_MAX_LEN] = {0};
    char turl_destination_resolved[GFAL_URL_MAX_LEN] = {0};
    char token_destination_resolved[GFAL_URL_MAX_LEN] = {0};
    char** protocols_tmp;
    char** protocols;
    char* protocol;
    size_t n_protocols;
    int i;

    gboolean protocol_agreement_flag = FALSE;
    GError* src_tmp_err = NULL;
    GError* dst_tmp_err = NULL;
    int src_turl_retc = 0;
    int dst_turl_retc = 0;

    protocols = srm_resolve_protocol_list(handle, source, dest);
    n_protocols = g_strv_length(protocols);

    for (i = 0; i < n_protocols; i++) {
        protocol = protocols[i];
        gfal2_log(G_LOG_LEVEL_DEBUG, "Trying to resolve TURLs for protocol: %s", protocol);
        protocols_tmp = calloc(2, sizeof(char *));
        protocols_tmp[0] = g_strdup(protocol);

        if (src_turl_retc <= 0) {
            src_turl_retc = srm_resolve_get_turl(handle, params, protocols_tmp, source,
                                                 turl_source, GFAL_URL_MAX_LEN,
                                                 token_source, GFAL_URL_MAX_LEN,
                                                 &src_tmp_err);
        }

        if (dst_turl_retc <= 0) {
            dst_turl_retc = srm_resolve_put_turl(handle, context, params, protocols_tmp,
                                                 dest, stat_source.st_size,
                                                 turl_destination, GFAL_URL_MAX_LEN,
                                                 token_destination, GFAL_URL_MAX_LEN,
                                                 &dst_tmp_err);
        }

        g_strfreev(protocols_tmp);
        g_clear_error(&src_tmp_err);
        g_clear_error(&dst_tmp_err);

        // Both TURLs resolved
        if (src_turl_retc >= 0 && dst_turl_retc >= 0) {
            // Write source TURL and token
            g_strlcpy(turl_source_resolved, turl_source, GFAL_URL_MAX_LEN);
            g_strlcpy(token_source_resolved, token_source, GFAL_URL_MAX_LEN);
            // Write destination TURL and token
            g_strlcpy(turl_destination_resolved, turl_destination, GFAL_URL_MAX_LEN);
            g_strlcpy(token_destination_resolved, token_destination, GFAL_URL_MAX_LEN);
            break;
        }

        // Keep the first TURL for source and destination as backups
        if (src_turl_retc >= 0 && turl_source_resolved[0] == '\0') {
            g_strlcpy(turl_source_resolved, turl_source, GFAL_URL_MAX_LEN);
            g_strlcpy(token_source_resolved, token_source, GFAL_URL_MAX_LEN);
        }

        if (dst_turl_retc >= 0 && turl_destination_resolved[0] == '\0') {
            g_strlcpy(turl_destination_resolved, turl_destination, GFAL_URL_MAX_LEN);
            g_strlcpy(token_destination_resolved, token_destination, GFAL_URL_MAX_LEN);
        }
    }

    if (turl_source_resolved[0] == '\0') {
        srm_resolve_protocol_list_log(G_LOG_LEVEL_ERROR, "Source does not support any TURL for protocols: ", protocols);
        gfalt_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EINVAL, __func__,
                        GFALT_ERROR_SOURCE, "SRM_RESOLVE_TURLS",  "SRM 3rd TURLs configuration invalid");
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        g_strfreev(protocols);
        return -1;
    }

    if (turl_destination_resolved[0] == '\0') {
        srm_resolve_protocol_list_log(G_LOG_LEVEL_ERROR, "Destination does not support any TURL for protocols: ", protocols);
        gfalt_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EINVAL, __func__,
                        GFALT_ERROR_DESTINATION, "SRM_RESOLVE_TURLS",  "SRM 3rd TURLs configuration invalid");
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        g_strfreev(protocols);
        return -1;
    }

    // Use reserves as main TURLs
    g_strlcpy(turl_source, turl_source_resolved, GFAL_URL_MAX_LEN);
    g_strlcpy(token_source, token_source_resolved, GFAL_URL_MAX_LEN);
    g_strlcpy(turl_destination, turl_destination_resolved, GFAL_URL_MAX_LEN);
    g_strlcpy(token_destination, token_destination_resolved, GFAL_URL_MAX_LEN);

    srm_resolve_get_turl_log_event(params, source, turl_source, token_source);
    srm_resolve_put_turl_log_event(params, dest, turl_destination, token_destination);

    g_strfreev(protocols);
    return 0;
}


static int srm_do_transfer(plugin_handle handle, gfal2_context_t context,
    gfalt_params_t params,
    const char *destination, const char *token_destination,
    const char *turl_source, const char *turl_destination,
    GError **err)
{
    GError *tmp_err = NULL;

    gfalt_params_t params_turl;

    params_turl = gfalt_params_handle_copy(params, NULL);

    // checksum check done here!
    if (gfalt_set_checksum(params_turl, GFALT_CHECKSUM_NONE, NULL, NULL, err) < 0) {
        return -1;
    }

    if (srm_check_url(destination)) { // srm destination
        gfalt_set_replace_existing_file(params_turl, FALSE, NULL);
        gfalt_set_strict_copy_mode(params_turl, TRUE, NULL);
    }

    gfalt_copy_file(context, params_turl, turl_source, turl_destination, &tmp_err);
    if (tmp_err != NULL) // We assume the underlying copy tagged properly
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    gfalt_params_handle_delete(params_turl, NULL);
    if (*err == NULL) {
        plugin_trigger_event(params, srm_domain(), GFAL_EVENT_DESTINATION,
            GFAL_EVENT_CLOSE_ENTER, "%s", destination);

        if (srm_check_url(destination)) {
            if (gfal_srm_putdone(handle, destination, token_destination, &tmp_err) < 0)
                gfalt_propagate_prefixed_error(err, tmp_err, __func__, GFALT_ERROR_DESTINATION, "SRM_PUTDONE");
        }

        plugin_trigger_event(params, srm_domain(), GFAL_EVENT_DESTINATION,
            GFAL_EVENT_CLOSE_EXIT, "%s", destination);
    }

    return *err == NULL ? 0 : -1;
}


static int srm_cleanup_copy(plugin_handle handle, gfal2_context_t context,
    const char *source, const char *destination,
    const char *token_source, const char *token_destination,
    gboolean transfer_finished,
    GError **err)
{
    if (*err != NULL) {
        srm_rollback_put(handle, context, destination, token_destination, transfer_finished, err);
    }
    if (token_source[0] != '\0') {
        srm_release_get(handle, source, token_source, err);
    }
    return 0;
}


static void castor_gridftp_session_hack(plugin_handle handle, gfal2_context_t context,
    const char *src, const char *dst)
{
    int src_is_castor = is_castor_endpoint(handle, src);
    int dst_is_castor = is_castor_endpoint(handle, dst);

    if (src_is_castor || dst_is_castor) {
        gfal2_log(G_LOG_LEVEL_MESSAGE,
            "Found a Castor endpoint, or could not determine version! Disabling GridFTP session reuse and stat on open");
        gfal2_set_opt_boolean(context, "GRIDFTP PLUGIN", "SESSION_REUSE", FALSE, NULL);
        gfal2_set_opt_boolean(context, "GRIDFTP PLUGIN", "STAT_ON_OPEN", FALSE, NULL);
    }
    else {
        gfal2_log(G_LOG_LEVEL_DEBUG,
            "No Castor endpoint. Honor configuration for SESSION_REUSE");
    }
}


int srm_plugin_filecopy(plugin_handle handle, gfal2_context_t context,
    gfalt_params_t params, const char *source, const char *dest, GError **err)
{
    GError *nested_error = NULL;
    char checksum_algorithm[64] = {0};
    char checksum_user[GFAL_URL_MAX_LEN] = {0};
    char checksum_source[GFAL_URL_MAX_LEN] = {0};
    char turl_source[GFAL_URL_MAX_LEN] = {0};
    char token_source[GFAL_URL_MAX_LEN] = {0};
    char turl_destination[GFAL_URL_MAX_LEN] = {0};
    char token_destination[GFAL_URL_MAX_LEN] = {0};
    gfalt_checksum_mode_t checksum_mode;
    gboolean transfer_finished = FALSE;

    // Check if any of the endpoints is castor
    // In that case, disable GridFTP session reuse (see LCGUTIL-448)
    // Reason is: Castor does not allow doing multiple operations within the same connection
    castor_gridftp_session_hack(handle, context, source, dest);

    plugin_trigger_event(params, srm_domain(), GFAL_EVENT_NONE,
        GFAL_EVENT_PREPARE_ENTER, "");

    srm_get_checksum_config(context, params,
        &checksum_mode,
        checksum_algorithm, sizeof(checksum_algorithm),
        checksum_user, sizeof(checksum_user),
        &nested_error);
    if (nested_error != NULL)
        goto copy_finalize;

    // Source checksum validation
    if (checksum_mode) {
        srm_validate_source_checksum(handle, context, params, source,
            checksum_mode,
            checksum_algorithm, checksum_user,
            checksum_source, sizeof(checksum_source),
            &nested_error);

        if (nested_error != NULL)
            goto copy_finalize;
    }

    // Resolve turls
    srm_resolve_turls(handle, context, params,
        source, turl_source, token_source,
        dest, turl_destination, token_destination,
        &nested_error);
    if (nested_error != NULL)
        goto copy_finalize;

    plugin_trigger_event(params, srm_domain(), GFAL_EVENT_NONE,
        GFAL_EVENT_PREPARE_EXIT, "");

    if (gfal_srm_check_cancel(context, &nested_error))
        goto copy_finalize;

    // Transfer
    srm_do_transfer(handle, context, params,
        dest, token_destination,
        turl_source, turl_destination, &nested_error);
    if (nested_error != NULL)
        goto copy_finalize;

    transfer_finished = TRUE;

    // Destination checksum validation
    if (checksum_mode & GFALT_CHECKSUM_TARGET) {
        srm_validate_destination_checksum(handle, context, params, dest,
            checksum_algorithm, checksum_user, checksum_source,
            &nested_error);
    }

// Cleanup and propagate error if needed
    copy_finalize:
    if (nested_error) {
        gfal2_log(G_LOG_LEVEL_WARNING, "Transfer failed with: %s", nested_error->message);
    }
    srm_cleanup_copy(handle, context, source, dest,
        token_source, token_destination,
        transfer_finished, &nested_error);
    if (nested_error != NULL)
        gfal2_propagate_prefixed_error(err, nested_error, __func__);
    else
        *err = NULL;
    return (*err == NULL) ? 0 : -1;
}
