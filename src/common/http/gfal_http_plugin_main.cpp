#include <davix_cpp.hpp>
#include <unistd.h>
#include "../gfal_common_errverbose.h"
#include "../fdesc/gfal_file_handle.h"
#include "../gfal_common_internal.h"
#include "../gfal_common_plugin.h"
#include "../gfal_types.h"

struct GfalHttpInternal {
    Davix::Context*       context;
    Davix::DavPosix*      posix;
    Davix::RequestParams* params;
};


const char* http_module_name = "http_plugin";
GQuark http_plugin_domain = g_quark_from_static_string(http_module_name);



static gboolean gfal_http_check_url(plugin_handle plugin_data, const char* url,
                                    plugin_mode operation, GError** err)
{
  return (strncmp("http:", url, 5) == 0 || strncmp("https:", url, 6) == 0 ||
          strncmp("dav:", url, 4) == 0 || strncmp("davs:", url, 5) == 0);
}



static const char* gfal_http_get_name(void)
{
  return http_module_name;
}



static void gfal_http_delete(plugin_handle plugin_data)
{
  GfalHttpInternal* davix = static_cast<GfalHttpInternal*>(plugin_data);
  delete davix->posix;
  delete davix->params;
  delete davix->context;
  delete davix;
}



static int gfal_http_stat(plugin_handle plugin_data, const char* url,
                          struct stat* buf, GError** err)
{
  GfalHttpInternal* davix = static_cast<GfalHttpInternal*>(plugin_data);
  
  try {
    davix->posix->stat(davix->params, url, buf);
    return 0;
  }
  catch (Glib::Error& e) {
    GError* tmp_err;
    tmp_err = g_error_new(http_plugin_domain, e.code(), "%s", e.what().c_str());
    g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
  }
  catch (...) {
    g_set_error(err, 0, EFAULT, "Unexpected exception catched on '%s'", __func__);
  }
  return -1;
}



static int gfal_http_access(plugin_handle plugin_data, const char* url,
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
  
  try {
    DAVIX_DIR* dir = davix->posix->opendir(davix->params, url);
    return gfal_file_handle_new(http_module_name, dir);
  }
  catch (Glib::Error& e) {
    GError* tmp_err;
    tmp_err = g_error_new(http_plugin_domain, e.code(), "%s", e.what().c_str());
    g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
  }
  catch (...) {
    g_set_error(err, 0, EFAULT, "Unexpected exception catched on '%s'", __func__);
  }
  return NULL;
}



struct dirent* gfal_http_readdir(plugin_handle plugin_data,
                                    gfal_file_handle dir_desc, GError** err)
{
  GfalHttpInternal* davix = static_cast<GfalHttpInternal*>(plugin_data);
  
  try {
    return davix->posix->readdir(gfal_file_handle_get_fdesc(dir_desc));
  }
  catch (Glib::Error& e) {
    GError* tmp_err;
    tmp_err = g_error_new(http_plugin_domain, e.code(), "%s", e.what().c_str());
    g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
  }
  catch (...) {
    g_set_error(err, 0, EFAULT, "Unexpected exception catched on '%s'", __func__);
  }
  return NULL;
}



int gfal_http_closedir(plugin_handle plugin_data, gfal_file_handle dir_desc,
                          GError** err)
{
  GfalHttpInternal* davix = static_cast<GfalHttpInternal*>(plugin_data);
  int ret = 0;
  
  try {
    davix->posix->closedir(gfal_file_handle_get_fdesc(dir_desc));
  }
  catch (Glib::Error& e) {
    GError* tmp_err;
    tmp_err = g_error_new(http_plugin_domain, e.code(), "%s", e.what().c_str());
    g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
    ret = -1;
  }
  catch (...) {
    g_set_error(err, 0, EFAULT, "Unexpected exception caught on '%s'", __func__);
    ret = -1;
  }
  
  gfal_file_handle_delete(dir_desc);
  
  return ret;
}



/*
 * Init function
 */
extern "C" gfal_plugin_interface gfal_plugin_init(gfal_handle handle,
                                                  GError** err)
{
  gfal_plugin_interface http_plugin;
  GfalHttpInternal* http_internal = new GfalHttpInternal();
  
  http_internal->context = new Davix::Context();
  http_internal->params  = new Davix::RequestParams();
  http_internal->posix   = new Davix::DavPosix(http_internal->context);

  *err = NULL;
  memset(&http_plugin, 0, sizeof(http_plugin));
  
  http_plugin.check_plugin_url = &gfal_http_check_url;
  http_plugin.getName          = &gfal_http_get_name;
  http_plugin.priority         = GFAL_PLUGIN_PRIORITY_DATA;
  http_plugin.plugin_data      = http_internal;
  http_plugin.plugin_delete    = &gfal_http_delete;
  
  http_plugin.statG     = &gfal_http_stat;
  http_plugin.accessG   = &gfal_http_access;
  http_plugin.opendirG  = &gfal_http_opendir;
  http_plugin.readdirG  = &gfal_http_readdir;
  http_plugin.closedirG = &gfal_http_closedir;
  
  return http_plugin;
}
