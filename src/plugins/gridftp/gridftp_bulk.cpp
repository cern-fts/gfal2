#include "gridftp_filecopy.h"
#include "gridftpwrapper.h"
#include "gridftp_namespace.h"
#include <common/gfal_common_err_helpers.h>
#include <common/gfal_common_internal.h>
#include <checksums/checksums.h>


static const Glib::Quark GSIFTP_BULK_DOMAIN("GridFTP::Filecopy");


struct GridFTPBulkData {
    GridFTPBulkData(size_t nbfiles) :
            srcs(NULL), dsts(NULL), checksums(nbfiles),
            errn(new int[nbfiles]), fsize(new off_t[nbfiles]),
            index(0), nbfiles(nbfiles), started(new bool[nbfiles]),
            params(NULL), ipv6(false), error(NULL), done(false)
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
    bool ipv6;

    globus_mutex_t lock;
    globus_cond_t cond;
    globus_object_t* error;
    globus_bool_t done;
};

// Called by Globus when done
static void gridftp_done_callback(void * user_arg, globus_ftp_client_handle_t * handle,
        globus_object_t * err)
{
    GridFTPBulkData* data = static_cast<GridFTPBulkData*>(user_arg);

    if (err) {
        data->error = globus_object_copy(err);
    }

    for (size_t i = 0; i < data->nbfiles; ++i) {
        if (data->started[i]) {
            plugin_trigger_event(data->params, GSIFTP_BULK_DOMAIN, GFAL_EVENT_NONE,
                    GFAL_EVENT_TRANSFER_EXIT,
                    "Done %s => %s", data->srcs[i], data->dsts[i]);
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
        gfal_log(GFAL_VERBOSE_DEBUG,
                "Skipping pair %d as marked failed with %d", data->index,
                data->errn[data->index]);
        data->index++;
    }

    // Return next pair
    if (data->index < data->nbfiles) {
        *source_url = (char*)data->srcs[data->index];
        *dest_url = (char*)data->dsts[data->index];
        data->started[data->index] = true;

        plugin_trigger_event(data->params, GSIFTP_BULK_DOMAIN, GFAL_EVENT_NONE,
                GFAL_EVENT_TRANSFER_ENTER,
                "Providing next pair: (%s) %s => (%s) %s",
                return_hostname(*source_url, data->ipv6).c_str(), *source_url,
                return_hostname(*dest_url, data->ipv6).c_str(), *dest_url);
    }
    else {
        *source_url = NULL;
        *dest_url = NULL;

        gfal_log(GFAL_VERBOSE_VERBOSE, "No more pairs to give");
    }
}


static
void gridftp_bulk_cancel(gfal2_context_t context, void* userdata)
{
    globus_ftp_client_handle_t* ftp_handle = static_cast<globus_ftp_client_handle_t*>(userdata);
    globus_ftp_client_abort(ftp_handle);
}


static
int gridftp_pipeline_transfer(plugin_handle plugin_data,
        gfal2_context_t context, GridFTPBulkData* pairs, GError** op_error)
{
    GridFTPModule* gsiftp = static_cast<GridFTPModule*>(plugin_data);
    GridFTPSession sess(
            gsiftp->get_session_factory()->gfal_globus_ftp_take_handle(
                    gridftp_hostname_from_url(pairs->srcs[0])));

    globus_ftp_client_handle_t ftp_handle;
    globus_ftp_client_operationattr_t ftp_operation_attr;
    globus_ftp_client_handleattr_t* ftp_handle_attr = sess.get_ftp_handle_attr();

    // First viable pair goes with the globus call
    pairs->index = 0;
    while (pairs->index < pairs->nbfiles && pairs->errn[pairs->index])
        pairs->index++;
    if (pairs->index >= pairs->nbfiles)
        return 0;

    pairs->started[pairs->index] = true;

    globus_ftp_client_handleattr_set_pipeline(ftp_handle_attr, 0, gridftp_pipeline_callback, pairs);
    globus_ftp_client_handle_init(&ftp_handle, ftp_handle_attr);
    globus_ftp_client_operationattr_init(&ftp_operation_attr);
    globus_ftp_client_operationattr_copy(&ftp_operation_attr, sess.get_op_attr_ftp());
    globus_ftp_client_operationattr_set_mode(&ftp_operation_attr, GLOBUS_FTP_CONTROL_MODE_EXTENDED_BLOCK);
    globus_ftp_client_operationattr_set_delayed_pasv(&ftp_operation_attr, GLOBUS_FALSE);

    gfal_cancel_token_t cancel_token;
    cancel_token = gfal2_register_cancel_callback(context, gridftp_bulk_cancel, ftp_handle);

    int res = 0;
    try {
        globus_result_t globus_return;

        plugin_trigger_event(pairs->params, GSIFTP_BULK_DOMAIN, GFAL_EVENT_NONE,
                GFAL_EVENT_TRANSFER_ENTER,
                "Providing first pair: (%s) %s => (%s) %s",
                return_hostname(pairs->srcs[pairs->index], pairs->ipv6).c_str(), pairs->srcs[pairs->index],
                return_hostname(pairs->dsts[pairs->index], pairs->ipv6).c_str(), pairs->dsts[pairs->index]);

        globus_return = globus_ftp_client_third_party_transfer(&ftp_handle,
                pairs->srcs[pairs->index], &ftp_operation_attr,
                pairs->dsts[pairs->index], &ftp_operation_attr,
                GLOBUS_NULL, gridftp_done_callback, pairs);
        gfal_globus_check_result("gridftp_bulk_copy", globus_return);

        globus_mutex_lock(&pairs->lock);
        while (!pairs->done) {
            globus_cond_wait(&pairs->cond, &pairs->lock);
        }
        globus_mutex_unlock(&pairs->lock);

        if (pairs->error) {
            char *err_buffer;
            int err_code = gfal_globus_error_convert(pairs->error, &err_buffer);
            if (err_code) {
                gfal_log(GFAL_VERBOSE_VERBOSE, "Bulk transfer failed with %s", err_buffer);
                gfal2_set_error(op_error, GSIFTP_BULK_DOMAIN, err_code, __func__, "%s", err_buffer);
                res = -1;
                g_free(err_buffer);
            }
        }
    }
    catch (const Gfal::CoreException& e) {
        gfal_log(GFAL_VERBOSE_NORMAL, "Bulk transfer failed with %s", e.what().c_str());
        gfal2_set_error(op_error, e.domain(), e.code(), __func__, "%s", e.what().c_str());
        res = -1;
    }

    gfal2_remove_cancel_callback(context, cancel_token);

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
                            gfal2_set_error(&(file_errors[i]), GSIFTP_BULK_DOMAIN, EIO,
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
                    gridftp_filecopy_delete_existing((GridFTPModule*) plugin_data,
                            pairs->params, pairs->dsts[i]);
                    gridftp_create_parent_copy((GridFTPModule*) plugin_data,
                            pairs->params, pairs->dsts[i]);
                }
                catch (const Gfal::TransferException& e) {
                    gfal2_set_error(&(file_errors[i]), GSIFTP_BULK_DOMAIN, e.code(),
                            __func__, "%s", e.what().c_str());
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
                    gfal2_set_error(&(file_errors[i]), GSIFTP_BULK_DOMAIN, EIO,
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
                                gfal2_set_error(&(file_errors[i]), GSIFTP_BULK_DOMAIN, EIO, __func__,
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
    gfal_log(GFAL_VERBOSE_TRACE, "-> %s", __func__);

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
    pairs.ipv6 = gfal2_get_opt_boolean_with_default(context,
            GRIDFTP_CONFIG_GROUP, gridftp_ipv6_config, false);

    // Preparation stage
    *file_errors = g_new0(GError*, nbfiles);
    int total_failed = gridftp_bulk_prepare(plugin_data, context, &pairs, *file_errors);

    // Transfer
    int transfer_ret = -1;
    if (!gfal2_is_canceled(context))
        transfer_ret = gridftp_pipeline_transfer(plugin_data, context, &pairs, op_error);
    if (transfer_ret < 0)
        total_failed = nbfiles;

    // Check destinations
    if (transfer_ret == 0)
        total_failed += gridftp_bulk_close(plugin_data, context, &pairs, *file_errors);

    // Done
    gfal_log(GFAL_VERBOSE_TRACE, "<- %s", __func__);
    gfal2_end_scope_cancel(context);
    return -total_failed;
}
