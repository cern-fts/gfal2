/*
* Copyright @ CERN, 2014.
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
#include <string>
#include <vector>

#include "gridftp_filecopy.h"
#include "gridftpwrapper.h"
#include "gridftp_namespace.h"
#include "gridftp_plugin.h"

#include <globus_ftp_client_throughput_plugin.h>
#include <checksums/checksums.h>


static const GQuark GSIFTP_BULK_DOMAIN = g_quark_from_static_string("GridFTP::Filecopy");


struct GridFTPBulkData {
    GridFTPBulkData(size_t nbfiles) :
            srcs(NULL), dsts(NULL), checksums(nbfiles),
            errn(new int[nbfiles]), fsize(new off_t[nbfiles]),
            index(0), nbfiles(nbfiles), started(new bool[nbfiles]),
            params(NULL), error(NULL), done(false)
    {
        for (size_t i = 0; i < nbfiles; ++i) {
            started[i] = false;
            errn[i] = 0;
            fsize[i] = 0;
        }
        globus_mutex_init(&lock, GLOBUS_NULL);
        globus_cond_init(&cond, GLOBUS_NULL);
    }

    ~GridFTPBulkData() {
        delete [] started;
        delete [] errn;
        delete [] fsize;
        if (error)
            globus_object_free(error);
    }

    const char* const* srcs;
    const char* const* dsts;
    std::vector<std::string> checksums;
    int* errn;
    off_t* fsize;

    size_t index, nbfiles;
    bool *started;

    gfalt_params_t params;

    globus_mutex_t lock;
    globus_cond_t cond;
    globus_object_t* error;
    bool done;
};


struct GridFTPBulkPerformance {
    std::string source, destination;
    gfalt_params_t params;
    bool ipv6;
    time_t start_time;

    globus_ftp_client_plugin_t* plugin;
};


// Called by Globus when done
static void gridftp_done_callback(void * user_arg, globus_ftp_client_handle_t * handle,
        globus_object_t * err)
{
    GridFTPBulkData* data = static_cast<GridFTPBulkData*>(user_arg);

    if (err) {
        data->error = globus_object_copy(err);
    }
    else {
        for (size_t i = 0; i < data->nbfiles; ++i) {
            if (data->started[i]) {
                plugin_trigger_event(data->params, GSIFTP_BULK_DOMAIN, GFAL_EVENT_NONE,
                        GFAL_EVENT_TRANSFER_EXIT,
                        "Done %s => %s", data->srcs[i], data->dsts[i]);
            }
        }
    }

    globus_mutex_lock(&data->lock);
    data->done = true;
    globus_cond_signal(&data->cond);
    globus_mutex_unlock(&data->lock);
}


// Called by Globus when ready for a new pair
static void gridftp_pipeline_callback(globus_ftp_client_handle_t * handle, char ** source_url,
        char ** dest_url, void * user_arg)
{
    GridFTPBulkData* data = static_cast<GridFTPBulkData*>(user_arg);

    // Next
    data->index++;

    // Skip pairs marked as failed
    while (data->index < data->nbfiles && data->errn[data->index]) {
        gfal2_log(G_LOG_LEVEL_DEBUG,
                "Skipping pair %d as marked failed with %d", data->index,
                data->errn[data->index]);
        data->index++;
    }

    // Return next pair
    if (data->index < data->nbfiles) {
        *source_url = (char*)data->srcs[data->index];
        *dest_url = (char*)data->dsts[data->index];
        data->started[data->index] = true;

        gfal2_log(G_LOG_LEVEL_INFO, "Providing pair %s => %s", *source_url, *dest_url);
    }
    else {
        *source_url = NULL;
        *dest_url = NULL;

        gfal2_log(G_LOG_LEVEL_INFO, "No more pairs to give");
    }
}


static
void gridftp_bulk_cancel(gfal2_context_t context, void* userdata)
{
    globus_ftp_client_handle_t* ftp_handle = static_cast<globus_ftp_client_handle_t*>(userdata);
    globus_ftp_client_abort(ftp_handle);
}


static void gridftp_bulk_begin_cb(void * user_specific,
        globus_ftp_client_handle_t * handle,
        const char * source_url,
        const char * dest_url)
{
    GridFTPBulkPerformance* original = static_cast<GridFTPBulkPerformance*>(user_specific);
    GridFTPBulkPerformance* pd;
    globus_ftp_client_throughput_plugin_get_user_specific(original->plugin, (void**)(&pd));

    pd->source = source_url;
    pd->destination = dest_url;
    pd->start_time = time(NULL);

    plugin_trigger_event(pd->params, GSIFTP_BULK_DOMAIN, GFAL_EVENT_NONE,
            GFAL_EVENT_TRANSFER_ENTER,
            "(%s) %s => (%s) %s",
            return_hostname(source_url, pd->ipv6).c_str(), source_url,
            return_hostname(dest_url, pd->ipv6).c_str(), dest_url);
}


static
void gridftp_bulk_throughput_cb(void *user_specific,
        globus_ftp_client_handle_t *handle, globus_off_t bytes,
        float instantaneous_throughput, float avg_throughput)
{
    GridFTPBulkPerformance* original = static_cast<GridFTPBulkPerformance*>(user_specific);
    GridFTPBulkPerformance* pd;
    globus_ftp_client_throughput_plugin_get_user_specific(original->plugin, (void**)(&pd));

    gfalt_hook_transfer_plugin_t hook;
    hook.bytes_transfered = bytes;
    hook.average_baudrate = (size_t) avg_throughput;
    hook.instant_baudrate = (size_t) instantaneous_throughput;
    hook.transfer_time = (time(NULL) - pd->start_time);

    gfalt_transfer_status_t state = gfalt_transfer_status_create(&hook);
    plugin_trigger_monitor(pd->params, state, pd->source.c_str(), pd->destination.c_str());
    gfalt_transfer_status_delete(state);
}


static
void gridftp_bulk_complete_cb(void * user_specific,
        globus_ftp_client_handle_t * handle, globus_bool_t success)
{
    GridFTPBulkPerformance* original = static_cast<GridFTPBulkPerformance*>(user_specific);
    GridFTPBulkPerformance* pd;
    globus_ftp_client_throughput_plugin_get_user_specific(original->plugin, (void**)(&pd));
}


static
void* gridftp_bulk_copy_perf_cb(void * user_specific)
{
    GridFTPBulkPerformance* pd = static_cast<GridFTPBulkPerformance*>(user_specific);
    return new GridFTPBulkPerformance(*pd);
}


static
void gridftp_bulk_destroy_perf_cb(void * user_specific)
{
    GridFTPBulkPerformance* pd = static_cast<GridFTPBulkPerformance*>(user_specific);
    delete pd;
}


static
int gridftp_pipeline_transfer(plugin_handle plugin_data,
        gfal2_context_t context, bool udt, GridFTPBulkData* pairs, GError** op_error)
{
    GridFTPModule* gsiftp = static_cast<GridFTPModule*>(plugin_data);
    GridFTPSessionHandler handler(gsiftp->get_session_factory(), pairs->srcs[0]);

    globus_ftp_client_plugin_t throughput_plugin;
    globus_ftp_client_handle_t ftp_handle;
    globus_ftp_client_operationattr_t ftp_operation_attr;
    globus_ftp_client_handleattr_t* ftp_handle_attr = handler.get_ftp_client_handleattr();

    // First viable pair goes with the globus call
    pairs->index = 0;
    while (pairs->index < pairs->nbfiles && pairs->errn[pairs->index])
        pairs->index++;
    if (pairs->index >= pairs->nbfiles)
        return 0;

    pairs->started[pairs->index] = true;

    GridFTPBulkPerformance perf;
    perf.params = pairs->params;
    perf.ipv6 = gfal2_get_opt_boolean_with_default(context, GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_IPV6, false);
    perf.plugin = &throughput_plugin;

    globus_ftp_client_throughput_plugin_init(&throughput_plugin,
            gridftp_bulk_begin_cb, NULL, gridftp_bulk_throughput_cb, gridftp_bulk_complete_cb,
            &perf);
    globus_ftp_client_throughput_plugin_set_copy_destroy(&throughput_plugin,
            gridftp_bulk_copy_perf_cb, gridftp_bulk_destroy_perf_cb);
    globus_ftp_client_handleattr_add_plugin(ftp_handle_attr, &throughput_plugin);

    globus_ftp_client_handleattr_set_pipeline(ftp_handle_attr, 0, gridftp_pipeline_callback, pairs);
    globus_ftp_client_handle_init(&ftp_handle, ftp_handle_attr);
    globus_ftp_client_operationattr_init(&ftp_operation_attr);
    globus_ftp_client_operationattr_copy(&ftp_operation_attr, handler.get_ftp_client_operationattr());
    globus_ftp_client_operationattr_set_mode(&ftp_operation_attr, GLOBUS_FTP_CONTROL_MODE_EXTENDED_BLOCK);
    globus_ftp_client_operationattr_set_delayed_pasv(&ftp_operation_attr, GLOBUS_FALSE);

    if (udt)
        globus_ftp_client_operationattr_set_net_stack(&ftp_operation_attr, "udt");
    else
        globus_ftp_client_operationattr_set_net_stack(&ftp_operation_attr, "default");

    int nbstreams = gfalt_get_nbstreams(pairs->params, NULL);
    guint64 buffer_size = gfalt_get_tcp_buffer_size(pairs->params, NULL);
    globus_ftp_control_parallelism_t parallelism;
    globus_ftp_control_tcpbuffer_t tcp_buffer_size;

    if (nbstreams > 1) {
        parallelism.fixed.size = nbstreams;
        parallelism.mode = GLOBUS_FTP_CONTROL_PARALLELISM_FIXED;
        globus_ftp_client_operationattr_set_mode(&ftp_operation_attr, GLOBUS_FTP_CONTROL_MODE_EXTENDED_BLOCK);
        globus_ftp_client_operationattr_set_parallelism(&ftp_operation_attr, &parallelism);
    }
    if (buffer_size > 0) {
        tcp_buffer_size.mode = GLOBUS_FTP_CONTROL_TCPBUFFER_FIXED;
        tcp_buffer_size.fixed.size = buffer_size;
        globus_ftp_client_operationattr_set_tcp_buffer(&ftp_operation_attr, &tcp_buffer_size);
    }


    gfal_cancel_token_t cancel_token;
    cancel_token = gfal2_register_cancel_callback(context, gridftp_bulk_cancel, ftp_handle);

    int res = 0;
    try {
        globus_result_t globus_return;

        globus_return = globus_ftp_client_third_party_transfer(&ftp_handle,
                pairs->srcs[pairs->index], &ftp_operation_attr,
                pairs->dsts[pairs->index], &ftp_operation_attr,
                GLOBUS_NULL, gridftp_done_callback, pairs);
        gfal_globus_check_result(GSIFTP_BULK_DOMAIN, globus_return);

        globus_mutex_lock(&pairs->lock);

        guint64 timeout = gfalt_get_timeout(pairs->params, NULL);
        globus_abstime_t timeout_expires;
        GlobusTimeAbstimeGetCurrent(timeout_expires);
        timeout_expires.tv_sec += timeout;

        int wait_ret = 0;
        while (!pairs->done && wait_ret != ETIMEDOUT) {
            if (timeout > 0)
                wait_ret = globus_cond_timedwait(&pairs->cond, &pairs->lock, &timeout_expires);
            else
                wait_ret = globus_cond_wait(&pairs->cond, &pairs->lock);
        }
        globus_mutex_unlock(&pairs->lock);

        if (pairs->error) {
            char *err_buffer;
            int err_code = gfal_globus_error_convert(pairs->error, &err_buffer);
            if (err_code) {
                gfal2_log(G_LOG_LEVEL_INFO, "Bulk transfer failed with %s", err_buffer);
                gfal2_set_error(op_error, GSIFTP_BULK_DOMAIN, err_code, __func__, "%s", err_buffer);
                res = -1;
                g_free(err_buffer);
            }
        }
        else if (wait_ret == ETIMEDOUT) {
            gfal2_set_error(op_error, GSIFTP_BULK_DOMAIN, ETIMEDOUT, __func__, "Transfer timed out");
            res = -1;
        }
    }
    catch (const Gfal::CoreException& e) {
        gfal2_log(G_LOG_LEVEL_MESSAGE, "Bulk transfer failed with %s", e.what());
        gfal2_set_error(op_error, e.domain(), e.code(), __func__, "%s", e.what());
        res = -1;
    }

    gfal2_remove_cancel_callback(context, cancel_token);

    globus_ftp_client_handleattr_remove_plugin(ftp_handle_attr, &throughput_plugin);
    globus_ftp_client_throughput_plugin_destroy(&throughput_plugin);

    globus_ftp_client_handle_destroy(&ftp_handle);
    globus_ftp_client_operationattr_destroy(&ftp_operation_attr);
    globus_ftp_client_handleattr_set_pipeline(ftp_handle_attr, 0, NULL, NULL);

    return res;
}


static
int gridftp_bulk_check_sources(plugin_handle plugin_data, gfal2_context_t context,
        GridFTPBulkData* pairs, GError** file_errors)
{
    struct stat st;
    int nfailed = 0, ret = 0;
    char chk_type[32] = {0}, chk_value[128], dummy[1];
    gboolean validate_checksum = gfalt_get_checksum_check(pairs->params, NULL);

    gfalt_get_user_defined_checksum(pairs->params, chk_type, sizeof(chk_type), dummy, 0, NULL);

    for (size_t i = 0; i < pairs->nbfiles; ++i) {
        if (gfal2_is_canceled(context)) {
            gfal2_set_error(&(file_errors[i]), GSIFTP_BULK_DOMAIN, EINTR,
                    __func__, "Operation canceled");
            pairs->errn[i] = EINTR;
        }
        else if (gfal_gridftp_statG(plugin_data, pairs->srcs[i], &st,
                &(file_errors[i])) < 0) {
            pairs->errn[i] = file_errors[i]->code;
        }
        else if (S_ISDIR(st.st_mode)) {
            gfal2_set_error(&(file_errors[i]), GSIFTP_BULK_DOMAIN, EISDIR,
                    __func__, "File is a directory");
            pairs->errn[i] = EISDIR;
        }
        else {
            pairs->fsize[i] = st.st_size;

            if (validate_checksum) {
                plugin_trigger_event(pairs->params, GSIFTP_BULK_DOMAIN,
                                     GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_ENTER,
                                     "%s", pairs->srcs[i]);

                ret = gfal_gridftp_checksumG(plugin_data, pairs->srcs[i], chk_type,
                        chk_value, sizeof(chk_value), 0, 0, &(file_errors[i]));
                if (ret == 0) {
                    if (!pairs->checksums[i].empty()) {
                        if (gfal_compare_checksums(pairs->checksums[i].c_str(), chk_value, sizeof(chk_value)) != 0) {
                            gfalt_set_error(&(file_errors[i]), GSIFTP_BULK_DOMAIN, EIO,
                                    GFALT_ERROR_SOURCE, GFALT_ERROR_CHECKSUM_MISMATCH,
                                    __func__, "User checksum and source checksum do not match: %s != %s",
                                    pairs->checksums[i].c_str(), chk_value);
                            pairs->errn[i] = EIO;
                        }
                    }
                    else {
                        pairs->checksums[i] = chk_value;
                    }
                }
                else {
                    pairs->errn[i] = file_errors[i]->code;
                }

                plugin_trigger_event(pairs->params, GSIFTP_BULK_DOMAIN,
                        GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_EXIT, "%s",
                        pairs->srcs[i]);
            }
        }

        if (file_errors[i] != NULL)
            ++nfailed;
    }

    return nfailed;
}


static
int gridftp_bulk_prepare_destination(plugin_handle plugin_data,
        gfal2_context_t context, GridFTPBulkData* pairs, GError** file_errors)
{
    int nfailed = 0;
    std::vector<std::string> created_parents;

    for (size_t i = 0; i < pairs->nbfiles; ++i) {
        // May have failed when preparing the source!
        if (pairs->errn[i] == 0) {
            if (gfal2_is_canceled(context)) {
                gfal2_set_error(&(file_errors[i]), GSIFTP_BULK_DOMAIN, EINTR,
                        __func__, "Operation canceled");
                pairs->errn[i] = EINTR;
            }
            else {
                try {
                    const char* slash = strrchr(pairs->dsts[i], '/');
                    std::string parent;
                    if (slash)
                        parent.assign(pairs->dsts[i], 0, slash - pairs->dsts[i]);

                    gridftp_filecopy_delete_existing(
                            (GridFTPModule*) plugin_data, pairs->params,
                            pairs->dsts[i]);

                    if (slash &&
                        std::find(created_parents.begin(), created_parents.end(), parent) != created_parents.end()) {
                        gfal2_log(G_LOG_LEVEL_INFO, "Skip mkdir of %s", parent.c_str());
                    }
                    else {
                        gridftp_create_parent_copy((GridFTPModule*) plugin_data,
                                pairs->params, pairs->dsts[i]);
                        created_parents.push_back(parent);
                    }
                }
                catch (const Gfal::TransferException& e) {
                    gfal2_set_error(&(file_errors[i]), GSIFTP_BULK_DOMAIN, e.code(),
                            __func__, "%s", e.what());
                    pairs->errn[i] = e.code();
                }
            }

            if (file_errors[i] != NULL)
                ++nfailed;
        }
    }

    return nfailed;
}


static
int gridftp_bulk_prepare(plugin_handle plugin_data,
        gfal2_context_t context, GridFTPBulkData* pairs, GError** file_errors)
{
    plugin_trigger_event(pairs->params, GSIFTP_BULK_DOMAIN,
            GFAL_EVENT_NONE, GFAL_EVENT_PREPARE_ENTER, "");

    int src_failed = gridftp_bulk_check_sources(plugin_data, context, pairs, file_errors);
    int dst_failed = gridftp_bulk_prepare_destination(plugin_data, context, pairs, file_errors);

    plugin_trigger_event(pairs->params, GSIFTP_BULK_DOMAIN,
            GFAL_EVENT_NONE, GFAL_EVENT_PREPARE_EXIT, "");
    return src_failed + dst_failed;
}


static
int gridftp_bulk_close(plugin_handle plugin_data,
        gfal2_context_t context, GridFTPBulkData* pairs, GError** file_errors)
{
    int nfailed = 0, ret = 0;
    struct stat st;
    char chk_type[32] = {0}, chk_value[128], dummy[1];
    gboolean validate_checksum = gfalt_get_checksum_check(pairs->params, NULL);

    gfalt_get_user_defined_checksum(pairs->params, chk_type, sizeof(chk_type), dummy, 0, NULL);

    plugin_trigger_event(pairs->params, GSIFTP_BULK_DOMAIN,
            GFAL_EVENT_NONE, GFAL_EVENT_CLOSE_ENTER, "");

    for (size_t i = 0; i < pairs->nbfiles; ++i) {
        if (pairs->errn[i] == 0) {
            if (gfal2_is_canceled(context)) {
                gfal2_set_error(&(file_errors[i]), GSIFTP_BULK_DOMAIN, EINTR,
                        __func__, "Operation canceled");
                pairs->errn[i] = EINTR;
            }
            else if (gfal_gridftp_statG(plugin_data, pairs->dsts[i], &st,
                    &(file_errors[i])) < 0) {
                pairs->errn[i] = file_errors[i]->code;
            }
            else {
                if (pairs->fsize[i] != st.st_size) {
                    gfalt_set_error(&(file_errors[i]), GSIFTP_BULK_DOMAIN, EIO,
                            GFALT_ERROR_DESTINATION, GFALT_ERROR_SIZE_MISMATCH,
                            __func__, "Source and destination file sizes do not match: %lld != %lld",
                            (long long)pairs->fsize, (long long)st.st_size);
                    pairs->errn[i] = EIO;
                }
                else if (validate_checksum) {
                    plugin_trigger_event(pairs->params, GSIFTP_BULK_DOMAIN,
                            GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_ENTER, "%s",
                            pairs->dsts[i]);

                    ret = gfal_gridftp_checksumG(plugin_data, pairs->dsts[i],
                            chk_type, chk_value, sizeof(chk_value), 0, 0, &(file_errors[i]));
                    if (ret == 0) {
                        if (!pairs->checksums[i].empty()) {
                            if (gfal_compare_checksums(
                                    pairs->checksums[i].c_str(), chk_value,
                                    sizeof(chk_value)) != 0) {
                                gfalt_set_error(&(file_errors[i]), GSIFTP_BULK_DOMAIN, EIO, __func__,
                                        GFALT_ERROR_DESTINATION, GFALT_ERROR_CHECKSUM_MISMATCH,
                                        "Destination checksum do not match: %s != %s",
                                        pairs->checksums[i].c_str(), chk_value);
                                pairs->errn[i] = EIO;
                            }
                        }
                        else {
                            pairs->checksums[i] = chk_value;
                        }
                    }
                    else {
                        pairs->errn[i] = file_errors[i]->code;
                    }

                    plugin_trigger_event(pairs->params, GSIFTP_BULK_DOMAIN,
                            GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_EXIT, "%s",
                            pairs->srcs[i]);
                }
            }

            if (file_errors[i] != NULL)
                ++nfailed;
        }
    }

    plugin_trigger_event(pairs->params, GSIFTP_BULK_DOMAIN,
                GFAL_EVENT_NONE, GFAL_EVENT_CLOSE_EXIT, "");
    return nfailed;
}


int gridftp_bulk_copy(plugin_handle plugin_data, gfal2_context_t context, gfalt_params_t params,
        size_t nbfiles, const char* const * srcs, const char* const * dsts,
        const char* const * checksums, GError** op_error, GError*** file_errors)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, "-> %s", __func__);

    if (nbfiles == 0 || srcs == NULL || dsts == NULL) {
        gfal2_set_error(op_error, GSIFTP_BULK_DOMAIN, EINVAL, __func__, "Invalid parameters");
        return -1;
    }

    if (gfal2_start_scope_cancel(context, op_error) < 0)
        return -1;

    GridFTPBulkData pairs(nbfiles);
    pairs.srcs = srcs;
    pairs.dsts = dsts;
    if (checksums) {
        for (size_t i = 0; i < nbfiles; ++i)
            pairs.checksums[i] = checksums[i];
    }
    pairs.nbfiles = nbfiles;
    pairs.params = params;

    // Preparation stage
    *file_errors = g_new0(GError*, nbfiles);
    int total_failed = gridftp_bulk_prepare(plugin_data, context, &pairs, *file_errors);

    // Transfer
    int transfer_ret = -1;
    if (!gfal2_is_canceled(context)) {
        bool udt = gfal2_get_opt_boolean_with_default(context,
                GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_TRANSFER_UDT, false);

        transfer_ret = gridftp_pipeline_transfer(plugin_data, context, udt, &pairs, op_error);
        // If UDT was tried and it failed, give it another shot
        if (transfer_ret < 0 && strstr((*op_error)->message, "udt driver not whitelisted") && !gfal2_is_canceled(context)) {
            udt = false;
            pairs.done = false;
            globus_object_free(pairs.error);
            pairs.error = NULL;
            g_error_free(*op_error);
            *op_error = NULL;

            gfal2_log(G_LOG_LEVEL_INFO, "UDT transfer failed! Disabling and retrying...");
            transfer_ret = gridftp_pipeline_transfer(plugin_data, context, udt, &pairs, op_error);
        }
    }
    if (transfer_ret < 0)
        total_failed = nbfiles;

    // Check destinations
    if (transfer_ret == 0)
        total_failed += gridftp_bulk_close(plugin_data, context, &pairs, *file_errors);

    // Done
    gfal2_log(G_LOG_LEVEL_DEBUG, "<- %s", __func__);
    gfal2_end_scope_cancel(context);
    return -total_failed;
}
