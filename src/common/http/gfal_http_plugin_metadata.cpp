#include <glib/gerror.h>
#include <unistd.h>
#include "gfal_http_plugin.h"



void gfal_http_delete(plugin_handle plugin_data)
{
  GfalHttpInternal* davix = static_cast<GfalHttpInternal*>(plugin_data);
  delete davix->posix;
  delete davix->params;
  delete davix->context;
  delete davix;
}



int gfal_http_stat(plugin_handle plugin_data, const char* url,
                   struct stat* buf, GError** err)
{
  GfalHttpInternal* davix = static_cast<GfalHttpInternal*>(plugin_data);
  Davix::DavixError* daverr = NULL;
  if (davix->posix->stat(davix->params, url, buf, &daverr) != 0) {
    davix2gliberr(daverr, err);
    delete daverr;
    return -1;
  }
  return 0;
}

int gfal_http_mkdirpG(plugin_handle plugin_data, const char* url, mode_t mode, gboolean rec_flag, GError** err){
  GfalHttpInternal* davix = static_cast<GfalHttpInternal*>(plugin_data);
  Davix::DavixError* daverr = NULL;
  if (davix->posix->mkdir(davix->params, url, mode, &daverr) != 0) {
    davix2gliberr(daverr, err);
    delete daverr;
    return -1;
  }
  return 0;
}

int gfal_http_access(plugin_handle plugin_data, const char* url,
                     int mode, GError** err)
{
  struct stat buf;
  GError*     tmp_err = NULL;
  
  if (gfal_http_stat(plugin_data, url, &buf, &tmp_err) != 0) {
    g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
    return -1;
  }
  
  uid_t real_uid = getuid();
  gid_t real_gid = getgid();
  
  int ngroups = getgroups(0, NULL);
  gid_t additional_gids[ngroups];
  getgroups(ngroups, additional_gids);
  
  if (real_uid == buf.st_uid)
    mode <<=  6;
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
    g_set_error(err, http_plugin_domain, EACCES,
                "[%s] Does not have enough permissions on '%s'", __func__, url);
    return -1;
  }
  else {
    return 0;
  }
}



gfal_file_handle gfal_http_opendir(plugin_handle plugin_data, const char* url,
                                   GError** err)
{
  GfalHttpInternal* davix = static_cast<GfalHttpInternal*>(plugin_data);
  Davix::DavixError* daverr = NULL;
  
  DAVIX_DIR* dir = davix->posix->opendir(davix->params, url, &daverr);
  if (dir == NULL) {
    davix2gliberr(daverr, err);
    delete daverr;
    return NULL;
  }
  return gfal_file_handle_new(http_module_name, dir);
}



struct dirent* gfal_http_readdir(plugin_handle plugin_data,
                                 gfal_file_handle dir_desc, GError** err)
{
  GfalHttpInternal* davix = static_cast<GfalHttpInternal*>(plugin_data);
  Davix::DavixError* daverr = NULL;
  
  daverr = NULL;
  struct dirent* de = davix->posix->readdir((DAVIX_DIR*)gfal_file_handle_get_fdesc(dir_desc),
                                            &daverr);
  if (de == NULL && daverr != NULL) {
    davix2gliberr(daverr, err);
    delete daverr;
  }
  return de;
}



int gfal_http_closedir(plugin_handle plugin_data, gfal_file_handle dir_desc,
                          GError** err)
{
  GfalHttpInternal* davix = static_cast<GfalHttpInternal*>(plugin_data);
  Davix::DavixError* daverr = NULL;
  int ret = 0;
  
  if (davix->posix->closedir((DAVIX_DIR*)gfal_file_handle_get_fdesc(dir_desc), &daverr) != 0) {
    davix2gliberr(daverr, err);
    delete daverr;
    ret = -1;
  }
  gfal_file_handle_delete(dir_desc);
  return ret;
}
