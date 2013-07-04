#include <glib.h>
#include <unistd.h>
#include "gfal_http_plugin.h"



gfal_file_handle gfal_http_fopen(plugin_handle plugin_data, const char* url, int flag, mode_t mode, GError** err)
{
  GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
  Davix::DavixError* daverr = NULL;
  DAVIX_FD* fd = davix->posix.open(&davix->params, url, flag, &daverr);
  if (fd == NULL) {
    davix2gliberr(daverr, err);
    Davix::DavixError::clearError(&daverr);
    return NULL;
  }
  return gfal_file_handle_new(http_module_name, fd);
}



ssize_t gfal_http_fread(plugin_handle plugin_data, gfal_file_handle fd, void* buff, size_t count, GError** err)
{
  GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
  Davix::DavixError* daverr = NULL;
  DAVIX_FD* dfd = (DAVIX_FD*)gfal_file_handle_get_fdesc(fd);

  ssize_t reads = davix->posix.read(dfd, buff, count, &daverr);
  if (reads < 0) {
    davix2gliberr(daverr, err);
    Davix::DavixError::clearError(&daverr);
  }

  return reads;
}



ssize_t gfal_http_fwrite(plugin_handle plugin_data, gfal_file_handle fd, const void* buff, size_t count, GError** err)
{
  GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
  Davix::DavixError* daverr = NULL;
  DAVIX_FD* dfd = (DAVIX_FD*)gfal_file_handle_get_fdesc(fd);

  ssize_t writes = davix->posix.write(dfd, buff, count, &daverr);
  if (writes < 0) {
    davix2gliberr(daverr, err);
    Davix::DavixError::clearError(&daverr);
  }

  return writes;
}



int gfal_http_fclose(plugin_handle plugin_data, gfal_file_handle fd, GError ** err)
{
  GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
  Davix::DavixError* daverr = NULL;
  DAVIX_FD* dfd = (DAVIX_FD*)gfal_file_handle_get_fdesc(fd);
  int ret = 0;

  if (davix->posix.close(dfd, &daverr) != 0) {
    davix2gliberr(daverr, err);
    Davix::DavixError::clearError(&daverr);
    ret = -1;
  }

  gfal_file_handle_delete(fd);

  return ret;
}



off_t gfal_http_fseek(plugin_handle plugin_data, gfal_file_handle fd, off_t offset, int whence, GError** err)
{
  GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
  Davix::DavixError* daverr = NULL;
  DAVIX_FD* dfd = (DAVIX_FD*)gfal_file_handle_get_fdesc(fd);

  off_t newOffset = davix->posix.lseek(dfd, offset, whence, &daverr);
  if (newOffset < 0) {
    davix2gliberr(daverr, err);
    Davix::DavixError::clearError(&daverr);
  }

  return newOffset;
}
