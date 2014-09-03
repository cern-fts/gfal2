#include "gfal_http_plugin.h"
#include <cstdio>
#include <davix.hpp>
#include <errno.h>
#include <common/gfal_common_err_helpers.h>
#include <config/gfal_config.h>
#include <logger/gfal_logger.h>

using namespace Davix;

const char* http_module_name = "http_plugin";
GQuark http_plugin_domain = g_quark_from_static_string(http_module_name);


static const char* gfal_http_get_name(void)
{
    return http_module_name;
}


static void gfal_http_refresh_params(gfal2_context_t handle, Davix::RequestParams& params)
{
    // disable SSL verification
    gboolean insecure_mode = gfal2_get_opt_boolean(handle, "HTTP PLUGIN", "INSECURE", NULL);
    if (insecure_mode) {
        params.setSSLCAcheck(false);
    }
}


GfalHttpPluginData::GfalHttpPluginData(gfal2_context_t handle):
    context(), posix(&context), params(), handle(handle)
{
    params.setTransparentRedirectionSupport(true);
    params.setUserAgent("gfal2::http");
    gfal_http_get_ucert(params, handle);
    gfal_http_get_aws(params, handle);

    // enable grid mode
    context.loadModule("grid");


    gfal_http_refresh_params(handle, params);
}


GfalHttpPluginData* gfal_http_get_plugin_context(gpointer ptr)
{
    GfalHttpPluginData* plugin_data = static_cast<GfalHttpPluginData*>(ptr);
    // Some parameters may change at run time!
    gfal_http_refresh_params(plugin_data->handle, plugin_data->params);
    return plugin_data;
}


void gfal_http_context_delete(gpointer plugin_data){
    GfalHttpPluginData* data = static_cast<GfalHttpPluginData*>(plugin_data);
    delete data;
}


void gfal_http_delete(plugin_handle plugin_data)
{
    gfal_http_context_delete(plugin_data);
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
                 strncmp("dav:", url, 4) == 0 || strncmp("davs:", url, 5) == 0 ||
                 strncmp("s3:", url, 3) == 0 || strncmp("s3s:", url, 4) == 0 ||
                 strncmp("http+3rd:", url, 9) == 0 || strncmp("https+3rd:", url, 10) == 0 ||
                 strncmp("dav+3rd:", url, 8) == 0 || strncmp("davs+3rd:", url, 9) == 0);
      default:
        return false;
    }
}



static int davix2errno(StatusCode::Code code)
{
    int errcode;

    switch (code) {
        case StatusCode::OK:
        case StatusCode::PartialDone:
            errcode = 0;
            break;
        case StatusCode::WebDavPropertiesParsingError:
        case StatusCode::UriParsingError:
            errcode = EIO;
            break;

        case StatusCode::SessionCreationError:
            errcode = EPERM;
            break;

        case StatusCode::NameResolutionFailure:
            errcode = EHOSTUNREACH;
            break;

        case StatusCode::ConnectionProblem:
            errcode = EHOSTDOWN;
            break;

        case StatusCode::RedirectionNeeded:
            errcode = ENOSYS;
            break;

        case StatusCode::ConnectionTimeout:
        case StatusCode::OperationTimeout:
            errcode = ETIMEDOUT;
            break;

        case StatusCode::PermissionRefused:
            errcode = EPERM;
            break;

        case StatusCode::IsNotADirectory:
            errcode = ENOTDIR;
            break;

        case StatusCode::InvalidFileHandle:
            errcode = EBADF;
            break;

        case StatusCode::AuthentificationError:
        case StatusCode::LoginPasswordError:
        case StatusCode::CredentialNotFound:
        case StatusCode::CredDecryptionError:
        case StatusCode::SSLError:
            errcode = EACCES;
            break;

        case StatusCode::FileNotFound:
            errcode = ENOENT;
            break;

        case StatusCode::FileExist:
            errcode = EEXIST;
            break;

        default:
            errcode = EIO;
            break;
    }

    return errcode;
}


void davix2gliberr(const DavixError* daverr, GError** err)
{
    gfal2_set_error(err, http_plugin_domain, davix2errno(daverr->getStatus()), __func__,
              "%s", daverr->getErrMsg().c_str());
}


static int http2errno(int http)
{
    if (http < 400)
        return 0;
    switch (http) {
        case 400: case 406:
            return EINVAL;
        case 401: case 402: case 403:
            return EACCES;
        case 404: case 410:
            return ENOENT;
        case 405:
            return EPERM;
        case 409:
            return EEXIST;
        case 501:
            return ENOSYS;
        default:
            if (http >= 400 && http < 500)
                return EINVAL;
            else
                return ECOMM;
    }
}


void http2gliberr(GError** err, int http, const char* func, const char* msg)
{
    int errn = http2errno(http);
    char buffer[512] = {0};
    strerror_r(errn, buffer, sizeof(buffer));
    gfal2_set_error(err, http_plugin_domain, errn, func, "%s: %s (HTTP %d)", msg, buffer, http);
}


/// Authn implementation
void gfal_http_get_ucert(RequestParams & params, gfal2_context_t handle)
{
    std::string ukey, ucert;
    DavixError* tmp_err=NULL;

    // Try user defined first
    gchar *ucert_p = gfal2_get_opt_string(handle, "X509", "CERT", NULL);
    gchar *ukey_p = gfal2_get_opt_string(handle, "X509", "KEY", NULL);
    if (ucert_p) {
        ucert.assign(ucert_p);
        ukey= (ukey_p != NULL)?(std::string(ukey_p)):(ucert);

        X509Credential cred;
        if(cred.loadFromFilePEM(ukey,ucert,"", &tmp_err) <0){
            gfal_log(GFAL_VERBOSE_VERBOSE,
                    "Could not load the user credentials: %s", tmp_err->getErrMsg().c_str());
        }else{
            params.setClientCertX509(cred);
        }
    }
    g_free(ucert_p);
    g_free(ukey_p);
}

/// AWS implementation
void gfal_http_get_aws(RequestParams & params, gfal2_context_t handle)
{
    gchar *secret_key = gfal2_get_opt_string(handle, "S3", "ACCESS_TOKEN_SECRET", NULL);
    gchar *access_key = gfal2_get_opt_string(handle, "S3", "ACCESS_TOKEN", NULL);

    if (secret_key && access_key) {
        gfal_log(GFAL_VERBOSE_DEBUG, "Setting S3 key pair");
        params.setAwsAuthorizationKeys(secret_key, access_key);
    }

    g_free(secret_key);
    g_free(access_key);
}

/// Init function
extern "C" gfal_plugin_interface gfal_plugin_init(gfal2_context_t handle, GError** err)
{
    gfal_plugin_interface http_plugin;

    *err = NULL;
    memset(&http_plugin, 0, sizeof(http_plugin));

    // Bind metadata
    http_plugin.check_plugin_url = &gfal_http_check_url;
    http_plugin.getName = &gfal_http_get_name;
    http_plugin.priority = GFAL_PLUGIN_PRIORITY_DATA
    ;
    http_plugin.plugin_data = new GfalHttpPluginData(handle);
    http_plugin.plugin_delete = &gfal_http_delete;

    http_plugin.statG = &gfal_http_stat;
    http_plugin.accessG = &gfal_http_access;
    http_plugin.mkdirpG = &gfal_http_mkdirpG;
    http_plugin.unlinkG = &gfal_http_unlinkG;
    http_plugin.rmdirG = &gfal_http_rmdirG;
    http_plugin.opendirG = &gfal_http_opendir;
    http_plugin.readdirG = &gfal_http_readdir;
    http_plugin.readdirppG = &gfal_http_readdirpp;
    http_plugin.closedirG = &gfal_http_closedir;

    // Bind IO
    http_plugin.openG = &gfal_http_fopen;
    http_plugin.readG = &gfal_http_fread;
    http_plugin.writeG = &gfal_http_fwrite;
    http_plugin.lseekG = &gfal_http_fseek;
    http_plugin.closeG = &gfal_http_fclose;

    // Checksum
    http_plugin.checksum_calcG = &gfal_http_checksum;

    // Bind 3rd party copy
    http_plugin.check_plugin_url_transfer = gfal_http_copy_check;
    http_plugin.copy_file = gfal_http_copy;

    return http_plugin;
}
