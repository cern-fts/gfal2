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

#include "gfal_http_plugin.h"
#include <cstdio>
#include <cstring>
#include <sstream>
#include <davix.hpp>
#include <errno.h>
#include <common/gfal_common_err_helpers.h>
#include <config/gfal_config.h>
#include <logger/gfal_logger.h>

using namespace Davix;

static const char* http_module_name = "http_plugin";
GQuark http_plugin_domain = g_quark_from_static_string(http_module_name);


const char* gfal_http_get_name(void)
{
    return GFAL2_PLUGIN_VERSIONED("http", VERSION);;
}


static int get_corresponding_davix_log_level()
{
    int davix_log_level = DAVIX_LOG_CRITICAL;
    GLogLevelFlags gfal2_log_level = gfal2_log_get_level();

    if (gfal2_log_level & G_LOG_LEVEL_DEBUG)
        davix_log_level = DAVIX_LOG_TRACE;
    else if (gfal2_log_level & G_LOG_LEVEL_INFO)
        davix_log_level = DAVIX_LOG_VERBOSE;

    return davix_log_level;
}


/// Authn implementation
static void gfal_http_get_ucert(RequestParams & params, gfal2_context_t handle)
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
            gfal2_log(G_LOG_LEVEL_WARNING,
                    "Could not load the user credentials: %s", tmp_err->getErrMsg().c_str());
        }else{
            params.setClientCertX509(cred);
        }
    }
    g_free(ucert_p);
    g_free(ukey_p);
}


/// AWS implementation
static void gfal_http_get_aws_keys(gfal2_context_t handle, const std::string& group,
        gchar** secret_key, gchar** access_key, gchar** token, gchar** region)
{
    *secret_key = gfal2_get_opt_string(handle, group.c_str(), "SECRET_KEY", NULL);
    *access_key = gfal2_get_opt_string(handle, group.c_str(), "ACCESS_KEY", NULL);
    *token = gfal2_get_opt_string(handle, group.c_str(), "TOKEN", NULL);
    *region = gfal2_get_opt_string(handle, group.c_str(), "REGION", NULL);

    // For retrocompatibility
    if (!*secret_key) {
        *secret_key = gfal2_get_opt_string(handle, group.c_str(), "ACCESS_TOKEN_SECRET", NULL);
    }
    if (!*access_key) {
        *access_key = gfal2_get_opt_string(handle, group.c_str(), "ACCESS_TOKEN", NULL);
    }
}

static void gfal_http_get_aws(RequestParams & params, gfal2_context_t handle, const Davix::Uri& uri)
{
    // Try generic configuration
    gchar *secret_key, *access_key, *token, *region;
    gfal_http_get_aws_keys(handle, "S3", &secret_key, &access_key, &token, &region);

    // If not present, try S3:HOST
    if (!secret_key) {
        std::string group_label("S3:");
        group_label += uri.getHost();
        std::transform(group_label.begin(), group_label.end(), group_label.begin(), ::toupper);
        gfal_http_get_aws_keys(handle, group_label, &secret_key, &access_key, &token, &region);
    }

    // Last attempt, S3:host removing bucket
    if (!secret_key) {
        std::string group_label("S3:");
        std::string host = uri.getHost();
        size_t i = host.find('.');
        if (i != std::string::npos) {
            group_label += host.substr(i + 1);
            std::transform(group_label.begin(), group_label.end(), group_label.begin(), ::toupper);
            gfal_http_get_aws_keys(handle, group_label, &secret_key, &access_key, &token, &region);
        }
    }

    if (secret_key && access_key) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Setting S3 key pair");
        params.setAwsAuthorizationKeys(secret_key, access_key);
    }
    if (token) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Using short-lived access token");
        params.setAwsToken(token);
    }
    if (region) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Using region %s", region);
        params.setAwsRegion(region);
    }

    g_free(secret_key);
    g_free(access_key);
    g_free(token);
    g_free(region);
}

void GfalHttpPluginData::get_params(Davix::RequestParams* req_params, const Davix::Uri& uri)
{
    *req_params = reference_params;
    gboolean insecure_mode = gfal2_get_opt_boolean_with_default(handle, "HTTP PLUGIN", "INSECURE", FALSE);
    if (insecure_mode) {
        req_params->setSSLCAcheck(false);
    }
    gfal_http_get_ucert(*req_params, handle);
    gfal_http_get_aws(*req_params, handle, uri);

    if (uri.getProtocol().compare(0, 4, "http") == 0 || uri.getProtocol().compare(0, 3, "dav") == 0) {
        req_params->setProtocol(Davix::RequestProtocol::Auto);
    }
    else if (uri.getProtocol().compare(0, 2, "s3") == 0) {
        req_params->setProtocol(Davix::RequestProtocol::AwsS3);
    }
    else {
        req_params->setProtocol(Davix::RequestProtocol::Auto);
    }

    // Keep alive
    gboolean keep_alive = gfal2_get_opt_boolean_with_default(handle, "HTTP PLUGIN", "KEEP_ALIVE", TRUE);
    req_params->setKeepAlive(keep_alive);

    // Reset here the verbosity level
    davix_set_log_level(get_corresponding_davix_log_level());

    // Avoid retries
    req_params->setOperationRetry(0);

    // User agent
    const char *agent, *version;
    gfal2_get_user_agent(handle, &agent, &version);

    std::ostringstream user_agent;
    if (agent) {
        user_agent << agent << "/" << version << " " << "gfal2/" << gfal2_version();
    }
    else {
        user_agent << "gfal2/" << gfal2_version();
    }
    req_params->setUserAgent(user_agent.str());

    // Client information
    char* client_info = gfal2_get_client_info_string(handle);
    if (client_info) {
        req_params->addHeader("ClientInfo", client_info);
    }
    g_free(client_info);
}


static void log_davix2gfal(void* userdata, int msg_level, const char* msg)
{
    GLogLevelFlags gfal_level = G_LOG_LEVEL_MESSAGE;
    switch (msg_level) {
        case DAVIX_LOG_TRACE:
        case DAVIX_LOG_DEBUG:
            gfal_level = G_LOG_LEVEL_DEBUG;
            break;
        default:
            gfal_level = G_LOG_LEVEL_INFO;
    }
    gfal2_log(gfal_level, "Davix: %s", msg);
}


GfalHttpPluginData::GfalHttpPluginData(gfal2_context_t handle):
    context(), posix(&context), reference_params(), handle(handle)
{
    davix_set_log_handler(log_davix2gfal, NULL);
    int davix_level = get_corresponding_davix_log_level();

    int davix_config_level = gfal2_get_opt_integer_with_default(handle, "HTTP PLUGIN", "LOG_LEVEL", 0);
    if (davix_config_level)
        davix_level = davix_config_level;
    davix_set_log_level(davix_level);

    reference_params.setTransparentRedirectionSupport(true);
    reference_params.setUserAgent("gfal2::http");
    context.loadModule("grid");
}


GfalHttpPluginData* gfal_http_get_plugin_context(gpointer ptr)
{
    return static_cast<GfalHttpPluginData*>(ptr);
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

        case StatusCode::OperationNonSupported:
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

        case StatusCode::IsADirectory:
            errcode = EISDIR;
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
            if (http >= 400 && http < 500) {
                return EINVAL;
            } else {
#ifdef ECOMM
                return ECOMM;
#else
                return EIO;
#endif
            }
    }
}


void http2gliberr(GError** err, int http, const char* func, const char* msg)
{
    int errn = http2errno(http);
    char buffer[512] = {0};
    strerror_r(errn, buffer, sizeof(buffer));
    gfal2_set_error(err, http_plugin_domain, errn, func, "%s: %s (HTTP %d)", msg, buffer, http);
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
