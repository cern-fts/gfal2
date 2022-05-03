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

#include <davix.hpp>
#include <copy/davixcopy.hpp>
#include <status/davixstatusrequest.hpp>
#include <unistd.h>
#include <checksums/checksums.h>
#include <cstdio>
#include <cstring>
#include <list>
#include <sstream>
#include "gfal_http_plugin.h"

// An enumeration of the different HTTP third-party-copy strategies.
// NOTE: we loop through strategies from beginning to end, meaning the default
// strategy should always be listed first.
typedef enum {HTTP_COPY_PULL=0,
              HTTP_COPY_PUSH,
              HTTP_COPY_STREAM,
              HTTP_COPY_END} CopyMode;
// The human-readable strings corresponding to the above enum; changes to the two
// lists must be synchronized.
const char* CopyModeStr[] = {
    GFAL_TRANSFER_TYPE_PULL, GFAL_TRANSFER_TYPE_PUSH, GFAL_TRANSFER_TYPE_STREAMED, NULL
};

static const CopyMode get_copy_mode_from_string(const char * copy_mode_str) {
    if (!strcmp(copy_mode_str, GFAL_TRANSFER_TYPE_PULL))
        return HTTP_COPY_PULL;
    if (!strcmp(copy_mode_str, GFAL_TRANSFER_TYPE_PUSH))
        return HTTP_COPY_PUSH;
     if (!strcmp(copy_mode_str, GFAL_TRANSFER_TYPE_STREAMED))
        return HTTP_COPY_STREAM;
     return HTTP_COPY_PULL;
}


struct PerfCallbackData {
    gfalt_params_t     params;

    std::string        source;
    std::string        destination;
    PerfCallbackData(gfalt_params_t params, const std::string& src, const std::string& dst):
         params(params), source(src), destination(dst)
    {
    }
};

static bool set_copy_mode(gfal2_context_t context,const char * copy_mode) {
    bool set = true;
    GError *error = NULL;
    if (!strcmp(copy_mode,"push" )) {
        gfal2_set_opt_boolean(context, "HTTP PLUGIN", "ENABLE_REMOTE_COPY", TRUE, &error);
        gfal2_set_opt_string(context, "HTTP PLUGIN", "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PUSH, &error);
        gfal2_set_opt_boolean(context, "HTTP PLUGIN", "ENABLE_FALLBACK_TPC_COPY", FALSE, &error);
        if (error != NULL) {
            g_clear_error(&error);
            set =false;
        }
    } else if (!strcmp(copy_mode,"pull") ) {
        gfal2_set_opt_boolean(context, "HTTP PLUGIN", "ENABLE_REMOTE_COPY", TRUE, &error);
        gfal2_set_opt_string(context, "HTTP PLUGIN", "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PULL, &error);
        gfal2_set_opt_boolean(context, "HTTP PLUGIN", "ENABLE_FALLBACK_TPC_COPY", FALSE, &error);
        if (error != NULL) {
            g_clear_error(&error);
            set =false;
        }
    } else {
        set =false;
    }
    return set;
}

static void extract_query_parameter(const char* url, const char *key, char *value, size_t val_size)
{
    value[0] = '\0';

    const char *str = strchr(url, '?');
    if (str == NULL) {
        return;
    }

    size_t key_len = strlen(key);
    char **args = g_strsplit(str + 1, "&", 0);
    int i;
    for (i = 0; args[i] != NULL; ++i) {
        if (strncmp(args[i], key, key_len) == 0) {
            char *p = strchr(args[i], '=');
            if (p) {
                g_strlcpy(value, p + 1, val_size);
                break;
            }
        }
    }

    g_strfreev(args);
}

static void set_copy_mode_from_urls(gfal2_context_t context, const char * src_url, const char * dst_url)
{
    char copy_mode[64] = {0};
    bool done = false;

    extract_query_parameter(src_url, "copy_mode", copy_mode, sizeof(copy_mode));

    if (copy_mode[0] != '\0') {
        gfal2_log(G_LOG_LEVEL_INFO, "Source copy mode is %s", copy_mode);
        done = set_copy_mode(context, copy_mode);
    }

    if (!done) {
        extract_query_parameter(dst_url, "copy_mode", copy_mode, sizeof(copy_mode));

        if (copy_mode[0] != '\0') {
	          gfal2_log(G_LOG_LEVEL_INFO, "Destination copy mode is %s", copy_mode);
            set_copy_mode(context, copy_mode);
        }
    }
}

static bool is_http_scheme(const char* url)
{
    const char *schemes[] = {"http:", "https:", "dav:", "davs:", "s3:", "s3s:", "gcloud:", "gclouds:", "swift:", "swifts:", "cs3:", "cs3s:", NULL};
    const char *colon = strchr(url, ':');
    if (!colon)
        return false;
    size_t scheme_len = colon - url + 1;
    for (size_t i = 0; schemes[i] != NULL; ++i) {
        if (strncmp(url, schemes[i], scheme_len) == 0)
            return true;
    }
    return false;
}

static bool is_http_3rdcopy_enabled(gfal2_context_t context)
{
    return gfal2_get_opt_boolean_with_default(context, "HTTP PLUGIN", "ENABLE_REMOTE_COPY", TRUE);
}


bool is_http_streaming_enabled(gfal2_context_t context, const char* src, const char* dst)
{
    auto host_streaming = [&context](const char* surl) -> int {
        Davix::Uri uri(surl);

        if (uri.getStatus() != Davix::StatusCode::OK) {
            return -1;
        }

        std::string prot = uri.getProtocol();

        if (prot.back() == 's') {
            prot.pop_back();
        }

        GError* error = NULL;
        std::string group = prot + ":" + uri.getHost();
        std::transform(group.begin(), group.end(), group.begin(), ::toupper);
        gboolean streaming_enabled = gfal2_get_opt_boolean(context, group.c_str(), "ENABLE_STREAM_COPY", &error);

        if (error != NULL) {
            return -1;
        }

        return streaming_enabled;
    };

    int src_streaming = host_streaming(src);
    int dst_streaming = host_streaming(dst);

    if (src_streaming > -1 || dst_streaming > -1) {
        return src_streaming && dst_streaming;
    }

    return gfal2_get_opt_boolean_with_default(context, "HTTP PLUGIN", "ENABLE_STREAM_COPY", TRUE);
}

static CopyMode get_default_copy_mode(gfal2_context_t context)
{
    return get_copy_mode_from_string(gfal2_get_opt_string_with_default(context, "HTTP PLUGIN", "DEFAULT_COPY_MODE", GFAL_TRANSFER_TYPE_PULL));
}

static bool is_http_3rdcopy_fallback_enabled(gfal2_context_t context)
{
    return gfal2_get_opt_boolean_with_default(context, "HTTP PLUGIN", "ENABLE_FALLBACK_TPC_COPY", TRUE);
}


static gboolean gfal_http_copy_should_fallback(int error_code)
{
    switch (error_code) {
        case ECANCELED:
        case EPERM:
        case ENOENT:
        case EACCES:
            return false;
        default:
            return true;
    }
}


static int gfal_http_exists(plugin_handle plugin_data,
        const char* url, GError** err)
{
    GError *nestedError = NULL;
    int access_stat = gfal_http_access(plugin_data, url, F_OK, &nestedError);

    if (access_stat == 0) {
        return 1;
    }
    else if (nestedError->code == ENOENT) {
        g_error_free(nestedError);
        return 0;
    }
    else {
        gfalt_propagate_prefixed_error(err, nestedError, __func__, "", "");
        return -1;
    }
}


static int gfal_http_copy_overwrite(plugin_handle plugin_data,
        gfalt_params_t params,  const char *dst, GError** err)
{
    GError *nestedError = NULL;

    int exists = gfal_http_exists(plugin_data, dst, &nestedError);

    if (exists > 0) {
        if (!gfalt_get_replace_existing_file(params,NULL)) {
            gfalt_set_error(err, http_plugin_domain, EEXIST, __func__,
                    GFALT_ERROR_DESTINATION, GFALT_ERROR_EXISTS, "The destination file exists and overwrite is not enabled");
            return -1;
        }

        gfal_http_unlinkG(plugin_data, dst, &nestedError);
        if (nestedError) {
            gfalt_propagate_prefixed_error(err, nestedError, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_OVERWRITE);
            return -1;
        }

        gfal2_log(G_LOG_LEVEL_DEBUG,
                 "File %s deleted with success (overwrite set)", dst);
        plugin_trigger_event(params, http_plugin_domain, GFAL_EVENT_DESTINATION,
                             GFAL_EVENT_OVERWRITE_DESTINATION, "Deleted %s", dst);
    }
    else if (exists == 0) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Destination does not exist");
    }
    else if (exists < 0) {
        gfalt_propagate_prefixed_error(err, nestedError, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_OVERWRITE);
        return -1;
    }

    return 0;
}


static char* gfal_http_get_parent(const char* url)
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


static int gfal_http_copy_make_parent(plugin_handle plugin_data,
        gfalt_params_t params, gfal2_context_t context,
        const char* dst, GError** err)
{
    GError *nestedError = NULL;

    if (!gfalt_get_create_parent_dir(params, NULL))
        return 0;

    char *parent = gfal_http_get_parent(dst);
    if (!parent) {
        gfalt_set_error(err, http_plugin_domain, EINVAL, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_PARENT,
                       "Could not get the parent directory of %s", dst);
        return -1;
    }

    int exists = gfal_http_exists(plugin_data, parent, &nestedError);
    // Error
    if (exists < 0) {
        gfalt_propagate_prefixed_error(err, nestedError, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_PARENT);
        return -1;
    }
    // Does exist
    else if (exists == 1) {
        return 0;
    }
    // Does not exist
    else {
        gfal2_mkdir_rec(context, parent, 0755, &nestedError);
        if (nestedError) {
            gfalt_propagate_prefixed_error(err, nestedError, __func__, GFALT_ERROR_DESTINATION, GFALT_ERROR_PARENT);
            return -1;
        }
        gfal2_log(G_LOG_LEVEL_DEBUG,
                 "[%s] Created parent directory %s", __func__, parent);
        return 0;
    }
}

static bool gfal_http_cancellationcopy_callback(void* data)
{
    return gfal2_is_canceled(*static_cast<gfal2_context_t*>(data));
}

static void gfal_http_3rdcopy_perfcallback(const Davix::PerformanceData& perfData, void* data)
{
    PerfCallbackData* pdata = static_cast<PerfCallbackData*>(data);
    static int callCount = 0;

    callCount++;

    if (pdata)
    {
        _gfalt_transfer_status status;

        status.average_baudrate = static_cast<size_t>(perfData.avgTransfer());
        status.bytes_transfered = static_cast<size_t>(perfData.totalTransferred());
        status.instant_baudrate = static_cast<size_t>(perfData.diffTransfer());
        status.transfer_time    = perfData.absElapsed();

        if ((callCount == 1) && (perfData.ipflag == Davix::IPv6)) {
            plugin_trigger_event(pdata->params, http_plugin_domain,
                                 GFAL_EVENT_DESTINATION, GFAL_EVENT_IPV6,
                                 "TRUE");
        }
        plugin_trigger_monitor(pdata->params, &status, pdata->source.c_str(), pdata->destination.c_str());
    }
}

/// Utility function to clean destination file after failed copy operation.
/// Clean-up is performed if the copy error is something else than EEXIST
static void gfal_http_copy_cleanup(plugin_handle plugin_data, const char* dst, GError** err)
{
    GError *unlink_err = NULL;

    if ((*err)->code != EEXIST) {
        if (gfal_http_unlinkG(plugin_data, dst, &unlink_err) != 0) {
            if (unlink_err->code != ENOENT) {
                gfal2_log(G_LOG_LEVEL_WARNING,
                          "When trying to clean the destination: %s", unlink_err->message);
            }
            g_error_free(unlink_err);
        } else {
            gfal2_log(G_LOG_LEVEL_DEBUG, "Destination file removed");
        }
    } else {
        gfal2_log(G_LOG_LEVEL_DEBUG, "The transfer failed because the file exists. Do not clean!");
    }
}

// Only http or https
// See DMC-473 and DMC-1010
static std::string get_canonical_uri(const std::string& original)
{
    std::string scheme;
    char last_scheme;

    if ((original.compare(0, 2, "s3") == 0)  ||
        (original.compare(0, 6, "gcloud") == 0 ) ||
        (original.compare(0, 5, "swift") == 0) ||
        (original.compare(0, 3, "cs3") == 0)) {
        return original;
    }

    size_t plus_pos = original.find('+');
    size_t colon_pos = original.find(':');

    if (plus_pos < colon_pos)
        last_scheme = original[plus_pos - 1];
    else
        last_scheme = original[colon_pos - 1];

    if (last_scheme == 's')
        scheme = "https";
    else
        scheme = "http";

    std::string canonical = (scheme + original.substr(colon_pos));
    return canonical;
}


static int gfal_http_third_party_copy(gfal2_context_t context,
                                      GfalHttpPluginData* davix,
                                      const char* src, const char* dst,
                                      CopyMode mode,
                                      gfalt_params_t params,
                                      GError** err)
{
    gfal2_log(G_LOG_LEVEL_MESSAGE, "Performing a HTTP third party copy");

    PerfCallbackData perfCallbackData(
            params, src, dst
    );

    Davix::RequestParams req_params;
    davix->get_tpc_params(&req_params, Davix::Uri(src), Davix::Uri(dst), params, mode == HTTP_COPY_PUSH);

    if (mode == HTTP_COPY_PUSH) {
        req_params.setCopyMode(Davix::CopyMode::Push);
    } else if (mode == HTTP_COPY_PULL) {
        req_params.setCopyMode(Davix::CopyMode::Pull);
    } else {
        gfal2_set_error(err, http_plugin_domain, EIO, __func__, "gfal_http_third_party_copy invalid copy mode");
        return -1;
    }

    // dCache requires RequireChecksumVerification to be explicitly false if no checksums
    // are to be used
    gfalt_checksum_mode_t checksum_mode = gfalt_get_checksum_mode(params, err);
    if (*err) {
        g_clear_error(err);
    } else {
        if (mode == HTTP_COPY_PUSH) {
            if ((checksum_mode & GFALT_CHECKSUM_SOURCE) || (checksum_mode == GFALT_CHECKSUM_NONE)) {
                req_params.addHeader("RequireChecksumVerification", "false");
            }
        } else if (mode == HTTP_COPY_PULL) {
            if ((checksum_mode & GFALT_CHECKSUM_TARGET) || (checksum_mode == GFALT_CHECKSUM_NONE)) {
                req_params.addHeader("RequireChecksumVerification", "false");
            }
        }
    }

    // add timeout
    struct timespec opTimeout;
    opTimeout.tv_sec = gfalt_get_timeout(params, NULL);
    req_params.setOperationTimeout(&opTimeout);

    Davix::Uri src_uri(get_canonical_uri(src));
    Davix::Uri dst_uri(get_canonical_uri(dst));

    Davix::DavixCopy copy(davix->context, &req_params);

    copy.setPerformanceCallback(gfal_http_3rdcopy_perfcallback, &perfCallbackData);
    copy.setCancellationCallback(gfal_http_cancellationcopy_callback, &context);

    Davix::DavixError* davError = NULL;
    copy.copy(src_uri, dst_uri,
              gfalt_get_nbstreams(params, NULL),
              &davError);

    if (davError != NULL) {
        davix2gliberr(davError, err, __func__);
        Davix::DavixError::clearError(&davError);
    }

    return *err == NULL ? 0 : -1;
}


struct HttpStreamProvider {
    const char *source, *destination;

    gfal2_context_t context;
    gfalt_params_t params;
    int source_fd;
    time_t start, last_update;
    dav_ssize_t read_instant;
    _gfalt_transfer_status perf;

    HttpStreamProvider(const char* source, const char* destination,
            gfal2_context_t context, int source_fd, gfalt_params_t params):
        source(source), destination(destination),
        context(context), params(params), source_fd(source_fd), start(time(NULL)),
        last_update(start), read_instant(0)
    {
        memset(&perf, 0, sizeof(perf));
    }
};


static dav_ssize_t gfal_http_streamed_provider(void *userdata,
        void *buffer, dav_size_t buflen)
{
    GError* error = NULL;
    HttpStreamProvider* data = static_cast<HttpStreamProvider*>(userdata);
    dav_ssize_t ret = 0;

    time_t now = time(NULL);

    if (buflen == 0) {
        data->perf.bytes_transfered = data->read_instant = 0;
        data->perf.average_baudrate = 0;
        data->perf.instant_baudrate = 0;
        data->start = data->last_update = now;

        if (gfal2_lseek(data->context, data->source_fd, 0, SEEK_SET, &error) < 0)
            ret = -1;
    }
    else {
        ret = gfal2_read(data->context, data->source_fd, buffer, buflen, &error);
        if (ret > 0)
            data->read_instant += ret;

        if (now - data->last_update >= 5) {
            data->perf.bytes_transfered += data->read_instant;
            data->perf.transfer_time = now - data->start;
            data->perf.average_baudrate = data->perf.bytes_transfered / data->perf.transfer_time;
            data->perf.instant_baudrate = data->read_instant / (now - data->last_update);

            data->last_update = now;
            data->read_instant = 0;

            plugin_trigger_monitor(data->params, &data->perf, data->source, data->destination);
        }
    }

    if (error)
        g_error_free(error);

    return ret;
}



static int gfal_http_streamed_copy(gfal2_context_t context,
        GfalHttpPluginData* davix,
        const char* src, const char* dst,
        gfalt_checksum_mode_t checksum_mode, const char *checksum_type, const char *user_checksum,
        gfalt_params_t params,
        GError** err)
{
    gfal2_log(G_LOG_LEVEL_MESSAGE, "Performing a HTTP streamed copy");
    GError *nested_err = NULL;

    struct stat src_stat;
    if (gfal2_stat(context, src, &src_stat, &nested_err) != 0) {
        gfal2_propagate_prefixed_error(err, nested_err, __func__);
        return -1;
    }

    int source_fd = gfal2_open(context, src, O_RDONLY, &nested_err);
    if (source_fd < 0) {
        gfal2_propagate_prefixed_error(err, nested_err, __func__);
        return -1;
    }

    Davix::Uri dst_uri(dst);
    Davix::RequestParams req_params;

    davix->get_params(&req_params, dst_uri, GfalHttpPluginData::OP::WRITE);
    //add timeout
    struct timespec opTimeout;
    opTimeout.tv_sec = gfalt_get_timeout(params, NULL);
    req_params.setOperationTimeout(&opTimeout);

    // Set MD5 header on the PUT
    if (checksum_mode & GFALT_CHECKSUM_TARGET && strcasecmp(checksum_type, "md5") == 0 && user_checksum[0]) {
    	req_params.addHeader("Content-MD5", user_checksum);
    }

    if (dst_uri.getProtocol() == "s3" || dst_uri.getProtocol() == "s3s")
    	req_params.setProtocol(Davix::RequestProtocol::AwsS3);
    else if (dst_uri.getProtocol() == "gcloud" ||  dst_uri.getProtocol() ==  "gclouds")
    	req_params.setProtocol(Davix::RequestProtocol::Gcloud);
    else if (dst_uri.getProtocol() == "swift" || dst_uri.getProtocol() == "swifts")
        req_params.setProtocol(Davix::RequestProtocol::Swift);
    else if (dst_uri.getProtocol() == "cs3" || dst_uri.getProtocol() == "cs3s")
        req_params.setProtocol(Davix::RequestProtocol::CS3);

    Davix::DavFile dest(davix->context,req_params, dst_uri );

    HttpStreamProvider provider(src, dst, context, source_fd, params);

    try {
    	dest.put(&req_params, std::bind(&gfal_http_streamed_provider,&provider,
        		  std::placeholders::_1, std::placeholders::_2), src_stat.st_size);

    } catch(Davix::DavixException &ex) {
    	Davix::DavixError* daverr = NULL;
        ex.toDavixError(&daverr);
        davix2gliberr(daverr, err, __func__);
        Davix::DavixError::clearError(&daverr);
    }

    gfal2_close(context, source_fd, &nested_err);
    // Throw away this error
    if (nested_err)
        g_error_free(nested_err);

    return *err == NULL ? 0 : -1;
}


void strip_3rd_from_url(const char* url_full, char* url, size_t url_size)
{
    const char* colon = strchr(url_full, ':');
    const char* plus = strchr(url_full, '+');
    if (!plus || !colon || plus > colon) {
        g_strlcpy(url, url_full, url_size);
    }
    else {
        size_t len = plus - url_full + 1;
        if (len >= url_size)
            len = url_size;
        g_strlcpy(url, url_full, len);
        g_strlcat(url, colon, url_size);
        gfal2_log(G_LOG_LEVEL_WARNING, "+3rd schemes deprecated");
    }
}


int gfal_http_copy(plugin_handle plugin_data, gfal2_context_t context,
        gfalt_params_t params, const char* src_full, const char* dst_full, GError** err)
{
    GError* nested_error = NULL;
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);

    plugin_trigger_event(params, http_plugin_domain,
                         GFAL_EVENT_NONE, GFAL_EVENT_PREPARE_ENTER,
                         "%s => %s", src_full, dst_full);

    // Determine if urls are 3rd party, and strip the +3rd if they are
    char src[GFAL_URL_MAX_LEN], dst[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(src_full, src, sizeof(src));
    strip_3rd_from_url(dst_full, dst, sizeof(dst));

    gfal2_log(G_LOG_LEVEL_DEBUG, "Using source: %s", src);
    gfal2_log(G_LOG_LEVEL_DEBUG, "Using destination: %s", dst);

    // Get user defined checksum
    gfalt_checksum_mode_t checksum_mode = GFALT_CHECKSUM_NONE;
    char checksum_type[1024] = {0};
    char user_checksum[1024] = {0};
    char src_checksum[1024] = {0};

    if (!gfalt_get_strict_copy_mode(params, NULL)) {
        checksum_mode = gfalt_get_checksum(params,
            checksum_type, sizeof(checksum_type),
            user_checksum, sizeof(user_checksum), NULL);
    }

    // Source checksum
    if (checksum_mode & GFALT_CHECKSUM_SOURCE) {
        plugin_trigger_event(params, http_plugin_domain, GFAL_EVENT_SOURCE,
                GFAL_EVENT_CHECKSUM_ENTER, "");

        gfal2_checksum(context, src, checksum_type,
                       0, 0, src_checksum, sizeof(src_checksum),
                       &nested_error);

        if (nested_error) {
            if (nested_error->code == ENOSYS || nested_error->code == ENOTSUP) {
                gfal2_log(G_LOG_LEVEL_WARNING,
                        "Checksum type %s not supported by source. Skip source check.",
                        checksum_type);
                g_clear_error (&nested_error);
                src_checksum[0] = '\0';
            } else {
                gfalt_propagate_prefixed_error(err, nested_error, __func__,
                        GFALT_ERROR_SOURCE, GFALT_ERROR_CHECKSUM);
                return -1;
            }
        }
        else if (user_checksum[0]) {
            if (gfal_compare_checksums(src_checksum, user_checksum, sizeof(src_checksum)) != 0) {
                gfalt_set_error(err, http_plugin_domain, EIO, __func__,
                        GFALT_ERROR_SOURCE, GFALT_ERROR_CHECKSUM_MISMATCH,
                        "Source and user-defined %s do not match (%s != %s)",
                        checksum_type, src_checksum, user_checksum);
                return -1;
            }
        }

        plugin_trigger_event(params, http_plugin_domain, GFAL_EVENT_SOURCE,
                GFAL_EVENT_CHECKSUM_EXIT, "");
    }

    // When this flag is not set, the plugin should handle overwriting,
    // parent directory creation,...
    if (!gfalt_get_strict_copy_mode(params, NULL)) {
        if (gfal_http_copy_overwrite(plugin_data, params, dst, &nested_error) != 0 ||
            gfal_http_copy_make_parent(plugin_data, params, context, dst, &nested_error) != 0) {
            gfal2_propagate_prefixed_error(err, nested_error, __func__);
            return -1;
        }
    }

    plugin_trigger_event(params, http_plugin_domain,
                         GFAL_EVENT_NONE, GFAL_EVENT_PREPARE_EXIT,
                         "%s => %s", src_full, dst_full);

    set_copy_mode_from_urls(context,src_full, dst_full);
    // Initial copy mode
    CopyMode copy_mode = get_default_copy_mode(context);

    bool only_streaming = false;
    // If source is not even http, go straight to streamed
    // or if third party copy is disabled, go straight to streamed
    if (!is_http_scheme(src) || !is_http_3rdcopy_enabled(context)) {
        copy_mode = HTTP_COPY_STREAM;
        only_streaming = true;
    }

    int ret = 0;
    std::list<std::string> attempted_mode;
    CopyMode end_copy_mode = HTTP_COPY_END;

    // Stop the loop prematurely if streaming is disabled
    bool streaming_enabled = is_http_streaming_enabled(context, src, dst);
    if (!streaming_enabled) {
        end_copy_mode = HTTP_COPY_STREAM;
    }

    plugin_trigger_event(params, http_plugin_domain,
                         GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_ENTER,
                         "%s => %s", src_full, dst_full);

    do {
        // Perform the copy, going through the different fallback copy modes
        gfal2_log(G_LOG_LEVEL_MESSAGE, "Trying copying with mode %s",
                  CopyModeStr[copy_mode]);
        plugin_trigger_event(params, http_plugin_domain,
                             GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_TYPE,
                             "%s", CopyModeStr[copy_mode]);
        g_clear_error(&nested_error);

        if (copy_mode == HTTP_COPY_STREAM) {
            if (streaming_enabled) {
                ret = gfal_http_streamed_copy(context, davix, src, dst,
                                              checksum_mode, checksum_type, user_checksum,
                                              params, &nested_error);
            } else if (only_streaming) {
                gfal2_set_error(&nested_error, http_plugin_domain, EINVAL, __func__,
                                "STREAMED DISABLED Only streamed copy possible but streaming is disabled");
                gfal2_log(G_LOG_LEVEL_WARNING, "%s", nested_error->message);
                ret = -1;
                break;
            }
        } else {
            ret = gfal_http_third_party_copy(context, davix, src, dst, copy_mode, params, &nested_error);
        }

        if (ret == 0) {
            gfal2_log(G_LOG_LEVEL_MESSAGE, "Copy succeeded using mode %s", CopyModeStr[copy_mode]);
            break;
        } else {
            gfal2_log(G_LOG_LEVEL_WARNING, "Copy failed with mode %s: %s", CopyModeStr[copy_mode], nested_error->message);
            // Delete any potential destination file
            gfal_http_copy_cleanup(plugin_data, dst, &nested_error);
        }

        attempted_mode.emplace_back(CopyModeStr[copy_mode]);
        copy_mode = (CopyMode)((int)copy_mode + 1);
    } while ((copy_mode < end_copy_mode) &&
             is_http_3rdcopy_fallback_enabled(context) &&
             gfal_http_copy_should_fallback(nested_error->code));

    if (ret == 0) {
        plugin_trigger_event(params, http_plugin_domain,
                             GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_EXIT,
                             "%s => %s", src, dst);
    } else {
        if (!attempted_mode.empty()) {
            std::ostringstream errmsg;
            errmsg << "Copy failed (";
            for (auto mode = attempted_mode.begin(); mode != attempted_mode.end(); mode++) {
                if (mode != attempted_mode.begin()) {
                    errmsg << ", ";
                }
                errmsg << (*mode);
            }
            errmsg << "). Last attempt: ";
            g_prefix_error(&nested_error, "ERROR: %s", errmsg.str().c_str());
        }

        plugin_trigger_event(params, http_plugin_domain,
                             GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_EXIT,
                             "%s", nested_error->message);
        gfalt_propagate_prefixed_error(err, nested_error, __func__, GFALT_ERROR_TRANSFER, NULL);
        return -1;
    }

    // Destination checksum validation
    if (checksum_mode & GFALT_CHECKSUM_TARGET) {
        char dst_checksum[1024];
        plugin_trigger_event(params, http_plugin_domain, GFAL_EVENT_DESTINATION,
                GFAL_EVENT_CHECKSUM_ENTER, "");

        gfal2_checksum(context, dst, checksum_type,
                       0, 0, dst_checksum, sizeof(dst_checksum),
                       &nested_error);

        if (nested_error) {
            gfalt_propagate_prefixed_error(err, nested_error, __func__,
                    GFALT_ERROR_DESTINATION, GFALT_ERROR_CHECKSUM);
            return -1;
        }

        if (src_checksum[0]) {
            if (gfal_compare_checksums(src_checksum, dst_checksum, sizeof(src_checksum)) != 0) {
                gfalt_set_error(err, http_plugin_domain, EIO, __func__,
                        GFALT_ERROR_DESTINATION, GFALT_ERROR_CHECKSUM_MISMATCH,
                        "Source and destination %s do not match (%s != %s)",
                        checksum_type, src_checksum, dst_checksum);
                return -1;
            }
        }
        else if (user_checksum[0]) {
            if (gfal_compare_checksums(user_checksum, dst_checksum, sizeof(user_checksum)) != 0) {
                gfalt_set_error(err, http_plugin_domain, EIO, __func__,
                        GFALT_ERROR_DESTINATION, GFALT_ERROR_CHECKSUM_MISMATCH,
                        "User-defined and destination %s do not match (%s != %s)",
                        checksum_type, user_checksum, dst_checksum);
                return -1;
            }
        }

        plugin_trigger_event(params, http_plugin_domain, GFAL_EVENT_DESTINATION,
                GFAL_EVENT_CHECKSUM_EXIT, "");
    }

    // Evict source file if configured
    if (gfalt_get_use_evict(params, NULL)) {
        GError* tmp_err;
        ret = gfal_http_release_file(plugin_data, src, "", &tmp_err);
        gfal2_log(G_LOG_LEVEL_INFO, "Eviction request exited with status code: %d", ret);

        if (ret < 0) {
            gfal2_log(G_LOG_LEVEL_INFO, "Eviction request failed: %s", tmp_err->message);
            g_error_free(tmp_err);
        }

        plugin_trigger_event(params, http_plugin_domain, GFAL_EVENT_SOURCE,
                             GFAL_EVENT_EVICT, "%d", ret);
    }

    return 0;
}


int gfal_http_copy_check(plugin_handle plugin_data, gfal2_context_t context, const char* src,
        const char* dst, gfal_url2_check check)
{
    if (check != GFAL_FILE_COPY)
        return 0;
    // This plugin handles everything that writes into an http endpoint
    // It will try to decide if it is better to do a third party copy, or a streamed copy later on
    return (is_http_scheme(dst) && ((strncmp(src, "file://", 7) == 0) || is_http_scheme(src)));
}
