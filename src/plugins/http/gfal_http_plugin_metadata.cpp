#include <errno.h>
#include <glib.h>
#include <unistd.h>
#include <common/gfal_common_err_helpers.h>
#include "gfal_http_plugin.h"



int gfal_http_stat(plugin_handle plugin_data, const char* url,
                   struct stat* buf, GError** err)
{
  GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
  Davix::DavixError* daverr = NULL;
  if (davix->posix.stat(&davix->params, url, buf, &daverr) != 0) {
    davix2gliberr(daverr, err);
    Davix::DavixError::clearError(&daverr);
    return -1;
  }
  return 0;
}



int gfal_http_mkdirpG(plugin_handle plugin_data, const char* url, mode_t mode, gboolean rec_flag, GError** err){
  GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
  Davix::DavixError* daverr = NULL;
  if (davix->posix.mkdir(&davix->params, url, mode, &daverr) != 0) {
    davix2gliberr(daverr, err);
    Davix::DavixError::clearError(&daverr);
    return -1;
  }
  return 0;
}



int gfal_http_unlinkG(plugin_handle plugin_data, const char* url, GError** err){
    GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    if (davix->posix.unlink(&davix->params, url, &daverr) != 0) {
      davix2gliberr(daverr, err);
      Davix::DavixError::clearError(&daverr);
      return -1;
    }
    return 0;
}



int gfal_http_rmdirG(plugin_handle plugin_data, const char* url, GError** err){
    GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    if (davix->posix.rmdir(&davix->params, url, &daverr) != 0) {
      davix2gliberr(daverr, err);
      Davix::DavixError::clearError(&daverr);
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
  GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
  Davix::DavixError* daverr = NULL;
  
  DAVIX_DIR* dir = davix->posix.opendirpp(&davix->params, url, &daverr);
  if (dir == NULL) {
    davix2gliberr(daverr, err);
    Davix::DavixError::clearError(&daverr);
    return NULL;
  }
  return gfal_file_handle_new2(http_module_name, dir, NULL, url);
}



struct dirent* gfal_http_readdir(plugin_handle plugin_data,
        gfal_file_handle dir_desc, GError** err)
{
  GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
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
    GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
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


int gfal_http_closedir(plugin_handle plugin_data, gfal_file_handle dir_desc,
                          GError** err)
{
  GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
  Davix::DavixError* daverr = NULL;
  int ret = 0;
  
  if (davix->posix.closedir((DAVIX_DIR*)gfal_file_handle_get_fdesc(dir_desc), &daverr) != 0) {
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
    GfalHttpInternal* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;

    if (start_offset != 0 || data_length != 0) {
        gfal2_set_error(err, http_plugin_domain, ENOTSUP, __func__,
                    "HTTP does not support partial checksums");
        return -1;
    }

    Davix::HttpRequest*  request = davix->context.createRequest(url, &daverr);
    Davix::RequestParams requestParams(davix->params);

    request->setRequestMethod("HEAD");
    request->addHeaderField("Want-Digest", check_type);
    requestParams.setTransparentRedirectionSupport(true);
    request->setParameters(requestParams);

    request->executeRequest(&daverr);
    if (daverr) {
      davix2gliberr(daverr, err);
      delete daverr;
      return -1;
    }

    std::string digest;
    request->getAnswerHeader("Digest", digest);
    delete request;

    if (digest.empty()) {
        gfal2_set_error(err, http_plugin_domain, ENOTSUP, __func__,
                    "No Digest header found for '%s'", url);
        return -1;
    }

    size_t valueOffset = digest.find('=');
    if (valueOffset == std::string::npos) {
        gfal2_set_error(err, http_plugin_domain, ENOTSUP, __func__,
                    "Malformed Digest header from '%s': %s", url, digest.c_str());
        return -1;
    }

    std::string csumtype  = digest.substr(0, valueOffset);
    std::string csumvalue = digest.substr(valueOffset + 1, std::string::npos);

    if (strcasecmp(csumtype.c_str(), check_type) != 0) {
        gfal2_set_error(err, http_plugin_domain, ENOTSUP, __func__,
                    "Asked for checksum %s, got %s: %s", check_type, csumtype.c_str(), url);
        return -1;
    }

    strncpy(checksum_buffer, csumvalue.c_str(), buffer_length);

    return 0;
}
