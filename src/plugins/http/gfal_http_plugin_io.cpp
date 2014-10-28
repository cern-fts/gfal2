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
    davix->get_params(&fd->req_params, Davix::Uri(stripped_url));
    if (strncmp("s3:", url, 3) == 0 || strncmp("s3s:", url, 4) == 0)
        fd->req_params.setProtocol(Davix::RequestProtocol::AwsS3);

    fd->davix_fd = davix->posix.open(&fd->req_params, stripped_url, flag, &daverr);

    if (fd->davix_fd == NULL) {
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
        delete fd;
        return NULL;
    }
    return gfal_file_handle_new(http_module_name, fd);
}



ssize_t gfal_http_fread(plugin_handle plugin_data, gfal_file_handle fd, void* buff, size_t count,
        GError** err)
{
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    GfalHTTPFD* dfd = (GfalHTTPFD*) gfal_file_handle_get_fdesc(fd);

    ssize_t reads = davix->posix.read(dfd->davix_fd, buff, count, &daverr);
    if (reads < 0) {
        davix2gliberr(daverr, err);
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
        davix2gliberr(daverr, err);
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
        davix2gliberr(daverr, err);
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
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
    }

    return newOffset;
}
