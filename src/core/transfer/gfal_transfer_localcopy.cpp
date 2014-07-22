#include <string.h>

#include <gfal_api.h>
#include <common/gfal_common_err_helpers.h>
#include <common/gfal_common_plugin_interface.h>
#include <checksums/checksums.h>
#include "gfal_transfer_plugins.h"
#include "gfal_transfer_internal.h"

static GQuark local_copy_domain = g_quark_from_static_string("FileCopy::local_copy");
const size_t DEFAULT_BUFFER_SIZE = 4000000;

static char* get_parent(const char* url)
{
    char *parent = g_strdup(url);
    char *slash = strrchr(parent, '/');
    if (slash) {
        *slash = '\0';
    }
    else {
        g_free(parent);
        parent = NULL;
    }
    return parent;
}


static int create_parent(gfal2_context_t context, gfalt_params_t params,
        const char* surl, GError** error)
{
    if (!gfalt_get_create_parent_dir(params, NULL))
        return 0;

    char *parent = get_parent(surl);
    if (!parent) {
        gfalt_set_error(error, local_copy_domain, EINVAL, __func__,
                GFALT_ERROR_DESTINATION, GFALT_ERROR_PARENT, "Could not get the parent directory of %s", surl);
        return -1;
    }

    GError* nested_error = NULL;
    gfal2_mkdir_rec(context, parent, 0755, &nested_error);
    if (nested_error != NULL) {
        gfal2_propagate_prefixed_error(error, nested_error, __func__);
        return -1;
    }

    return 0;
}


static int unlink_if_exists(gfal2_context_t context, gfalt_params_t params,
        const char* surl, GError** error)
{
    GError* nested_error = NULL;
    struct stat st;
    if (gfal2_stat(context, surl, &st, &nested_error) != 0) {
        if (nested_error->code == ENOENT) {
            g_error_free(nested_error);
            return 0;
        }
        gfal2_propagate_prefixed_error(error, nested_error, __func__);
        return -1;
    }

    if (!gfalt_get_replace_existing_file(params,NULL)) {
        gfal2_set_error(error, local_copy_domain, EEXIST, __func__, "The file exists and overwrite is not set");
        return -1;
    }

    gfal2_unlink(context, surl, &nested_error);
    if (nested_error != NULL) {
        if (nested_error->code != ENOENT) {
            gfal2_propagate_prefixed_error(error, nested_error, __func__);
            return -1;
        }
        g_error_free(nested_error);
    }

    return 0;
}

struct perf_data_t {
    time_t start, last_update, now;
    size_t done;
    size_t done_since_last_update;
};

static void send_performance_data(gfalt_params_t params, const char* src, const char* dst, const perf_data_t& perf)
{
    gfalt_monitor_func callback = gfalt_get_monitor_callback(params, NULL);
    gpointer callback_data = gfalt_get_user_data(params, NULL);

    if (!callback)
        return;

    gfalt_hook_transfer_plugin_t hook;

    time_t total_time = perf.now - perf.start;
    time_t inc_time = perf.now - perf.last_update;

    hook.average_baudrate = static_cast<size_t>(perf.done / total_time);
    hook.bytes_transfered = static_cast<size_t>(perf.done);
    hook.instant_baudrate = static_cast<size_t>(perf.done_since_last_update / inc_time);
    hook.transfer_time    = total_time;

    gfalt_transfer_status_t state = gfalt_transfer_status_create(&hook);
    callback(state, src, dst, callback_data);
    gfalt_transfer_status_delete(state);
}


static int streamed_copy(gfal2_context_t context, gfalt_params_t params,
        const char* src, const char* dst, GError** error)
{
    GError *nested_error = NULL;

    gfal_log(GFAL_VERBOSE_TRACE, " open src file : %s ", src);
    gfal_file_handle f_src = gfal_plugin_openG(context, src, O_RDONLY, 0, &nested_error);
    if (nested_error) {
        gfal2_propagate_prefixed_error_extended(error, nested_error, __func__, "Could not open source: ");
        return -1;
    }

    gfal_file_handle f_dst = gfal_plugin_openG(context, dst, O_WRONLY | O_CREAT, 0755, &nested_error);
    if (nested_error) {
        gfal_plugin_closeG(context, f_src, NULL);
        gfal2_propagate_prefixed_error_extended(error, nested_error, __func__, "Could not open destination: ");
        return -1;
    }

    perf_data_t perf_data;
    perf_data.start = perf_data.now = perf_data.last_update = time(NULL);
    perf_data.done = perf_data.done_since_last_update = 0;

    const time_t timeout = perf_data.start + gfalt_get_timeout(params, NULL);
    ssize_t s_file = 1;
    char buff[DEFAULT_BUFFER_SIZE];

    gfal_log(GFAL_VERBOSE_TRACE, "  begin local transfer %s ->  %s with buffer size %ld", src, dst, sizeof(buff));

    while (s_file > 0 && !nested_error) {
        s_file = gfal_plugin_readG(context, f_src, buff, sizeof(buff), &nested_error);
        if (s_file > 0)
            gfal_plugin_writeG(context, f_dst, buff, s_file, &nested_error);

        perf_data.done += s_file;
        perf_data.done_since_last_update += s_file;

        // Make sure we don't have to cancel
        if (gfal2_is_canceled(context))
            g_set_error(&nested_error, local_copy_domain, ECANCELED, "Transfer canceled");
        // Timed-out?
        else {
            perf_data.now = time(NULL);
            if (perf_data.now >= timeout) {
                g_set_error(&nested_error, local_copy_domain, ETIMEDOUT, "Transfer canceled because the timeout expired");
            }
            else if (perf_data.now - perf_data.last_update > 5) {
                send_performance_data(params, src, dst, perf_data);
                perf_data.done_since_last_update = 0;
                perf_data.last_update = perf_data.now;
            }
        }
    }

    gfal_plugin_closeG(context, f_dst, (nested_error)?NULL:(&nested_error));
    gfal_plugin_closeG(context, f_src, (nested_error)?NULL:(&nested_error));

    if (nested_error) {
        gfal2_propagate_prefixed_error(error, nested_error, __func__);
        return -1;
    }
    else {
        return 0;
    }
}


int perform_local_copy(gfal2_context_t context, gfalt_params_t params,
        const char* src, const char* dst, GError** error)
{
    GError* nested_error = NULL;
    gfal_log(GFAL_VERBOSE_TRACE, " -> Gfal::Transfer::start_local_copy ");

    char checksum_type[1024];
    char user_checksum[1024];
    char source_checksum[1024];
    gboolean is_strict_mode = gfalt_get_strict_copy_mode(params, NULL);
    gboolean is_checksum_enabled = !is_strict_mode && gfalt_get_checksum_check(params, NULL);

    // Source checksum
    if (is_checksum_enabled) {
        plugin_trigger_event(params, local_copy_domain, GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_ENTER, "");
        gfalt_get_user_defined_checksum(params,
                                        checksum_type, sizeof(checksum_type),
                                        user_checksum, sizeof(user_checksum),
                                        NULL);
        if (checksum_type[0] == '\0')
            strncpy(checksum_type, "ADLER32", sizeof(checksum_type));

        gfal2_checksum(context, src, checksum_type, 0, 0, source_checksum, sizeof(source_checksum), &nested_error);
        if (nested_error != NULL) {
            gfal2_propagate_prefixed_error_extended(error, nested_error, __func__, "Could not get the source checksum: ");
            return -1;
        }

        if (user_checksum[0] && gfal_compare_checksums(user_checksum, source_checksum, 1024) != 0) {
            gfal2_set_error(error, local_copy_domain, EIO, __func__,
                    "Source checksum and user-specified checksum do not match: %s != %s", source_checksum, user_checksum);
            return -1;
        }

        plugin_trigger_event(params, local_copy_domain, GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_EXIT, "");
    }

    // Parent directory
    create_parent(context, params, dst, &nested_error);
    if (nested_error != NULL) {
        gfal2_propagate_prefixed_error(error, nested_error, __func__);
        return -1;
    }

    // Remove if exists and overwrite is set
    if (!is_strict_mode) {
        unlink_if_exists(context, params, dst, &nested_error);
        if (nested_error != NULL) {
            gfal2_propagate_prefixed_error(error, nested_error, __func__);
            return -1;
        }
    }

    // Do the transfer
    streamed_copy(context, params, src, dst, &nested_error);
    if (nested_error != NULL) {
        gfal2_propagate_prefixed_error(error, nested_error, __func__);
        return -1;
    }

    // Destination checksum
    if (is_checksum_enabled) {
        char destination_checksum[1024];

        plugin_trigger_event(params, local_copy_domain, GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_ENTER, "");

        gfal2_checksum(context, dst, checksum_type, 0, 0, destination_checksum, sizeof(destination_checksum), &nested_error);
        if (nested_error != NULL) {
            gfal2_propagate_prefixed_error_extended(error, nested_error, __func__, "Could not get the destination checksum: ");
            return -1;
        }

        if (gfal_compare_checksums(source_checksum, destination_checksum, 1204) != 0) {
            gfal2_set_error(error, local_copy_domain, EIO, __func__,
                    "Source checksum and destination checksum do not match: %s != %s", source_checksum, destination_checksum);
            return -1;
        }

        plugin_trigger_event(params, local_copy_domain, GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_EXIT, "");
    }

    gfal_log(GFAL_VERBOSE_TRACE, " <- Gfal::Transfer::start_local_copy ");
    return 0;
}
