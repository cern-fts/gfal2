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

#include <ctype.h>
#include <gfal_plugins_api.h>
#include "gfal_xrootd_plugin_interface.h"
#include "gfal_xrootd_plugin_utils.h"
#include "uri/gfal2_parsing.h"

#undef TRUE
#undef FALSE

#include <memory>
#include <XrdCl/XrdClCopyProcess.hh>
#include <XrdCl/XrdClFileSystem.hh>
#include <XrdVersion.hh>


class CopyFeedback: public XrdCl::CopyProgressHandler
{
public:
    CopyFeedback(gfal2_context_t context, gfalt_params_t p, bool isThirdParty) :
            context(context), params(p), startTime(0), isThirdParty(isThirdParty)
    {
        memset(&status, 0x00, sizeof(status));
    }

    virtual ~CopyFeedback()
    {
    }

    void BeginJob(uint16_t jobNum, uint16_t jobTotal, const XrdCl::URL *source,
            const XrdCl::URL *destination)
    {
        this->startTime = time(NULL);
        this->source = source->GetURL();
        this->destination = destination->GetURL();

        plugin_trigger_event(this->params, xrootd_domain, GFAL_EVENT_NONE,
                GFAL_EVENT_TRANSFER_ENTER, "%s => %s", this->source.c_str(),
                this->destination.c_str());

        if (this->isThirdParty) {
            plugin_trigger_event(params, xrootd_domain,
                GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_TYPE, GFAL_TRANSFER_TYPE_PULL);
        }
        else {
            plugin_trigger_event(params, xrootd_domain,
                GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_TYPE, GFAL_TRANSFER_TYPE_STREAMED);
        }
    }


    void EndJob(uint16_t jobNum, const XrdCl::PropertyList* result)
    {
        std::ostringstream msg;
        msg << "Job finished";

        if (result->HasProperty("status")) {
            XrdCl::XRootDStatus status;
            result->Get("status", status);
            msg << ", " << status.ToStr();
        }
        if (result->HasProperty("realTarget")) {
            std::string value;
            result->Get("realTarget", value);
            msg << ", Real target: " << value;
        }

        plugin_trigger_event(this->params, xrootd_domain, GFAL_EVENT_NONE,
                GFAL_EVENT_TRANSFER_EXIT, "%s", msg.str().c_str());
    }


    void JobProgress(uint16_t jobNum, uint64_t bytesProcessed,
            uint64_t bytesTotal)
    {
        time_t now = time(NULL);
        time_t elapsed = now - this->startTime;

        this->status.status = 0;
        this->status.bytes_transfered = bytesProcessed;
        this->status.transfer_time = elapsed;
        if (elapsed > 0)
            this->status.average_baudrate = bytesProcessed / elapsed;
        this->status.instant_baudrate = this->status.average_baudrate;

        plugin_trigger_monitor(this->params, &this->status, this->source.c_str(), this->destination.c_str());
    }

    bool ShouldCancel(uint16_t jobNum)
    {
        return gfal2_is_canceled(this->context);
    }

private:
    gfal2_context_t context;
    gfalt_params_t params;

    _gfalt_transfer_status status;
    time_t startTime;

    std::string source, destination;
    bool isThirdParty;
};


static void xrootd2gliberr(GError** err, const char* func, const char* format,
                           const XrdCl::XRootDStatus& status)
{
    std::string statusStr = status.ToStr();
    const char *str = statusStr.c_str();
    size_t str_len = statusStr.length();
    gchar *escaped_str = gfal2_utf8escape_string(str, str_len, "\n\r\t\\");

    gfal2_set_error(err, xrootd_domain, xrootd_status_to_posix_errno(status),
                    func, format, escaped_str);
    g_free(escaped_str);
}

int gfal_xrootd_3rdcopy_check(plugin_handle plugin_data,
        gfal2_context_t context, const char* src, const char* dst,
        gfal_url2_check check)
{
    if (check != GFAL_FILE_COPY && check != GFAL_BULK_COPY)
        return 0;

    bool src_is_root = strncmp(src, "root://", 7) == 0 ||
                       strncmp(src, "xroot://", 8) == 0;
    bool dst_is_root = strncmp(dst, "root://", 7) == 0 ||
                       strncmp(dst, "xroot://", 8) == 0;
    bool src_is_file = strncmp(src, "file://", 7) == 0;
    bool dst_is_file = strncmp(dst, "file://", 7) == 0;

    if (src_is_root) {
        return dst_is_root || dst_is_file;
    }
    else if (dst_is_root) {
        return src_is_root || src_is_file;
    }
    return false;
}


static void gfal_xrootd_3rd_init_url(gfal2_context_t context, XrdCl::URL& xurl,
        const char* url, const char* token)
{
    xurl.FromString(prepare_url(context, url));
    if (token) {
        XrdCl::URL::ParamsMap params;
        params.insert(std::make_pair("svcClass", token));
        xurl.SetParams(params);
    }
}

/// Clean dst, update err if failed during cleanup with something else than ENOENT,
/// returns always -1 for convenience
static int gfal_xrootd_copy_cleanup(plugin_handle plugin_data, const char* dst, GError** err)
{
    GError *unlink_err = NULL;
    if ((*err)->code != EEXIST) {
        if (gfal_xrootd_unlinkG(plugin_data, dst, &unlink_err) != 0) {
            if (unlink_err->code != ENOENT) {
                gfal2_log(G_LOG_LEVEL_WARNING,
                         "When trying to clean the destination: %s", unlink_err->message);
            }
            g_error_free(unlink_err);
        }
        else {
            gfal2_log(G_LOG_LEVEL_INFO, "Destination file removed");
        }
    }
    else {
        gfal2_log(G_LOG_LEVEL_DEBUG, "The transfer failed because the file exists. Do not clean!");
    }
    return -1;
}

int gfal_xrootd_3rd_copy_bulk(plugin_handle plugin_data,
        gfal2_context_t context, gfalt_params_t params, size_t nbfiles,
        const char* const * srcs, const char* const * dsts,
        const char* const * checksums, GError** op_error,
        GError*** file_errors)
{
    GError* internalError = NULL;
    char _checksumType[64] = { 0 };
    char _checksumValue[512] = { 0 };
    bool isThirdParty = false;

    gfalt_checksum_mode_t checksumMode = gfalt_get_checksum(params,
        _checksumType, sizeof(_checksumType),
        _checksumValue, sizeof(_checksumValue), NULL);

    XrdCl::CopyProcess copy_process;
    std::vector<XrdCl::PropertyList> results;
    for (size_t i = 0; i < nbfiles; ++i) {
        results.push_back(XrdCl::PropertyList());
    }

    const char* src_spacetoken =  gfalt_get_src_spacetoken(params, NULL);
    const char* dst_spacetoken =  gfalt_get_dst_spacetoken(params, NULL);

    for (size_t i = 0; i < nbfiles; ++i) {
        XrdCl::URL source_url, dest_url;
        gfal_xrootd_3rd_init_url(context, source_url, srcs[i], src_spacetoken);
        gfal_xrootd_3rd_init_url(context, dest_url, dsts[i], dst_spacetoken);

        XrdCl::PropertyList job;
        job.Set("source", source_url.GetURL());
        job.Set("target", dest_url.GetURL());
        job.Set("force", gfalt_get_replace_existing_file(params, NULL));
        job.Set("makeDir", gfalt_get_create_parent_dir(params, NULL));
        job.Set("SubStreamsPerChannel", gfalt_get_nbstreams(params, NULL));
        job.Set("posc", true);
        if ((source_url.GetProtocol() == "root") && (dest_url.GetProtocol() == "root")) {
            job.Set("thirdParty", "only");
            isThirdParty = true;
            job.Set("delegate", gfalt_get_use_proxy_delegation(params, NULL));
        }
        else {
            job.Set("thirdParty", "first");
        }
        job.Set("tpcTimeout", gfalt_get_timeout(params, NULL));
        gfal2_log(G_LOG_LEVEL_DEBUG, "Copy job: tpc_only=%d tpc_delegation=%d tpc_timeout=%d", isThirdParty,
                  gfalt_get_use_proxy_delegation(params, NULL),
                  gfalt_get_timeout(params, NULL));

        if (checksumMode) {
            char checksumType[64] = { 0 };
            char checksumValue[512] = { 0 };
            char **chks = g_strsplit(checksums[i], ":", 2);
            char *s = chks[1];
            while (*s && *s == '0') s++;
            strncpy(checksumType, chks[0], sizeof(chks[0]));
            strncpy(checksumValue, s, sizeof(s));
            checksumType[63] = checksumValue[511] = '\0';
            g_strfreev(chks);

            if (!checksumType[0]) {
                char* defaultChecksumType = gfal2_get_opt_string(context, XROOTD_CONFIG_GROUP, XROOTD_DEFAULT_CHECKSUM, &internalError);
                if (internalError) {
                    gfal2_set_error(op_error, xrootd_domain, internalError->code, __func__,
                            "%s", internalError->message);
                    g_error_free(internalError);
                    return -1;
                }

                g_strlcpy(checksumType, defaultChecksumType, sizeof(checksumType));
                g_free(defaultChecksumType);
            }

            std::string sChecksumType = predefined_checksum_type_to_lower(checksumType);
            std::string sChecksumValue(checksumValue);
            std::transform(sChecksumValue.begin(), sChecksumValue.end(), sChecksumValue.begin(), ::tolower);
            std::string sChecksumMode = "none";

            switch (checksumMode) {
                case GFALT_CHECKSUM_BOTH:
                    sChecksumMode = "end2end";
                    break;
                case GFALT_CHECKSUM_TARGET:
                    sChecksumMode = "target";
                    break;
                case GFALT_CHECKSUM_SOURCE:
                    sChecksumMode = "source";
                    break;
            }

            gfal2_log(G_LOG_LEVEL_DEBUG, "Predefined Checksum Mode: %s", sChecksumMode.c_str());
            gfal2_log(G_LOG_LEVEL_DEBUG, "Predefined Checksum Type: %s", sChecksumType.c_str());
            gfal2_log(G_LOG_LEVEL_DEBUG, "Predefined Checksum Value: %s", sChecksumValue.c_str());

            job.Set("checkSumMode", sChecksumMode);
            job.Set("checkSumType", sChecksumType);
            job.Set("checkSumPreset", sChecksumValue);
        }

        copy_process.AddJob(job, &(results[i]));
    }

    // Configuration job
    int parallel = gfal2_get_opt_integer_with_default(context,
            XROOTD_CONFIG_GROUP, XROOTD_PARALLEL_COPIES,
            20);

    XrdCl::PropertyList config_job;
    config_job.Set("jobType", "configuration");
    config_job.Set("parallel", parallel);
    copy_process.AddJob(config_job, NULL);


    XrdCl::XRootDStatus status = copy_process.Prepare();
    if (!status.IsOK()) {
        xrootd2gliberr(op_error, __func__, "Error on XrdCl::CopyProcess::Prepare(): %s", status);
        return -1;
    }

    CopyFeedback feedback(context, params, isThirdParty);
    status = copy_process.Run(&feedback);

    // On bulk operations, even if there is one single failure we will get it
    // here, so ignore!
    if (nbfiles == 1 && !status.IsOK()) {
        xrootd2gliberr(op_error, __func__, "Error on XrdCl::CopyProcess::Run(): %s", status);
        return gfal_xrootd_copy_cleanup(plugin_data, dsts[0],op_error);
    }

    // For bulk operations, here we do get the actual status per file
    int n_failed = 0;
    int n_evict = 0;
    *file_errors = g_new0(GError*, nbfiles);
    const char* files_to_evict[nbfiles];
    for (size_t i = 0; i < nbfiles; ++i) {
        status = results[i].Get<XrdCl::XRootDStatus>("status");
        if (!status.IsOK()) {
            xrootd2gliberr(&((*file_errors)[i]), __func__, "Error on XrdCl::CopyProcess::Run(): %s", status);
            gfal_xrootd_copy_cleanup(plugin_data, dsts[i],file_errors[i]);
            ++n_failed;
        }
        else {
            files_to_evict[n_evict] = srcs[i];
            ++n_evict;
        }
    }

    // Evict source files if flag is set to true
    if (gfalt_get_use_evict(params, NULL)) {
        std::vector<GError*> errors(n_evict, NULL);
        int ret = gfal_xrootd_release_file_list(plugin_data, n_evict, files_to_evict, "", errors.data()); //No token is needed for evict operation in xrootd
        gfal2_log(G_LOG_LEVEL_DEBUG, "Eviction request exited with status code: %d", ret);
        if (ret < 0) {
            gfal2_log(G_LOG_LEVEL_INFO, "Eviction request failed in one or more files");
            for(int i = 0; i < n_evict; ++i) {
                g_error_free(errors[i]);
            }
        }

        plugin_trigger_event(params, xrootd_domain, GFAL_EVENT_SOURCE,
                             GFAL_EVENT_EVICT, "%d", ret);
    }

    return -n_failed;
}


int gfal_xrootd_3rd_copy(plugin_handle plugin_data, gfal2_context_t context,
        gfalt_params_t params, const char* src, const char* dst, GError** err)
{
    GError* op_error = NULL;
    GError** file_error = NULL;

    char checksumType[64] = { 0 };
    char checksumValue[512] = { 0 };
    gfalt_get_checksum(params, checksumType, sizeof(checksumType),
            checksumValue, sizeof(checksumValue),
            NULL);

    char *checksumConcat[1];
    checksumConcat[0] = g_strdup_printf("%s:%s", checksumType, checksumValue);

    int ret = gfal_xrootd_3rd_copy_bulk(plugin_data,
            context, params, 1,
            &src, &dst, checksumConcat,
            &op_error, &file_error);

    g_free(checksumConcat[0]);

    if (ret < 0) {
        if (op_error) {
            gfal2_propagate_prefixed_error(err, op_error, __func__);
        }
        else if (file_error) {
            gfal2_propagate_prefixed_error(err, file_error[0], __func__);
            g_free(file_error);
        }
    }

    return ret;
}
