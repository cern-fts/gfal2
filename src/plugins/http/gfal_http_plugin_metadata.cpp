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

#include <cstring>
#include <cerrno>
#include <glib.h>
#include <unistd.h>
#include "gfal_http_plugin.h"



int gfal_http_stat(plugin_handle plugin_data, const char* url,
                   struct stat* buf, GError** err)
{
    Davix::StatInfo info;
    char stripped_url[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(url, stripped_url, sizeof(stripped_url));
    if(buf==NULL){
        gfal2_set_error(err, http_plugin_domain, EINVAL, __func__, "Invalid stat argument");
        return -1;
    }

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    Davix::RequestParams req_params;
    davix->get_params(&req_params, Davix::Uri(stripped_url));

    // Attempt stat over WebDav first, then fallback to HTTP
    if (req_params.getProtocol() == Davix::RequestProtocol::Http) {
      gfal2_log(G_LOG_LEVEL_DEBUG, "Identified stat over HTTP protocol. Attempting stat over WebDav first");
      req_params.setProtocol(Davix::RequestProtocol::Webdav);
      Davix::StatInfo statInfo;

      if (davix->posix.stat64(&req_params, stripped_url, &statInfo, &daverr) != 0) {
        gfal2_log(G_LOG_LEVEL_MESSAGE, "Stat over WebDav failed with error: %s. Will fallback to HTTP protocol", daverr->getErrMsg().c_str());
        Davix::DavixError::clearError(&daverr);
        req_params.setProtocol(Davix::RequestProtocol::Http);
      } else {
        statInfo.toPosixStat(*buf);
        return 0;
      }
    }

    if (davix->posix.stat64(&req_params, stripped_url, &info, &daverr) != 0) {
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
        return -1;
    }
    info.toPosixStat(*buf);
    return 0;
}



int gfal_http_mkdirpG(plugin_handle plugin_data, const char* url, mode_t mode, gboolean rec_flag, GError** err)
{
    char stripped_url[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(url, stripped_url, sizeof(stripped_url));

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    Davix::Uri uri(stripped_url);
    Davix::RequestParams req_params;
    bool retrieve_token = gfal2_get_opt_boolean_with_default(davix->handle, "HTTP PLUGIN", "RETRIEVE_BEARER_TOKEN", false);

    if (retrieve_token) {
        gchar *token = davix->find_se_token(uri, true, true);

        if (!token) {
            g_free(token);
            std::string reserved(stripped_url);
            if (reserved.back() != '/') {
                reserved.push_back('/');
            }
            reserved += "gfal2_mkdir.reserved";

            Davix::Uri reserved_uri(reserved);
            req_params = davix->reference_params;
            davix->get_params_internal(req_params, reserved_uri);
            davix->retrieve_and_store_se_token(req_params, reserved_uri, true, 60);
        }
    }

    davix->get_params(&req_params, uri, true, true);
    if (davix->posix.mkdir(&req_params, stripped_url, mode, &daverr) != 0) {
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
        return -1;
    }
    return 0;
}



int gfal_http_unlinkG(plugin_handle plugin_data, const char* url, GError** err)
{
    char stripped_url[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(url, stripped_url, sizeof(stripped_url));

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;

    Davix::RequestParams req_params;
    davix->get_params(&req_params, Davix::Uri(stripped_url), true);
    req_params.setMetalinkMode(Davix::MetalinkMode::Disable);

    if (davix->posix.unlink(&req_params, stripped_url, &daverr) != 0) {
      davix2gliberr(daverr, err);
      Davix::DavixError::clearError(&daverr);
      return -1;
    }
    return 0;
}



int gfal_http_rmdirG(plugin_handle plugin_data, const char* url, GError** err)
{
    char stripped_url[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(url, stripped_url, sizeof(stripped_url));
    struct stat st;

    if (gfal_http_stat(plugin_data, stripped_url, &st, err) != 0) {
        return -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        gfal2_set_error(err, http_plugin_domain, ENOTDIR, __func__,
                        "Can not rmdir a file");
        return -1;
    }

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;

    Davix::RequestParams req_params;
    davix->get_params(&req_params, Davix::Uri(stripped_url), true);

    if (davix->posix.rmdir(&req_params, stripped_url, &daverr) != 0) {
      davix2gliberr(daverr, err);
      Davix::DavixError::clearError(&daverr);
      return -1;
    }
    return 0;
}


int gfal_http_rename(plugin_handle plugin_data, const char* oldurl, const char* newurl, GError** err)
{
    char stripped_old[GFAL_URL_MAX_LEN];
    char stripped_new[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(oldurl, stripped_old, sizeof(stripped_old));
    strip_3rd_from_url(newurl, stripped_new, sizeof(stripped_new));

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;

    Davix::RequestParams req_params;
    davix->get_params(&req_params, Davix::Uri(stripped_old), true);

    if (davix->posix.rename(&req_params, stripped_old, stripped_new, &daverr) != 0) {
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
        return -1;
    }
    return 0;
}


int gfal_http_access(plugin_handle plugin_data, const char* url, int mode, GError** err)
{
    struct stat buf;
    GError* tmp_err = NULL;

    memset(&buf, 0, sizeof(buf));
    if (gfal_http_stat(plugin_data, url, &buf, &tmp_err) != 0) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }

    uid_t real_uid = getuid();
    gid_t real_gid = getgid();

    int ngroups = getgroups(0, NULL);
    if (ngroups < 0) {
        gfal2_set_error(err, http_plugin_domain, errno, __func__,
                "Could not get the groups of the current user");
        return -1;
    }

    gid_t additional_gids[ngroups];
    getgroups(ngroups, additional_gids);

    if (real_uid == buf.st_uid)
        mode <<= 6;
    else if (real_gid == buf.st_gid)
        mode <<= 3;
    else {
        for (int i = 0; i < ngroups; ++i) {
            if (additional_gids[i] == buf.st_gid) {
                mode <<= 3;
                break;
            }
        }
    }

    if ((mode & buf.st_mode) != static_cast<mode_t>(mode)) {
        gfal2_set_error(err, http_plugin_domain, EACCES, __func__,
                "Does not have enough permissions on '%s'", url);
        return -1;
    }
    else {
        return 0;
    }
}



gfal_file_handle gfal_http_opendir(plugin_handle plugin_data, const char* url,
                                   GError** err)
{
    char stripped_url[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(url, stripped_url, sizeof(stripped_url));

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;

    Davix::RequestParams req_params;
    davix->get_params(&req_params, Davix::Uri(stripped_url));

    DAVIX_DIR* dir = davix->posix.opendirpp(&req_params, stripped_url, &daverr);
    if (dir == NULL) {
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
        return NULL;
    }
    return gfal_file_handle_new2(gfal_http_get_name(), dir, NULL, url);
}



struct dirent* gfal_http_readdir(plugin_handle plugin_data,
        gfal_file_handle dir_desc, GError** err)
{
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;

    daverr = NULL;
    struct stat _;
    struct dirent* de = davix->posix.readdirpp((DAVIX_DIR*)gfal_file_handle_get_fdesc(dir_desc),
                                             &_, &daverr);
    if (de == NULL && daverr != NULL) {
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
    }
    return de;
}



struct dirent* gfal_http_readdirpp(plugin_handle plugin_data,
        gfal_file_handle dir_desc, struct stat* st, GError** err)
{
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;

    daverr = NULL;
    struct dirent* de = davix->posix.readdirpp((DAVIX_DIR*)gfal_file_handle_get_fdesc(dir_desc),
                                               st, &daverr);
    if (de == NULL && daverr != NULL) {
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
    }
    return de;
}


int gfal_http_closedir(plugin_handle plugin_data, gfal_file_handle dir_desc, GError** err)
{
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    int ret = 0;

    if (davix->posix.closedir((DAVIX_DIR*) gfal_file_handle_get_fdesc(dir_desc), &daverr) != 0) {
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
        ret = -1;
    }
    gfal_file_handle_delete(dir_desc);
    return ret;
}



int gfal_http_checksum(plugin_handle plugin_data, const char* url, const char* check_type,
                       char * checksum_buffer, size_t buffer_length,
                       off_t start_offset, size_t data_length,
                       GError ** err)
{
    char stripped_url[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(url, stripped_url, sizeof(stripped_url));

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    std::string buffer_chk, algo_chk(check_type);

    if (start_offset != 0 || data_length != 0) {
        gfal2_set_error(err, http_plugin_domain, ENOTSUP, __func__,
                    "HTTP does not support partial checksums");
        return -1;
    }

    Davix::RequestParams req_params;
    davix->get_params(&req_params, Davix::Uri(stripped_url));

    // Override timeout with checksum timeout
    struct timespec opTimeout;
    opTimeout.tv_sec = gfal2_get_opt_integer_with_default(davix->handle,
        CORE_CONFIG_GROUP, CORE_CONFIG_CHECKSUM_TIMEOUT, 300);
    req_params.setOperationTimeout(&opTimeout);
    // thois is needed by DPM + DOME as it implements a queue for checksum calculation
    req_params.setAcceptedRetry(100);
    req_params.setAcceptedRetryDelay(15);

    Davix::File f(davix->context, Davix::Uri(stripped_url));
    if(f.checksum(&req_params, buffer_chk, check_type, &daverr) <0 ){
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
    }

    g_strlcpy(checksum_buffer, buffer_chk.c_str(), buffer_length);
    return 0;
}

