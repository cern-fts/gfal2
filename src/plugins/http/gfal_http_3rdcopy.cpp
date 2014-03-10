#include <davix.hpp>
#include <copy/davixcopy.hpp>
#include <unistd.h>
#include <transfer/gfal_transfer_plugins.h>
#include <checksums/checksums.h>
#include <cstdio>
#include <common/gfal_common_errverbose.h>
#include "gfal_http_plugin.h"


struct PerfCallbackData {
    std::string        source;
    std::string        destination;
    gfalt_monitor_func externalCallback;
    void*              externalData;

    PerfCallbackData(const std::string& src, const std::string& dst,
                     gfalt_monitor_func callback, void* udata):
         source(src), destination(dst),
         externalCallback(callback), externalData(udata)
    {
    }
};


int gfal_http_exists(plugin_handle plugin_data,
                     const char *dst,
                     GError** err)
{
    GError *nestedError = NULL;
    struct stat st;
    gfal_http_stat(plugin_data, dst, &st, &nestedError);

    if (nestedError && nestedError->code == ENOENT) {
        g_error_free(nestedError);
        return 0;
    }
    else if (nestedError) {
        g_propagate_prefixed_error(err, nestedError, "[%s]", __func__);
        return -1;
    }
    else {
        return 1;
    }
}



int gfal_http_3rdcopy_overwrite(plugin_handle plugin_data,
                                gfalt_params_t params,
                                const char *dst,
                                GError** err)
{
    GError *nestedError = NULL;

    if (!gfalt_get_replace_existing_file(params,NULL))
        return 0;

    int exists = gfal_http_exists(plugin_data, dst, &nestedError);

    if (exists < 0) {
        g_propagate_prefixed_error(err, nestedError, "[%s]", __func__);
        return -1;
    }
    else if (exists == 1) {
        gfal_http_unlinkG(plugin_data, dst, &nestedError);
        if (nestedError) {
            g_propagate_prefixed_error(err, nestedError, "[%s]", __func__);
            return -1;
        }

        gfal_log(GFAL_VERBOSE_TRACE,
                 "File %s deleted with success (overwrite set)", dst);
    }
    return 0;
}



char* gfal_http_get_parent(const char* url)
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



int gfal_http_3rdcopy_make_parent(plugin_handle plugin_data,
                                  gfalt_params_t params,
                                  const char* dst,
                                  GError** err)
{
    GError *nestedError = NULL;

    if (!gfalt_get_create_parent_dir(params, NULL))
        return 0;

    char *parent = gfal_http_get_parent(dst);
    if (!parent) {
        *err = g_error_new(http_plugin_domain, EINVAL,
                           "[%s] Could not get the parent directory of %s",
                           __func__, dst);
        return -1;
    }

    int exists = gfal_http_exists(plugin_data, parent, &nestedError);
    // Error
    if (exists < 0) {
        g_propagate_prefixed_error(err, nestedError, "[%s]", __func__);
        return -1;
    }
    // Does exist
    else if (exists == 1) {
        return 0;
    }
    // Does not exist
    else {
        gfal_http_mkdirpG(plugin_data, parent, 0755, TRUE, &nestedError);
        if (nestedError) {
            g_propagate_prefixed_error(err, nestedError, "[%s]", __func__);
            return -1;
        }
        gfal_log(GFAL_VERBOSE_TRACE,
                 "[%s] Created parent directory %s", __func__, parent);
        return 0;
    }
}


// dst may be NULL. In that case, the user-defined checksum
// is compared with the source checksum.
// If dst != NULL, then user-defined is ignored
int gfal_http_3rdcopy_checksum(plugin_handle plugin_data,
                               gfalt_params_t params,
                               const char *src, const char *dst,
                               GError** err)
{
    if (!gfalt_get_checksum_check(params, NULL))
        return 0;

    char checksum_type[1024];
    char checksum_value[1024];
    gfalt_get_user_defined_checksum(params,
                                    checksum_type, sizeof(checksum_type),
                                    checksum_value, sizeof(checksum_value),
                                    NULL);
    if (!checksum_type[0])
        strcpy(checksum_type, "MD5");

    GError *nestedError = NULL;
    char src_checksum[1024];
    gfal_http_checksum(plugin_data, src, checksum_type,
                       src_checksum, sizeof(src_checksum),
                       0, 0, &nestedError);
    if (nestedError) {
        g_propagate_prefixed_error(err, nestedError, "[%s]", __func__);
        return -1;
    }

    if (!dst) {
        if (checksum_value[0] && gfal_compare_checksums(src_checksum, checksum_value, sizeof(checksum_value)) != 0) {
            *err = g_error_new(http_plugin_domain, EINVAL,
                               "[%s] Source and user-defined %s do not match (%s != %s)",
                               __func__, checksum_type, src_checksum, checksum_value);
            return -1;
        }
        else if (checksum_value[0]) {
            gfal_log(GFAL_VERBOSE_TRACE,
                     "[%s] Source and user-defined %s match: %s",
                     __func__, checksum_type, checksum_value);
        }
    }
    else {
        char dst_checksum[1024];
        gfal_http_checksum(plugin_data, dst, checksum_type,
                           dst_checksum, sizeof(dst_checksum),
                           0, 0, &nestedError);
        if (nestedError) {
            g_propagate_prefixed_error(err, nestedError, "[%s]", __func__);
            return -1;
        }

        if (gfal_compare_checksums(src_checksum, dst_checksum, sizeof(dst_checksum)) != 0) {
            *err = g_error_new(http_plugin_domain, EINVAL,
                                       "[%s] Source and destination %s do not match (%s != %s)",
                                       __func__, checksum_type, src_checksum, dst_checksum);
            return -1;
        }

        gfal_log(GFAL_VERBOSE_TRACE,
                 "[%s] Source and destination %s match: %s",
                 __func__, checksum_type, src_checksum);
    }
    return 0;
}



static void gfal_http_3rdcopy_perfcallback(const Davix::PerformanceData& perfData, void* data)
{
    PerfCallbackData* pdata = static_cast<PerfCallbackData*>(data);
    if (pdata && pdata->externalCallback)
    {
        gfalt_hook_transfer_plugin_t hook;

        hook.average_baudrate = static_cast<size_t>(perfData.avgTransfer());
        hook.bytes_transfered = static_cast<size_t>(perfData.totalTransferred());
        hook.instant_baudrate = static_cast<size_t>(perfData.diffTransfer());
        hook.transfer_time    = perfData.absElapsed();

        gfalt_transfer_status_t state = gfalt_transfer_status_create(&hook);
        pdata->externalCallback(state,
                pdata->source.c_str(), pdata->destination.c_str(),
                pdata->externalData);
        gfalt_transfer_status_delete(state);
    }
}



int gfal_http_3rdcopy(plugin_handle plugin_data, gfal2_context_t context,
        gfalt_params_t params, const char* src, const char* dst, GError** err)
{
    GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);

    plugin_trigger_event(params, http_plugin_domain,
                         GFAL_EVENT_NONE, GFAL_EVENT_PREPARE_ENTER,
                         "%s => %s", src, dst);

    // When this flag is set, the plugin should handle overwriting,
    // parent directory creation,...
    if (!gfalt_get_strict_copy_mode(params, NULL)) {
        plugin_trigger_event(params, http_plugin_domain,
                             GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_ENTER,
                             "");
        if (gfal_http_3rdcopy_checksum(plugin_data, params, src, NULL, err) != 0)
            return -1;
        plugin_trigger_event(params, http_plugin_domain,
                             GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_EXIT,
                             "");

        if (gfal_http_3rdcopy_overwrite(plugin_data, params, dst, err) != 0 ||
            gfal_http_3rdcopy_make_parent(plugin_data, params, dst, err) != 0)
            return -1;
    }

    plugin_trigger_event(params, http_plugin_domain,
                         GFAL_EVENT_NONE, GFAL_EVENT_PREPARE_EXIT,
                         "%s => %s", src, dst);

    // Let Davix do the copy
    Davix::DavixError* davError = NULL;
    Davix::DavixCopy copy(davix->context, &davix->params);

    plugin_trigger_event(params, http_plugin_domain,
                         GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_ENTER,
                         "%s => %s", src, dst);

    PerfCallbackData perfCallbackData(src, dst,
            gfalt_get_monitor_callback(params, NULL), gfalt_get_user_data(params, NULL));

    copy.setPerformanceCallback(gfal_http_3rdcopy_perfcallback, &perfCallbackData);
    copy.copy(Davix::Uri(src), Davix::Uri(dst),
              gfalt_get_nbstreams(params, NULL),
              &davError);

    plugin_trigger_event(params, http_plugin_domain,
                         GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_EXIT,
                         "%s => %s", src, dst);

    if (davError != NULL) {
        davix2gliberr(davError, err);
        Davix::DavixError::clearError(&davError);
        return -1;
    }

    // Checksum check
    if (!gfalt_get_strict_copy_mode(params, NULL)) {
        plugin_trigger_event(params, http_plugin_domain,
                             GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_ENTER,
                             "");
        if (gfal_http_3rdcopy_checksum(plugin_data, params, src, dst, err) != 0)
            return -1;
        plugin_trigger_event(params, http_plugin_domain,
                             GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_ENTER,
                             "");
    }

    return 0;
}


static bool is_supported_scheme(const char* url)
{
    const char *schemes[] = {"http", "https", "dav", "davs", NULL};
    const char *colon = strchr(url, ':');
    if (!colon)
        return false;
    size_t scheme_len = colon - url;
    for (size_t i = 0; schemes[i] != NULL; ++i) {
        if (strncmp(url, schemes[i], scheme_len) == 0)
            return true;
    }
    return false;
}

int gfal_http_3rdcopy_check(plugin_handle plugin_data, const char* src,
        const char* dst, gfal_url2_check check)
{
    if (check != GFAL_FILE_COPY)
        return 0;
    return is_supported_scheme(src) && is_supported_scheme(dst);
}
