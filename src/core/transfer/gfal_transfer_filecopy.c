/*
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <common/gfal_common_internal.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_err_helpers.h>
#include <transfer/gfal_transfer_plugins.h>
#include <transfer/gfal_transfer_internal.h>
#include <transfer/gfal_transfer_types_internal.h>
#include <cancel/gfal_cancel.h>

static GQuark scope_copy_domain() {
    return g_quark_from_static_string("FileCopy::start_copy");
}


static void* find_copy_plugin(gfal2_context_t context, gfal_url2_check operation,
        const char* src, const char* dst, void** plugin_data, GError** error)
{
    GError * tmp_err = NULL;
    plugin_pointer_handle start_list, p_list;
    void* resu = NULL;

    start_list = p_list = gfal_plugins_list_handler(context, &tmp_err);
    if (tmp_err != NULL) {
        gfal2_propagate_prefixed_error(error, tmp_err, __func__);
        return NULL;
    }

    while (p_list->plugin_api != NULL) {
        plugin_url_check2_call check_call = p_list->plugin_api->check_plugin_url_transfer;
        if (check_call != NULL) {
            gboolean compatible;
            if ((compatible = check_call(p_list->plugin_data, context, src, dst,
                    operation)) == TRUE) {
                *plugin_data = p_list->plugin_data;
                switch (operation) {
                    case GFAL_FILE_COPY:
                        resu = p_list->plugin_api->copy_file;
                        break;
                    case GFAL_BULK_COPY:
                        resu = p_list->plugin_api->copy_bulk;
                        break;
                }
            }
        }
        p_list++;
    }
    g_free(start_list);
    return resu;
}


static int trigger_listener_plugins(gfal2_context_t context, gfalt_params_t params, GError** error)
{
    GError * tmp_err = NULL;
    plugin_pointer_handle start_list, p_list;
    start_list = p_list = gfal_plugins_list_handler(context, &tmp_err);

    if (tmp_err != NULL) {
        gfal2_propagate_prefixed_error(error, tmp_err, __func__);
        return -1;
    }

    while (p_list->plugin_api != NULL) {
        if (p_list->plugin_api->copy_enter_hook)
            p_list->plugin_api->copy_enter_hook(p_list->plugin_api->plugin_data, context, params);
        p_list++;
    }

    g_free(start_list);
    return 0;
}


static int perform_copy(gfal2_context_t context, gfalt_params_t params, const char* src,
        const char* dst, GError** error)
{
    gfal_log(GFAL_VERBOSE_TRACE, " -> Gfal::Transfer::FileCopy");
    GError *tmp_err = NULL;
    int res = -1;

    if (trigger_listener_plugins(context, params, &tmp_err) < 0) {
        gfal2_propagate_prefixed_error(error, tmp_err, __func__);
        return -1;
    }

    void *plugin_data = NULL;
    plugin_filecopy_call p_copy = find_copy_plugin(context, GFAL_FILE_COPY, src, dst,
            &plugin_data, &tmp_err);

    if (tmp_err == NULL) {
        if (p_copy == NULL) {
            if (gfalt_get_local_transfer_perm(params, NULL)) {
                res = perform_local_copy(context, params, src, dst, &tmp_err);
            }
            else {
                gfal2_set_error(error, scope_copy_domain(), EPROTONOSUPPORT, __func__,
                        "No plugin supports a transfer from %s to %s, and local streaming is disabled",
                        src, dst);
                res = -1;
            }
        }
        else {
            res = p_copy(plugin_data, context, params, src, dst, &tmp_err);
        }
    }

    gfal_log(GFAL_VERBOSE_TRACE, " <- Gfal::Transfer::FileCopy");

    if (tmp_err != NULL)
        gfal2_propagate_prefixed_error(error, tmp_err, __func__);
    return res;
}


static void set_checksum(gfalt_params_t params, const char* checksum)
{
    if (checksum == NULL) {
        gfalt_set_checksum_check(params, FALSE, NULL);
        gfalt_set_user_defined_checksum(params, NULL, NULL, NULL);
    }
    else {
        gfalt_set_checksum_check(params, TRUE, NULL);

        const char* colon = strchr(checksum, ':');
        if (colon == NULL) {
            char chktype[64], chkvalue[1];
            gfalt_get_user_defined_checksum(params, chktype, sizeof(chktype), chkvalue,
                    sizeof(chkvalue), NULL);
            gfalt_set_user_defined_checksum(params, chktype, checksum, NULL);
        }
        else {
            char chktype[64];
            size_t chktype_len = colon - checksum;
            g_strlcpy(chktype, checksum, chktype_len < 64 ? chktype_len : 64);
            gfalt_set_user_defined_checksum(params, chktype, colon + 1, NULL);
        }
    }
}


static int bulk_fallback(gfal2_context_t context, gfalt_params_t params, size_t nbfiles,
        const char* const * srcs, const char* const * dsts, const char* const * checksums,
        GError** op_error, GError*** file_errors)
{
    *file_errors = g_new0(GError*, nbfiles);
    int ret = 0;
    size_t i;
    for (i = 0; i < nbfiles; ++i) {
        if (checksums) {
            const char* checksum = checksums[i];
            set_checksum(params, checksum);
        }
        else {
            set_checksum(params, NULL);
        }

        int subret = perform_copy(context, params, srcs[i], dsts[i], &(*file_errors)[i]);
        if (subret < 0)
            ret -= 1;
    }
    return ret;
}


static int perform_bulk_copy(gfal2_context_t context, gfalt_params_t params,
        size_t nbfiles, const char* const * srcs, const char* const * dsts,
        const char* const * checksums, GError** op_error, GError*** file_errors)
{
    GError* tmp_err = NULL;
    int res = -1;

    gfal_log(GFAL_VERBOSE_TRACE, " -> Gfal::Transfer::BulkFileCopy");

    if (trigger_listener_plugins(context, params, &tmp_err) < 0) {
        gfal2_propagate_prefixed_error(op_error, tmp_err, __func__);
        return -1;
    }


    void *plugin_data = NULL;
    plugin_filecopy_bulk_call p_copy = find_copy_plugin(context, GFAL_BULK_COPY, srcs[0],
            dsts[0], &plugin_data, &tmp_err);

    if (tmp_err == NULL) {
        if (p_copy == NULL) {
            res = bulk_fallback(context, params, nbfiles, srcs, dsts, checksums, op_error,
                    file_errors);
        }
        else {
            res = p_copy(plugin_data, context, params, nbfiles, srcs, dsts, checksums,
                    op_error, file_errors);
        }
    }

    gfal_log(GFAL_VERBOSE_TRACE, " <- Gfal::Transfer::BulkFileCopy");

    if (tmp_err != NULL)
        gfal2_propagate_prefixed_error(op_error, tmp_err, __func__);
    return res;
}


gfalt_transfer_status_t gfalt_transfer_status_create(
        const gfalt_hook_transfer_plugin_t * hook)
{
    gfalt_transfer_status_t state = g_new0(struct _gfalt_transfer_status, 1);
    state->hook = hook;
    return state;
}


void gfalt_transfer_status_delete(gfalt_transfer_status_t state)
{
    if (state)
        g_free(state);
}


int gfalt_copy_file(gfal2_context_t handle, gfalt_params_t params, const char* src,
        const char* dst, GError** err)
{
    g_return_val_err_if_fail(handle && src && dst, -1, err,
            "invalid source or/and destination values");
    gfalt_params_t p = NULL;

    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);

    int ret = -1;
    GError* nested_error = NULL;
    if (params == NULL) {
        p = gfalt_params_handle_new(NULL);
        ret = perform_copy(handle, p, src, dst, &nested_error);
    }
    else {
        ret = perform_copy(handle, params, src, dst, &nested_error);
    }
    gfalt_params_handle_delete(p, NULL);

    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(ret, nested_error, err);
}


int gfalt_copy_bulk(gfal2_context_t context, gfalt_params_t params, size_t nbfiles,
        const char* const * srcs, const char* const * dsts, const char* const * checksums,
        GError** op_error, GError*** file_errors)
{
    g_return_val_err_if_fail(context && srcs && dsts, -1, op_error,
            "invalid source or/and destination values");
    gfalt_params_t p = NULL;

    int ret = -1;
    if (params == NULL) {
        p = gfalt_params_handle_new(NULL);
        params = p;
    }

    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, op_error);

    ret = perform_bulk_copy(context, params, nbfiles, srcs, dsts, checksums, op_error,
            file_errors);
    gfalt_params_handle_delete(p, NULL);

    GFAL2_END_SCOPE_CANCEL(context);

    return ret;
}
