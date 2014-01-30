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
 * @file gfal_srm_copy.c
 * @brief file for the third party transfer implementation
 * @author Devresse Adrien
 * */
#include <checksums/checksums.h>
#include <common/gfal_types.h>
#include <common/gfal_common_errverbose.h>
#include <file/gfal_file_api.h>
#include <transfer/gfal_transfer.h>
#include <transfer/gfal_transfer_plugins.h>
#include <uri/uri_util.h>

#include "gfal_srm_getput.h"
#include "gfal_srm_stat.h"
#include "gfal_srm_url_check.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_checksum.h"
#include "gfal_srm_mkdir.h"
#include "gfal_srm_bringonline.h"


GQuark srm_domain(){
    return g_quark_from_static_string("SRM");
}


int srm_plugin_delete_existing_copy(plugin_handle handle, gfalt_params_t params,
        const char * surl, GError ** err)
{
    GError * tmp_err = NULL;
    int res = 0;
    const gboolean replace = gfalt_get_replace_existing_file(params, NULL );
    if (replace) {
        gfal_log(GFAL_VERBOSE_TRACE, "\tTrying to delete %s", surl);
        res = gfal_srm_unlinkG(handle, surl, &tmp_err);
        if (res == 0) {
            gfal_log(GFAL_VERBOSE_TRACE, "\t%s deleted with success", surl);
        }
        else if (tmp_err->code == ENOENT) {
            gfal_log(GFAL_VERBOSE_TRACE, "\t%s doesn't exist, carry on", surl);
            g_clear_error(&tmp_err);
            tmp_err = NULL;
            res = 0;
        }
    }
    G_RETURN_ERR(res, tmp_err, err);
}

// create the parent directory 
// return 0 if nothing or not requested
// return 1 if creation has been done
// return < 0 in case of error
int srm_plugin_create_parent_copy(plugin_handle handle, gfalt_params_t params,
        const char * surl, GError ** err)
{
    GError * tmp_err = NULL;
    int res = -1;
    const gboolean create_parent = gfalt_get_create_parent_dir(params, NULL);
    if (create_parent) {
        char * path_dir = g_strdup(surl);
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
            gfal_log(GFAL_VERBOSE_TRACE, " try to create parent dir : %s for %s", path_dir, surl);
            res = gfal_srm_mkdir_recG(handle, path_dir, 0755, &tmp_err);
            if (res == 0)
                gfal_log(GFAL_VERBOSE_TRACE, "parent path %s created with success", path_dir);
        }
        else {
            g_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EINVAL,
                    "Invalid srm url %s", surl);
            res = -1;
        }
        g_free(path_dir);
    }
    else {
        res = 0;
    }
    G_RETURN_ERR(res, tmp_err, err);
}


static int srm_plugin_prepare_dest_put(plugin_handle handle,
        gfal2_context_t context, gfalt_params_t params, const char * surl,
        GError ** err)
{
    GError * tmp_err = NULL;
    int res;
    res = srm_plugin_delete_existing_copy(handle, params, surl, &tmp_err);
    if (res == 0)
        res = srm_plugin_create_parent_copy(handle, params, surl, &tmp_err);
    G_RETURN_ERR(res, tmp_err, err);
}


static int srm_resolve_get_turl(plugin_handle handle, gfalt_params_t params,
        const char* surl,
        char* turl, size_t turl_size,
        char* token, size_t token_size,
        GError** err)
{
    int res = 0;
    if (srm_check_url(surl)) {
        gfal_log(GFAL_VERBOSE_TRACE, "\t\tGET surl -> turl dst resolution start");
        res = gfal_srm_get_rd3_turl(handle, params, surl, turl, turl_size, token, token_size, err);
        if (res == 0) {
            gfal_log(GFAL_VERBOSE_TRACE, "\t\tGET surl -> turl dst resolution finished: %s -> %s (%s)",
                    surl, turl, token);
        }
    }
    else {
        g_strlcpy(turl, surl, turl_size);
        gfal_log(GFAL_VERBOSE_TRACE, "\t\tNo SRM resolution needed on %s", surl);
        token[0] = '\0';
    }
    return res;
}

// = 0 on success
// < 0 on failure
// > 0 if surl is not an srm endpoint
static int srm_resolve_put_turl(plugin_handle handle, gfal2_context_t context,
        gfalt_params_t params,
        const char* surl, off_t file_size_surl,
        char* turl, size_t turl_size,
        char* token, size_t token_size,
        GError ** err)
{
    int res = 0;
    if (srm_check_url(surl)) {
        gfal_log(GFAL_VERBOSE_TRACE, "\t\tPUT surl -> turl src resolution start ");
        res = srm_plugin_prepare_dest_put(handle, context, params, surl, err);
        if (res == 0) {
            res = gfal_srm_put_rd3_turl(handle, params, surl, file_size_surl,
                    turl, turl_size,
                    token, token_size,
                    err);
            if (res == 0)
                gfal_log(GFAL_VERBOSE_TRACE, "\t\tPUT surl -> turl src resolution ended : %s -> %s (%s)",
                        surl, turl, token);
        }
    }
    else {
        g_strlcpy(turl, surl, turl_size);
        gfal_log(GFAL_VERBOSE_TRACE, "\t\tNo SRM resolution needed on %s", surl);
        token[0] = '\0';
        res = 1;
    }
    return res;
}


static int srm_get_checksum_config(gfal2_context_t context, gfalt_params_t params,
        gboolean* enabled, gboolean* allow_empty,
        char* algorithm, size_t algorithm_size,
        char* user_checksum, size_t user_checksum_size,
        GError** err)
{
    *enabled = gfalt_get_checksum_check(params, NULL);
    *allow_empty = gfal2_get_opt_boolean(context, "SRM PLUGIN",
            "ALLOW_EMPTY_SOURCE_CHECKSUM", NULL);

    gfalt_get_user_defined_checksum(params, algorithm, algorithm_size,
            user_checksum, user_checksum_size, NULL);

    if (algorithm[0] == '\0') {
       const char* configured;
       configured = gfal2_get_opt_string(context, srm_config_group, srm_config_transfer_checksum, err);
       if (configured != NULL)
           strncpy(algorithm, configured, algorithm_size);
    }

    if (*err == NULL) {
        gfal_log(GFAL_VERBOSE_VERBOSE, "\t\tChecksum check enabled: %d", *enabled);
        if (*enabled) {
            gfal_log(GFAL_VERBOSE_VERBOSE, "\t\tAllow empty source checksum: %d", *allow_empty);
            gfal_log(GFAL_VERBOSE_VERBOSE, "\t\tChecksum algorithm: %s", algorithm);
            gfal_log(GFAL_VERBOSE_VERBOSE, "\t\tUser defined checksum: %s", user_checksum);
        }
        return 0;
    }
    else {
        return -1;
    }
}


static int srm_validate_source_checksum(plugin_handle handle, gfal2_context_t context,
        gfalt_params_t params, const char* src,
        const char* checksum_algorithm, const char* checksum_user,
        gboolean allow_empty,
        char* checksum_source, size_t checksum_source_size,
        GError** err)
{
    plugin_trigger_event(params, srm_domain(), GFAL_EVENT_SOURCE,
            GFAL_EVENT_CHECKSUM_ENTER, "");

    int ret = 0;

    ret = gfal_srm_checksumG_fallback(handle, src, checksum_algorithm,
            checksum_source, checksum_source_size, 0, 0, !allow_empty, err);
    if (ret == 0) {
        if (checksum_source[0] != '\0') {
            if (checksum_user[0] != '\0' &&
                gfal_compare_checksums(checksum_source, checksum_user, checksum_source_size) != 0) {
                g_set_error(err, gfal2_get_plugin_srm_quark(), EIO,
                        "User defined checksum and source checksum do not match %s != %s",
                        checksum_source, checksum_user);
                ret = -1;
            }
        }
        else if (!allow_empty) {
            g_set_error(err, gfal2_get_plugin_srm_quark(), EINVAL,
                    "Empty source checksum");
            ret = -1;
        }
        else {
            gfal_log(GFAL_VERBOSE_VERBOSE, "Source checksum could not be retrieved. Ignoring.");
        }
    }

    plugin_trigger_event(params, srm_domain(), GFAL_EVENT_SOURCE,
            GFAL_EVENT_CHECKSUM_EXIT, "");

    return ret;
}


static int srm_validate_destination_checksum(plugin_handle handle, gfal2_context_t context,
        gfalt_params_t params, const char* dst,
        const char* checksum_algorithm, const char* checksum_user, const char* checksum_source,
        GError** err)
{
    plugin_trigger_event(params, srm_domain(), GFAL_EVENT_DESTINATION,
            GFAL_EVENT_CHECKSUM_ENTER, "");

    int ret = 0;

    char checksum_destination[GFAL_URL_MAX_LEN] = {0};
    ret = gfal_srm_checksumG_fallback(handle, dst, checksum_algorithm,
            checksum_destination, sizeof(checksum_destination), 0, 0, TRUE,
            err);

    if (ret == 0) {
        if (checksum_destination[0] != '\0') {
            if (checksum_source[0] != '\0' &&
                gfal_compare_checksums(checksum_source, checksum_destination, sizeof(checksum_destination)) != 0) {
                g_set_error(err, gfal2_get_plugin_srm_quark(), EINVAL,
                        "Source and destination checksums do not match %s != %s",
                        checksum_source, checksum_destination);
                ret = -1;
            }
            else if (checksum_user[0] != '\0' &&
                gfal_compare_checksums(checksum_user, checksum_destination, sizeof(checksum_destination)) != 0) {
                g_set_error(err, gfal2_get_plugin_srm_quark(), EINVAL,
                        "User defined checksum and destination checksums do not match %s != %s",
                        checksum_user, checksum_destination);
                ret = -1;
            }
        }
        else {
            g_set_error(err, gfal2_get_plugin_srm_quark(), EINVAL,
                    "Empty destination checksum");
            ret = -1;
        }
    }

    plugin_trigger_event(params, srm_domain(), GFAL_EVENT_DESTINATION,
                GFAL_EVENT_CHECKSUM_EXIT, "");

    return ret;
}


static void srm_rollback_put(plugin_handle handle,
        gfal2_context_t context,
        const char* surl, const char* token,
        gboolean transfer_finished,
        GError** err)
{
    gfal_log(GFAL_VERBOSE_VERBOSE, "Rolling back PUT");

    GError* abort_error = NULL;
    // If the transfer finished, or the destination is not an SRM,
    // remove the destination
    if (transfer_finished || !srm_check_url(surl)) {
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
                GError* merged = NULL;
                g_set_error(&merged, gfal2_get_plugin_srm_quark(), (*err)->code,
                        "Transfer failed with %s\n"
                        "Also got an error when canceling the PUT request: %s",
                        (*err)->message, abort_error->message);
                g_error_free(*err);
                g_error_free(abort_error);
                *err = merged;
            }
        }
    }
}


static void srm_release_get(plugin_handle handle, const char* surl, const char* token,
        GError** err)
{
    gfal_log(GFAL_VERBOSE_VERBOSE, "Rolling back GET");

    GError* release_error = NULL;
    gfal_srmv2_release_fileG(handle, surl, token, &release_error);
    if (release_error != NULL) {
        if (*err == NULL) {
            *err = release_error;
        }
        else {
            GError* merged = NULL;
            g_set_error(&merged, gfal2_get_plugin_srm_quark(), (*err)->code,
                    "Transfer failed with %s\n"
                    "Also got an error when releasing the source file: %s",
                    (*err)->message, release_error->message);
            g_error_free(*err);
            g_error_free(release_error);
            *err = merged;
        }
    }
}


static int srm_resolve_turls(plugin_handle handle, gfal2_context_t context,
        gfalt_params_t params,
        const char* source, char* turl_source, char* token_source,
        const char* dest, char* turl_destination, char* token_destination,
        GError** err)
{
    struct stat stat_source;
    memset(&stat_source, 0, sizeof(stat_source));
    if (gfal2_stat(context, source, &stat_source, err) != 0) {
        stat_source.st_size = 0;
        gfal_log(GFAL_VERBOSE_DEBUG,
                "Fail to stat src SRM url %s to determine file size, try with file_size=0, error %s",
                source, (*err)->message);
        g_clear_error(err);
        *err = NULL;
    }

    srm_resolve_get_turl(handle, params, source,
            turl_source, GFAL_URL_MAX_LEN,
            token_source, GFAL_URL_MAX_LEN,
            err);
    if (*err != NULL)
        return -1;

    srm_resolve_put_turl(handle, context, params,
            dest, stat_source.st_size,
            turl_destination, GFAL_URL_MAX_LEN,
            token_destination, GFAL_URL_MAX_LEN,
            err);
    if (*err != NULL)
        return -1;

    return 0;
}


static int srm_do_transfer(plugin_handle handle, gfal2_context_t context,
        gfalt_params_t params,
        const char* destination, const char* token_destination,
        const char* turl_source, const char* turl_destination,
        GError **err)
{
    gfalt_params_t params_turl;

    params_turl = gfalt_params_handle_copy(params, NULL);
    gfalt_set_checksum_check(params_turl, FALSE, NULL); // checksum check done here!
    if (srm_check_url(destination)) { // srm destination
        gfalt_set_replace_existing_file(params_turl, FALSE, NULL);
        gfalt_set_strict_copy_mode(params_turl, TRUE, NULL);
    }

    gfalt_copy_file(context, params_turl, turl_source, turl_destination, err);
    gfalt_params_handle_delete(params_turl, NULL);
    if (*err == NULL) {
        plugin_trigger_event(params, srm_domain(), GFAL_EVENT_DESTINATION,
                GFAL_EVENT_CLOSE_ENTER, "%s", destination);

        if (srm_check_url(destination))
            gfal_srm_putdone_simple(handle, destination, token_destination, err);

        plugin_trigger_event(params, srm_domain(), GFAL_EVENT_DESTINATION,
                GFAL_EVENT_CLOSE_EXIT, "%s", destination);
    }

    return *err == NULL ? 0 : -1;
}


static int srm_cleanup_copy(plugin_handle handle, gfal2_context_t context,
        const char* source, const char* destination,
        const char* token_source, const char* token_destination,
        gboolean transfer_finished,
        GError** err)
{
    if (*err != NULL)
        srm_rollback_put(handle, context, destination, token_destination, transfer_finished, err);
    if (token_source[0] != '\0')
        srm_release_get(handle, source, token_source, err);
    return 0;
}


int srm_plugin_filecopy(plugin_handle handle, gfal2_context_t context,
        gfalt_params_t params, const char* source, const char* dest, GError ** err)
{
    GError *nested_error = NULL;
    char checksum_algorithm[64] = {0};
    char checksum_user[GFAL_URL_MAX_LEN] = {0};
    char checksum_source[GFAL_URL_MAX_LEN] = {0};
    char turl_source[GFAL_URL_MAX_LEN] = {0};
    char token_source[GFAL_URL_MAX_LEN] = {0};
    char turl_destination[GFAL_URL_MAX_LEN] = {0};
    char token_destination[GFAL_URL_MAX_LEN] = {0};
    gboolean is_checksum_enabled;
    gboolean allow_empty_source_checksum;
    gboolean transfer_finished = FALSE;

    plugin_trigger_event(params, srm_domain(), GFAL_EVENT_NONE,
            GFAL_EVENT_PREPARE_ENTER, "");

    srm_get_checksum_config(context, params,
            &is_checksum_enabled, &allow_empty_source_checksum,
            checksum_algorithm, sizeof(checksum_algorithm),
            checksum_user, sizeof(checksum_user),
            &nested_error);
    if (nested_error != NULL)
        goto copy_finalize;

    // Source checksum validation
    if (is_checksum_enabled) {
        srm_validate_source_checksum(handle, context, params, source,
                checksum_algorithm, checksum_user,
                allow_empty_source_checksum,
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
    if (is_checksum_enabled) {
        srm_validate_destination_checksum(handle, context, params, dest,
                checksum_algorithm, checksum_user, checksum_source,
                &nested_error);
    }

// Cleanup and propagate error if needed
copy_finalize:
    srm_cleanup_copy(handle, context, source, dest,
            token_source, token_destination,
            transfer_finished, &nested_error);
    if (nested_error != NULL)
        g_propagate_prefixed_error(err, nested_error, "[%s]", __func__);
    else
        *err = NULL;
    return (*err == NULL)? 0 : -1;
}
