#include "gfal_http_plugin.h"
#include <errno.h>
#include <davix.hpp>
#include <status/davix_error.h>


const char* http_module_name = "http_plugin";
GQuark http_plugin_domain = g_quark_from_static_string(http_module_name);



static const char* gfal_http_get_name(void)
{
  return http_module_name;
}



static gboolean gfal_http_check_url(plugin_handle plugin_data, const char* url,
                                    plugin_mode operation, GError** err)
{
    switch(operation){
        case GFAL_PLUGIN_ACCESS:
        case GFAL_PLUGIN_OPEN:
        case GFAL_PLUGIN_STAT:
        case GFAL_PLUGIN_MKDIR:
        case GFAL_PLUGIN_OPENDIR:
        case GFAL_PLUGIN_RMDIR:
        case GFAL_PLUGIN_UNLINK:
        case GFAL_PLUGIN_CHECKSUM:
            return (strncmp("http:", url, 5) == 0 || strncmp("https:", url, 6) == 0 ||
                 strncmp("dav:", url, 4) == 0 || strncmp("davs:", url, 5) == 0);
      default:
        return false;
    }
}



static int davix2errno(Davix::StatusCode::Code code)
{
  int errcode;

  switch (code) {
    case DAVIX_STATUS_OK:
    case DAVIX_STATUS_PARTIAL_DONE:
      errcode = 0;
      break;
    case DAVIX_STATUS_WEBDAV_PROPERTIES_PARSING_ERROR:
    case DAVIX_STATUS_URI_PARSING_ERROR:
      errcode = EIO;
      break;

    case DAVIX_STATUS_SESSION_CREATION_ERROR:
      errcode = EPERM;
      break;

    case DAVIX_STATUS_NAME_RESOLUTION_FAILURE:
      errcode = EHOSTUNREACH;
      break;

    case DAVIX_STATUS_CONNECTION_PROBLEM:
      errcode = EHOSTDOWN;
      break;

    case DAVIX_STATUS_REDIRECTION_NEEDED:
      errcode = ENOSYS;
      break;

    case DAVIX_STATUS_CONNECTION_TIMEOUT:
    case DAVIX_STATUS_OPERATION_TIMEOUT:
      errcode = ETIMEDOUT;
      break;

    case DAVIX_STATUS_OPERATION_NOT_SUPPORTED:
      errcode = EPERM;
      break;

    case DAVIX_STATUS_IS_NOT_A_DIRECTORY:
      errcode = ENOTDIR;
      break;

    case DAVIX_STATUS_INVALID_FILE_HANDLE:
      errcode = EBADF;
      break;

    case DAVIX_STATUS_AUTHENTIFICATION_ERROR:
    case DAVIX_STATUS_LOGIN_PASSWORD_ERROR:
    case DAVIX_STATUS_CREDENTIAL_NOT_FOUND:
    case DAVIX_STATUS_PERMISSION_REFUSED:
      errcode = EACCES;
      break;

    case DAVIX_STATUS_FILE_NOT_FOUND:
      errcode = ENOENT;
      break;

    default:
      errcode = EIO;
      break;
  }

  return errcode;
}



void davix2gliberr(const Davix::DavixError* daverr, GError** err)
{
  g_set_error(err, http_plugin_domain, davix2errno(daverr->getStatus()),
              "%s", daverr->getErrMsg().c_str());
}


/// Authn implementation
int gfal_http_authn_cert_X509(void* userdata, const Davix::SessionInfo & info, Davix::X509Credential * cert, Davix::DavixError** err){


  // Convert X509 to PKCS12
  char *ucert, *ukey;
  ucert = ukey = getenv("X509_USER_PROXY");
  if (!ucert) {
    ucert = getenv("X509_USER_CERT");
    ukey  = getenv("X509_USER_KEY");
  }

  if (!ucert || !ukey) {
    Davix::DavixError::setupError(err, http_module_name, Davix::StatusCode::AuthentificationError,
                      "Could not set the user's proxy or certificate");
    return -1;
  }

  char p12file[PATH_MAX];
  snprintf(p12file, sizeof(p12file), "/tmp/gfal_http_authn_p%d.p12", getpid());
  if (convert_x509_to_p12(ukey, ucert, p12file, (davix_error_t*)err) < 0) {
    Davix::DavixError::setupError(err, http_module_name, Davix::StatusCode::AuthentificationError,
                                "Can not convert credential correctly");
    return -1;
  }


  cert->loadFromFileP12(p12file, "", err);
  // Set certificate
  return 0;
}



/// Init function
extern "C" gfal_plugin_interface gfal_plugin_init(gfal_handle handle,
                                                  GError** err)
{
  gfal_plugin_interface http_plugin;
  GfalHttpInternal* http_internal = new GfalHttpInternal();
  char * env_var = NULL;

  http_internal->context = new Davix::Context();
  http_internal->params  = new Davix::RequestParams();
  http_internal->posix   = new Davix::DavPosix(http_internal->context);

  *err = NULL;
  memset(&http_plugin, 0, sizeof(http_plugin));

  // Configure params
  http_internal->params->addCertificateAuthorityPath( // TODO : thix should be configurable with gfal_get_opt system
              (env_var = (char*) g_getenv("X509_CERT_DIR"))?env_var:"/etc/grid-security/certificates/");

  http_internal->params->setTransparentRedirectionSupport(true);
  http_internal->params->setUserAgent("gfal2::http");
  http_internal->params->setClientCertCallbackX509(&gfal_http_authn_cert_X509, NULL);

  // Bind metadata
  http_plugin.check_plugin_url = &gfal_http_check_url;
  http_plugin.getName          = &gfal_http_get_name;
  http_plugin.priority         = GFAL_PLUGIN_PRIORITY_DATA;
  http_plugin.plugin_data      = http_internal;
  http_plugin.plugin_delete    = &gfal_http_delete;

  http_plugin.statG     = &gfal_http_stat;
  http_plugin.accessG   = &gfal_http_access;
  http_plugin.mkdirpG  = &gfal_http_mkdirpG;
  http_plugin.unlinkG = &gfal_http_unlinkG;
  http_plugin.rmdirG = &gfal_http_rmdirG;
  http_plugin.opendirG  = &gfal_http_opendir;
  http_plugin.readdirG  = &gfal_http_readdir;
  http_plugin.closedirG = &gfal_http_closedir;

  // Bind IO
  http_plugin.openG  = &gfal_http_fopen;
  http_plugin.readG  = &gfal_http_fread;
  http_plugin.writeG = &gfal_http_fwrite;
  http_plugin.lseekG = &gfal_http_fseek;
  http_plugin.closeG = &gfal_http_fclose;

  // Checksum
  http_plugin.checksum_calcG = &gfal_http_checksum;

  // Bind 3rd party copy
  http_plugin.check_plugin_url_transfer = gfal_http_3rdcopy_check;
  http_plugin.copy_file = gfal_http_3rdcopy;

  return http_plugin;
}
