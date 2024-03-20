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
#include <glib.h>
#include <unistd.h>
#include "gfal_http_plugin.h"


struct GfalHTTPFD {
    Davix::RequestParams req_params;
    DAVIX_FD* davix_fd;
};



gfal_file_handle gfal_http_fopen(plugin_handle plugin_data, const char* url, int flag, mode_t mode,
        GError** err)
{
    char stripped_url[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(url, stripped_url, sizeof(stripped_url));

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;

    GfalHTTPFD* fd = new GfalHTTPFD();
    GfalHttpPluginData::OP operation = (flag & O_WRONLY) ?
            GfalHttpPluginData::OP::WRITE : GfalHttpPluginData::OP::READ;
    davix->get_params(&fd->req_params, Davix::Uri(stripped_url), operation);
    if (strncmp("s3:", url, 3) == 0 || strncmp("s3s:", url, 4) == 0) {
        fd->req_params.setProtocol(Davix::RequestProtocol::AwsS3);
    }
    else if (strncmp("gcloud:", url, 7) == 0 || strncmp("gclouds:", url, 8) == 0) {
        fd->req_params.setProtocol(Davix::RequestProtocol::Gcloud);
    }
    else if (strncmp("swift:", url, 6) == 0 || strncmp("swifts:", url, 7) == 0) {
        fd->req_params.setProtocol(Davix::RequestProtocol::Swift);
    }
    else if (strncmp("cs3:", url, 4) == 0 || strncmp("cs3s:", url, 5) == 0) {
        fd->req_params.setProtocol(Davix::RequestProtocol::CS3);
    }

    // DMC-1348: Use resolved URLs for data operations
    std::string resolved_url = davix->resolved_url(stripped_url);
    fd->davix_fd = davix->posix.open(&fd->req_params, resolved_url, flag, &daverr);

    if (fd->davix_fd == NULL) {
        davix2gliberr(daverr, err, __func__);
        Davix::DavixError::clearError(&daverr);
        delete fd;
        return NULL;
    }
    return gfal_file_handle_new(gfal_http_get_name(), fd);
}



ssize_t gfal_http_fread(plugin_handle plugin_data, gfal_file_handle fd, void* buff, size_t count,
        GError** err)
{
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    GfalHTTPFD* dfd = (GfalHTTPFD*) gfal_file_handle_get_fdesc(fd);

    ssize_t reads = davix->posix.read(dfd->davix_fd, buff, count, &daverr);
    if (reads < 0) {
        davix2gliberr(daverr, err, __func__);
        Davix::DavixError::clearError(&daverr);
    }

    return reads;
}



ssize_t gfal_http_fwrite(plugin_handle plugin_data, gfal_file_handle fd, const void* buff,
        size_t count, GError** err)
{
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    GfalHTTPFD* dfd = (GfalHTTPFD*) gfal_file_handle_get_fdesc(fd);

    ssize_t writes = davix->posix.write(dfd->davix_fd, buff, count, &daverr);
    if (writes < 0) {
        davix2gliberr(daverr, err, __func__);
        Davix::DavixError::clearError(&daverr);
    }

    return writes;
}



int gfal_http_fclose(plugin_handle plugin_data, gfal_file_handle fd, GError ** err)
{
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    GfalHTTPFD* dfd = (GfalHTTPFD*) gfal_file_handle_get_fdesc(fd);
    int ret = 0;

    if (davix->posix.close(dfd->davix_fd, &daverr) != 0) {
        davix2gliberr(daverr, err, __func__);
        Davix::DavixError::clearError(&daverr);
        ret = -1;
    }

    gfal_file_handle_delete(fd);

    return ret;
}



off_t gfal_http_fseek(plugin_handle plugin_data, gfal_file_handle fd, off_t offset, int whence,
        GError** err)
{
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    GfalHTTPFD* dfd = (GfalHTTPFD*) gfal_file_handle_get_fdesc(fd);

    off_t newOffset = static_cast<off_t>(davix->posix.lseek64(dfd->davix_fd,
            static_cast<dav_off_t>(offset), whence, &daverr));
    if (newOffset < 0) {
        davix2gliberr(daverr, err, __func__);
        Davix::DavixError::clearError(&daverr);
    }

    return newOffset;
}
