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

#include <common/gfal_plugin.h>
#include <common/gfal_error.h>
#include <transfer/gfal_transfer_plugins.h>
#include <transfer/gfal_transfer_internal.h>
#include <common/gfal_cancel.h>

static GQuark scope_copy_domain() {
    return g_quark_from_static_string("GFAL2:CORE:COPY");
}


static gfal_plugin_interface* find_copy_plugin(gfal2_context_t context, gfal_url2_check operation,
        const char* src, const char* dst, void** plugin_data, GError** error)
{
    GList* item = g_list_first(context->plugin_opt.sorted_plugin);
    void* resu = NULL;

    while (item != NULL && resu == NULL) {
        gfal_plugin_interface* plugin_ifce = (gfal_plugin_interface*)item->data;
        if (plugin_ifce->check_plugin_url_transfer != NULL) {
            gboolean compatible;
            if ((compatible = plugin_ifce->check_plugin_url_transfer(plugin_ifce->plugin_data, context, src, dst,
                    operation)) == TRUE) {
                *plugin_data = plugin_ifce->plugin_data;
                resu = plugin_ifce;
            }
        }
        item = g_list_next(item);
    }

    return resu;
}


static int trigger_listener_plugins(gfal2_context_t context, gfalt_params_t params, GError** error)
{
    GList *item = g_list_first(context->plugin_opt.sorted_plugin);

    while (item != NULL) {
        gfal_plugin_interface* plugin_ifce = (gfal_plugin_interface*)item->data;
        if (plugin_ifce->copy_enter_hook) {
            GError* tmp_error = NULL;
            plugin_ifce->copy_enter_hook(plugin_ifce->plugin_data, context, params, &tmp_error);
            if (tmp_error) {
                gfal2_log(G_LOG_LEVEL_MESSAGE, "Copy enter hook failed: %s", tmp_error->message);
                g_error_free(tmp_error);
            }
        }
        item = g_list_next(item);
    }

    return 0;
}


static int notify_copy_list(gfal2_context_t context, gfalt_params_t params,
        size_t nbfiles, const char* const * srcs, const char* const * dsts,
        GError** error)
{
    plugin_trigger_event(params, scope_copy_domain(),
            GFAL_EVENT_NONE, GFAL_EVENT_LIST_ENTER, NULL);

    size_t i;
    for (i = 0; i < nbfiles; ++i) {
        gchar* src = g_markup_escape_text(srcs[i], -1);
        gchar* dst = g_markup_escape_text(dsts[i], -1);
        plugin_trigger_event(params, scope_copy_domain(),
                GFAL_EVENT_NONE, GFAL_EVENT_LIST_ITEM,
                "%s => %s", src, dst);
        g_free(src);
        g_free(dst);
    }

    plugin_trigger_event(params, scope_copy_domain(),
            GFAL_EVENT_NONE, GFAL_EVENT_LIST_EXIT, NULL);

    return 0;
}


static int perform_copy(gfal2_context_t context, gfalt_params_t params, const char* src,
        const char* dst, GError** error)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, " -> Gfal::Transfer::FileCopy");
    GError *tmp_err = NULL;
    int res = -1;

    if (trigger_listener_plugins(context, params, &tmp_err) < 0) {
        gfal2_propagate_prefixed_error(error, tmp_err, __func__);
        return -1;
    }
    if (notify_copy_list(context, params, 1, &src, &dst, &tmp_err)) {
        gfal2_propagate_prefixed_error(error, tmp_err, __func__);
        return -1;
    }

    void *plugin_data = NULL;
    gfal_plugin_interface* plugin = find_copy_plugin(context, GFAL_FILE_COPY, src, dst,
            &plugin_data, &tmp_err);

    if (tmp_err == NULL) {
        if (plugin == NULL) {
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
            res = plugin->copy_file(plugin_data, context, params, src, dst, &tmp_err);
        }
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, " <- Gfal::Transfer::FileCopy");

    if (tmp_err != NULL)
        gfal2_propagate_prefixed_error(error, tmp_err, __func__);
    return res;
}


static int set_checksum(gfalt_params_t params, const char* checksum, GError **err)
{
    gfalt_checksum_mode_t mode = gfalt_get_checksum_mode(params, err);
    if (*err) {
        return -1;
    }
    if (checksum == NULL) {
        return gfalt_set_checksum(params, mode, NULL, NULL, err);
    }
    else {
        const char* colon = strchr(checksum, ':');
        if (colon == NULL) {
            return gfalt_set_checksum(params, mode, NULL, checksum, err);
        }
        else {
            char chktype[64];
            size_t chktype_len = colon - checksum;
            g_strlcpy(chktype, checksum, chktype_len < 64 ? chktype_len : 64);
            return gfalt_set_checksum(params, mode, chktype, colon + 1, err);
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
        int subret = 0;

        if (checksums) {
            const char* checksum = checksums[i];
            subret = set_checksum(params, checksum, &(*file_errors)[i]);
        }
        else {
            subret = set_checksum(params, NULL, &(*file_errors)[i]);
        }
        if (subret < 0) {
            ret -= 1;
            continue;
        }

        subret = perform_copy(context, params, srcs[i], dsts[i], &(*file_errors)[i]);
        if (subret < 0) {
            ret -= 1;
        }
    }
    return ret;
}


static int perform_bulk_copy(gfal2_context_t context, gfalt_params_t params,
        size_t nbfiles, const char* const * srcs, const char* const * dsts,
        const char* const * checksums, GError** op_error, GError*** file_errors)
{
    GError* tmp_err = NULL;
    int res = -1;

    gfal2_log(G_LOG_LEVEL_DEBUG, " -> Gfal::Transfer::BulkFileCopy");

    if (trigger_listener_plugins(context, params, &tmp_err) < 0) {
        gfal2_propagate_prefixed_error(op_error, tmp_err, __func__);
        return -1;
    }
    if (notify_copy_list(context, params, nbfiles, srcs, dsts, &tmp_err)) {
        gfal2_propagate_prefixed_error(op_error, tmp_err, __func__);
        return -1;
    }

    void *plugin_data = NULL;
    gfal_plugin_interface *plugin = find_copy_plugin(context, GFAL_BULK_COPY, srcs[0],
            dsts[0], &plugin_data, &tmp_err);

    if (tmp_err == NULL) {
        if (plugin == NULL) {
            res = bulk_fallback(context, params, nbfiles, srcs, dsts, checksums, op_error,
                    file_errors);
        }
        else {
            res = plugin->copy_bulk(plugin_data, context, params, nbfiles, srcs, dsts, checksums,
                    op_error, file_errors);
        }
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, " <- Gfal::Transfer::BulkFileCopy");

    if (tmp_err != NULL)
        gfal2_propagate_prefixed_error(op_error, tmp_err, __func__);
    return res;
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
