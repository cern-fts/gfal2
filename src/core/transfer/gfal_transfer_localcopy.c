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

#include <string.h>

#include <gfal_api.h>
#include <common/gfal_plugin_interface.h>
#include <checksums/checksums.h>
#include "gfal_transfer_plugins.h"
#include "gfal_transfer_internal.h"


const size_t DEFAULT_BUFFER_SIZE = 4194304;


static GQuark local_copy_domain() {
    return g_quark_from_static_string("GFAL2:CORE:COPY:LOCAL");
}


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
        gfalt_set_error(error, local_copy_domain(), EINVAL, __func__,
                GFALT_ERROR_DESTINATION, GFALT_ERROR_PARENT, "Could not get the parent directory of %s", surl);
        return -1;
    }

    GError* nested_error = NULL;
    struct stat st;
    if (gfal2_stat(context, parent, &st, &nested_error) < 0) {
        if (nested_error->code != ENOENT) {
            gfal2_propagate_prefixed_error(error, nested_error, __func__);
            return -1;
        }
        g_clear_error(&nested_error);
    }
    else {
        return 0;
    }

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

    if (S_ISFIFO(st.st_mode) || S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISSOCK(st.st_mode)) {
        gfal2_log(G_LOG_LEVEL_MESSAGE, "%s is a special file (%o), so keep going", surl, S_IFMT & st.st_mode);
        return 0;
    }

    if (!gfalt_get_replace_existing_file(params,NULL)) {
        gfal2_set_error(error, local_copy_domain(), EEXIST, __func__, "The file exists and overwrite is not set");
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
    else {
        plugin_trigger_event(params, local_copy_domain(),
                GFAL_EVENT_DESTINATION, GFAL_EVENT_OVERWRITE_DESTINATION,
                "Deleted %s", surl);
    }

    return 0;
}


struct perf_data_t {
    time_t start, last_update, now;
    size_t done;
    size_t done_since_last_update;
};


static void send_performance_data(gfalt_params_t params, const char* src, const char* dst, const struct perf_data_t* perf)
{
    struct _gfalt_transfer_status status;

    time_t total_time = perf->now - perf->start;
    time_t inc_time = perf->now - perf->last_update;

    status.average_baudrate = (size_t)(perf->done / total_time);
    status.bytes_transfered = (size_t)(perf->done);
    status.instant_baudrate = (size_t)(perf->done_since_last_update / inc_time);
    status.transfer_time    = total_time;

    plugin_trigger_monitor(params, &status, src, dst);
}


static int streamed_copy(gfal2_context_t context, gfalt_params_t params,
        const char* src, const char* dst, GError** error)
{
    GError *nested_error = NULL;

    plugin_trigger_event(params, local_copy_domain(),
            GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_ENTER,
            "%s => %s", src, dst);

    size_t alignment = gfal2_get_opt_integer_with_default(context, "CORE", "COPY_BUFFER_ALIGNMENT", 512);
    size_t buffersize = gfal2_get_opt_integer_with_default(context, "CORE", "COPY_BUFFERSIZE", DEFAULT_BUFFER_SIZE);
    char *buffer;
    errno = posix_memalign((void**)&buffer, alignment, buffersize);
    if (errno) {
        g_set_error(error, local_copy_domain(), errno, "Failed to allocate aligned buffer");
        return -1;
    }

    int src_open_flags = O_RDONLY;

#ifdef O_DIRECT
    gboolean direct_io = gfal2_get_opt_boolean_with_default(context, "CORE", "COPY_DIRECT_IO", FALSE);

    if (direct_io) {
        src_open_flags |= O_DIRECT;
        gfal2_log(G_LOG_LEVEL_DEBUG, " open src file with direct io : %s ", src);
    }
    else {
        gfal2_log(G_LOG_LEVEL_DEBUG, " open src file : %s ", src);
    }
#endif

    gfal_file_handle f_src = gfal_plugin_openG(context, src, src_open_flags, 0, &nested_error);
    if (nested_error) {
        free(buffer);
        gfal2_propagate_prefixed_error_extended(error, nested_error, __func__, "Could not open source: ");
        return -1;
    }

    int dst_open_flags = O_WRONLY | O_CREAT;

#ifdef O_DIRECT
    if (direct_io) {
        dst_open_flags |= O_DIRECT;
        gfal2_log(G_LOG_LEVEL_DEBUG, " open dst file with direct io : %s ", dst);
    }
    else {
        gfal2_log(G_LOG_LEVEL_DEBUG, " open dst file : %s ", dst);
    }
#endif

    gfal_file_handle f_dst = gfal_plugin_openG(context, dst, dst_open_flags, 0755, &nested_error);
    if (nested_error) {
        free(buffer);
        gfal_plugin_closeG(context, f_src, NULL);
        gfal2_propagate_prefixed_error_extended(error, nested_error, __func__, "Could not open destination: ");
        return -1;
    }

    struct perf_data_t perf_data;
    perf_data.start = perf_data.now = perf_data.last_update = time(NULL);
    perf_data.done = perf_data.done_since_last_update = 0;

    const time_t timeout = perf_data.start + gfalt_get_timeout(params, NULL);
    ssize_t s_file = 1;

    gfal2_log(G_LOG_LEVEL_DEBUG, "  begin local transfer %s ->  %s with buffer size %ld", src, dst, buffersize);

    while (s_file > 0 && !nested_error) {
        s_file = gfal_plugin_readG(context, f_src, buffer, buffersize, &nested_error);
        if (s_file > 0) {
            gfal_plugin_writeG(context, f_dst, buffer, s_file, &nested_error);
        }

        perf_data.done += s_file;
        perf_data.done_since_last_update += s_file;

        // Make sure we don't have to cancel
        if (gfal2_is_canceled(context)) {
            if (nested_error == NULL)
                g_set_error(&nested_error, local_copy_domain(), ECANCELED, "Transfer canceled");
        }
        // Timed-out?
        else {
            perf_data.now = time(NULL);
            if (perf_data.now >= timeout) {
                if (nested_error == NULL)
                    g_set_error(&nested_error, local_copy_domain(), ETIMEDOUT, "Transfer canceled because the timeout expired");
            }
            else if (perf_data.now - perf_data.last_update > 5) {
                send_performance_data(params, src, dst, &perf_data);
                perf_data.done_since_last_update = 0;
                perf_data.last_update = perf_data.now;
            }
        }
    }
    free(buffer);

    gfal_plugin_closeG(context, f_dst, (nested_error)?NULL:(&nested_error));
    gfal_plugin_closeG(context, f_src, (nested_error)?NULL:(&nested_error));

    if (nested_error) {
        gfal2_propagate_prefixed_error(error, nested_error, __func__);
        return -1;
    }
    else {
        plugin_trigger_event(params, local_copy_domain(), GFAL_EVENT_NONE,
                GFAL_EVENT_TRANSFER_EXIT, "%s => %s", src, dst);
        return 0;
    }
}


int perform_local_copy(gfal2_context_t context, gfalt_params_t params,
        const char* src, const char* dst, GError** error)
{
    GError* nested_error = NULL;
    gfal2_log(G_LOG_LEVEL_DEBUG, " -> Gfal::Transfer::start_local_copy ");

    char checksum_type[1024] = {0};
    char user_checksum[1024] = {0};
    char source_checksum[1024] = {0};
    gboolean is_strict_mode = gfalt_get_strict_copy_mode(params, NULL);
    gfalt_checksum_mode_t checksum_mode = GFALT_CHECKSUM_NONE;

    if (!is_strict_mode) {
        checksum_mode = gfalt_get_checksum(params,
            checksum_type, sizeof(checksum_type),
            user_checksum, sizeof(user_checksum),
            error);
    }

    // Source checksum
    if (checksum_mode & GFALT_CHECKSUM_SOURCE) {
        plugin_trigger_event(params, local_copy_domain(), GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_ENTER, "");
        gfal2_checksum(context, src, checksum_type, 0, 0, source_checksum, sizeof(source_checksum), &nested_error);
        if (nested_error != NULL) {
            gfal2_propagate_prefixed_error_extended(error, nested_error, __func__, "Could not get the source checksum: ");
            return -1;
        }
        plugin_trigger_event(params, local_copy_domain(), GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_EXIT, "");
    }

    if (user_checksum[0] && source_checksum[0]) {
        if (gfal_compare_checksums(user_checksum, source_checksum, 1024) != 0) {
            gfalt_set_error(error, local_copy_domain(), EIO, __func__,
                    GFALT_ERROR_SOURCE, GFALT_ERROR_CHECKSUM_MISMATCH,
                    "Source checksum and user-specified checksum do not match: %s != %s", source_checksum, user_checksum);
            return -1;
        }
    }

    if (!is_strict_mode) {
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
    }

    // Do the transfer
    streamed_copy(context, params, src, dst, &nested_error);
    if (nested_error != NULL) {
        gfal2_propagate_prefixed_error(error, nested_error, __func__);
        return -1;
    }

    // Destination checksum
    char *compare_against = user_checksum;
    char *compare_side = "User defined";
    if (user_checksum[0] == '\0') {
        compare_against = source_checksum;
        compare_side = "Source";
    }

    if (checksum_mode & GFALT_CHECKSUM_TARGET) {
        char destination_checksum[1024];

        plugin_trigger_event(params, local_copy_domain(), GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_ENTER, "");

        gfal2_checksum(context, dst, checksum_type, 0, 0, destination_checksum, sizeof(destination_checksum), &nested_error);
        if (nested_error != NULL) {
            gfal2_propagate_prefixed_error_extended(error, nested_error, __func__, "Could not get the destination checksum: ");
            return -1;
        }
        plugin_trigger_event(params, local_copy_domain(), GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_EXIT, "");

        if (gfal_compare_checksums(compare_against, destination_checksum, 1204) != 0) {
            gfalt_set_error(error, local_copy_domain(), EIO, __func__,
                GFALT_ERROR_DESTINATION, GFALT_ERROR_CHECKSUM_MISMATCH,
                "%s checksum and destination checksum do not match: %s != %s",
                compare_side, compare_against, destination_checksum);
            return -1;
        }
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, " <- Gfal::Transfer::start_local_copy");
    return 0;
}
