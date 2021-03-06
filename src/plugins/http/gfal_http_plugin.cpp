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
#include "uri/gfal2_parsing.h"
#include <cstdio>
#include <cstring>
#include <sstream>
#include <list>
#include <davix.hpp>
#include <errno.h>
#include <davix/utils/davix_gcloud_utils.hpp>

using namespace Davix;

static const char* http_module_name = "http_plugin";
GQuark http_plugin_domain = g_quark_from_static_string(http_module_name);


const char* gfal_http_get_name(void)
{
    return GFAL2_PLUGIN_VERSIONED("http", VERSION);;
}

// this is to understand if the active storage in TPC needs gridsite delegation
// if the destination does not need tls we avoid it
static bool delegation_required(const Davix::Uri& uri)
{
   bool needs_delegation = false;

   if ((uri.getProtocol().compare(0, 5, "https") == 0) ||
      (uri.getProtocol().compare(0, 4, "davs") == 0)) {
          needs_delegation = true;
   }
   return needs_delegation;
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

static bool isS3SignedURL(const Davix::Uri &url)
{
    if(url.queryParamExists("AWSAccessKeyId") && url.queryParamExists("Signature")) {
    	return true;
    }

    if(url.queryParamExists("X-Amz-Credential") && url.queryParamExists("X-Amz-Signature")) {
    	return true;
    }

    return false;
}

/// Token-based authorization.
//  If this returns `true`, the RequestParams have been successfully
//  configured to utilize a bearer token.  In such a case, no other
//  authorization mechanism (such as user certs) should be used.
static bool gfal_http_get_token(RequestParams & params,
                                gfal2_context_t handle,
                                const Davix::Uri &url,
                                bool secondary_endpoint)
{

    if (isS3SignedURL(url)) {
	return false;
    }

    GError *error = NULL;
    gchar *token = gfal2_cred_get(handle, GFAL_CRED_BEARER,
                                  url.getString().c_str(),
                                  NULL, &error);
    g_clear_error(&error);  // for now, ignore the error messages.

    if (!token) {
        // if we don't have specific token for requested URL fallback
        // to token stored for hostname (for TPC with macaroon tokens
        // we need at least BEARER set for full source URL and hostname
        // BEARER for destination because that one could be also used
        // to create all missing parent directories)
        token = gfal2_cred_get(handle, GFAL_CRED_BEARER,
                               url.getHost().c_str(),
                               NULL, &error);
        g_clear_error(&error);  // for now, ignore the error messages.
    }

    if (!token) {
        return false;
    }

    std::stringstream ss;
    ss << "Bearer " << token;

    gfal2_log(G_LOG_LEVEL_DEBUG, "Using bearer token for HTTPS request authorization%s",
              secondary_endpoint ? " (passive TPC)" : "");

    if (secondary_endpoint) {
        params.addHeader("TransferHeaderAuthorization", ss.str());
        // If we have a valid token for the destination, we explicitly disable credential
        // delegation.
        params.addHeader("Credential", "none");
    } else {
        params.addHeader("Authorization", ss.str());
    }
    g_free(token);
    return true;
}

/// Authn implementation
static void gfal_http_get_ucert(const Davix::Uri &url, RequestParams & params, gfal2_context_t handle)
{
    std::string ukey, ucert;
    DavixError* tmp_err = NULL;
    GError *error = NULL;

    // Try user defined first
    std::string url_string = url.getString();

    gchar *ucert_p = gfal2_cred_get(handle, GFAL_CRED_X509_CERT, url_string.c_str(), NULL, &error);
    g_clear_error(&error);
    gchar *ukey_p = gfal2_cred_get(handle, GFAL_CRED_X509_KEY, url_string.c_str(), NULL, &error);
    g_clear_error(&error);

    if (ucert_p) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Using client X509 for HTTPS session authorization");
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

/// AWS authorization
static void gfal_http_get_aws_keys(gfal2_context_t handle, const std::string& group,
                                   gchar** secret_key, gchar** access_key,
                                   gchar** token, gchar** region)
{
    *secret_key    = (*secret_key) ? *secret_key : gfal2_get_opt_string(handle, group.c_str(), "SECRET_KEY", NULL);
    *access_key    = (*access_key) ? *access_key : gfal2_get_opt_string(handle, group.c_str(), "ACCESS_KEY", NULL);
    *token         = (*token)      ? *token      : gfal2_get_opt_string(handle, group.c_str(), "TOKEN", NULL);
    *region        = (*region)     ? *region     : gfal2_get_opt_string(handle, group.c_str(), "REGION", NULL);

    // For retrocompatibility
    if (!*access_key || !*secret_key) {
        *access_key = gfal2_get_opt_string(handle, group.c_str(), "ACCESS_TOKEN", NULL);
        *secret_key = gfal2_get_opt_string(handle, group.c_str(), "ACCESS_TOKEN_SECRET", NULL);
    }
}

/// AWS authorization + parameters
static void gfal_http_get_aws(RequestParams& params, gfal2_context_t handle, const Davix::Uri& uri)
{
    bool alternate_url;
    gchar *access_key = NULL, *secret_key = NULL, *token = NULL, *region = NULL;
    bool access_pair_set = false, token_set = false, region_set = false;
    bool alternate_set = false;

    std::list<std::string> group_labels;
    std::string host = uri.getHost();

    // Add S3:HOST group label
    std::string group_label = std::string("S3:") + host;
    std::transform(group_label.begin(), group_label.end(), group_label.begin(), ::toupper);
    group_labels.push_back(group_label);

    // Add S3:(no-bucket)HOST group label
    size_t pos = host.find(".");
    if (pos != std::string::npos) {
        group_label = std::string("S3:") + host.substr(pos + 1);
        std::transform(group_label.begin(), group_label.end(), group_label.begin(), ::toupper);
        group_labels.push_back(group_label);
    }

    // ADD S3 group label
    group_labels.push_back("S3");

    // Extract data from the config options
    // Order: Most specific group --> most generic group
    // Mechanism: Once a property is set, it will not be overwritten by later groups
    std::list<std::string>::const_iterator it;
    for (it = group_labels.begin(); it != group_labels.end(); it++) {
        gfal_http_get_aws_keys(handle, *it, &secret_key, &access_key, &token, &region);

        if (!access_pair_set && secret_key && access_key) {
            gfal2_log(G_LOG_LEVEL_DEBUG, "Setting S3 key pair [%s]", it->c_str());
            params.setAwsAuthorizationKeys(secret_key, access_key);
            access_pair_set = true;
        }

        if (!token_set && token) {
            gfal2_log(G_LOG_LEVEL_DEBUG, "Using short-lived access token [%s]", it->c_str());
            params.setAwsToken(token);
            token_set = true;
        }

        if (!region_set && region) {
            gfal2_log(G_LOG_LEVEL_DEBUG, "Using region %s [%s]", region, it->c_str());
            params.setAwsRegion(region);
            region_set = true;
        }

        if (!alternate_set) {
            GError *alternate_err = NULL;
            alternate_url = gfal2_get_opt_boolean(handle, it->c_str(), "ALTERNATE", &alternate_err);

            if (!alternate_err) {
                gfal2_log(G_LOG_LEVEL_DEBUG, "Setting S3 alternate URL to: %s [%s]",
                          (alternate_url) ? "true" : "false", it->c_str());
                params.setAwsAlternate(alternate_url);
                alternate_set = true;
            }

            g_clear_error(&alternate_err);
        }
    }

    g_free(access_key);
    g_free(secret_key);
    g_free(token);
    g_free(region);
}

static void gfal_http_get_gcloud(RequestParams & params, gfal2_context_t handle, const Davix::Uri& uri)
{
    gchar *gcloud_json_file, *gcloud_json_string;
    std::string group_label("GCLOUD");

    gcloud_json_file = gfal2_get_opt_string(handle, group_label.c_str(), "JSON_AUTH_FILE", NULL);
    gcloud_json_string = gfal2_get_opt_string(handle, group_label.c_str(), "JSON_AUTH_STRING", NULL);
    gcloud::CredentialProvider provider;
    if (gcloud_json_file) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Using gcloud json credential file");
        params.setGcloudCredentials(provider.fromFile(std::string(gcloud_json_file)));
    } else if (gcloud_json_string) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Using gcloud json credential string");
        params.setGcloudCredentials(provider.fromJSONString (std::string(gcloud_json_string)));
    }

    g_free(gcloud_json_file);
    g_free(gcloud_json_string);
}

static void gfal_http_get_cred(RequestParams & params,
                               gfal2_context_t handle,
                               const Davix::Uri& uri,
                               bool secondary_endpoint = false)
{
    // We still setup GSI in case the storage endpoint tries to fall back to GridSite delegation.
    // That does mean that we might contact the endpoint with both X509 and token auth -- but seems
    // to be an acceptable compromise.
    gfal_http_get_ucert(uri, params, handle);

    // Explicit request for S3 or GCloud
    if (uri.getProtocol().compare(0, 2, "s3") == 0) {
        gfal_http_get_aws(params, handle, uri);
    } else if (uri.getProtocol().compare(0, 6, "gcloud") == 0) {
        gfal_http_get_gcloud(params, handle, uri);
    } // Use bearer token
    else if (!gfal_http_get_token(params, handle, uri, secondary_endpoint)) {
        // Utilize AWS or GCLOUD tokens if no bearer token is available (to be reviewed)
        gfal_http_get_aws(params, handle, uri);
        gfal_http_get_gcloud(params, handle, uri);
    }
}

static void gfal_http_get_params(RequestParams & params, gfal2_context_t handle, const Davix::Uri& uri)
{
    gboolean insecure_mode = gfal2_get_opt_boolean_with_default(handle, "HTTP PLUGIN", "INSECURE", FALSE);
    if (insecure_mode) {
        params.setSSLCAcheck(false);
    }

    if (uri.getProtocol().compare(0, 4, "http") == 0 )  {
        params.setProtocol(Davix::RequestProtocol::Http);
    }
    else if (uri.getProtocol().compare(0, 3, "dav") == 0) {
        params.setProtocol(Davix::RequestProtocol::Webdav);
    }
    else if (uri.getProtocol().compare(0, 2, "s3") == 0) {
        params.setProtocol(Davix::RequestProtocol::AwsS3);
    }
    else if (uri.getProtocol().compare(0, 6, "gcloud") == 0) {
        params.setProtocol(Davix::RequestProtocol::Gcloud);
    }
    else {
        params.setProtocol(Davix::RequestProtocol::Auto);
    }
    // Keep alive
    gboolean keep_alive = gfal2_get_opt_boolean_with_default(handle, "HTTP PLUGIN", "KEEP_ALIVE", TRUE);
    params.setKeepAlive(keep_alive);

    // Reset here the verbosity level
    davix_set_log_level(get_corresponding_davix_log_level());

    // Avoid retries
    params.setOperationRetry(0);

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
    params.setUserAgent(user_agent.str());

    // Client information
    char* client_info = gfal2_get_client_info_string(handle);
    if (client_info) {
        params.addHeader("ClientInfo", client_info);
    }
    g_free(client_info);

    // Custom headers
    gsize headers_length = 0;
    char **headers = gfal2_get_opt_string_list_with_default(handle, "HTTP PLUGIN", "HEADERS", &headers_length, NULL);
    if (headers) {
        for (char **hi = headers; *hi != NULL; ++hi) {
            char **kv = g_strsplit(*hi, ":", 2);
            g_strstrip(kv[0]);
            g_strstrip(kv[1]);
            params.addHeader(kv[0], kv[1]);
            g_strfreev(kv);
        }
        g_strfreev(headers);
    }

    // Timeout
    struct timespec opTimeout;
    opTimeout.tv_sec = gfal2_get_opt_integer_with_default(handle, "HTTP PLUGIN", HTTP_CONFIG_OP_TIMEOUT, 8000);
    params.setOperationTimeout(&opTimeout);
}

void GfalHttpPluginData::get_tpc_params(bool push_mode,
                                        Davix::RequestParams * req_params,
                                        const Davix::Uri& src_uri,
                                        const Davix::Uri& dst_uri)
{
    *req_params = reference_params;

    bool do_delegation = false;
    if (push_mode) {
        gfal_http_get_params(*req_params, handle, src_uri);
        gfal_http_get_cred(*req_params, handle, src_uri);
        gfal_http_get_cred(*req_params, handle, dst_uri, true);
        do_delegation = delegation_required(dst_uri);
    } else {  // Pull mode
        gfal_http_get_params(*req_params, handle, dst_uri);
        gfal_http_get_cred(*req_params, handle, src_uri, true);
        gfal_http_get_cred(*req_params, handle, dst_uri);
        do_delegation = delegation_required(src_uri);
    }
    // The TPC request should be explicit in terms of how the active endpoint should manage credentials,
    // as it can be ambiguous from the request (i.e., client X509 authenticated by Macaroon present or
    // Macaroon present at an endpoint that supports OIDC).
    // If a token is present for the inactive endpoint, then we set `Credential: none` earlier; hence,
    // if that header is missing, we explicitly chose `gridsite` here. We should first check if source/dest
    // needs delegation
    if (do_delegation) {
        const HeaderVec &headers = req_params->getHeaders();
        bool set_credential = false;
        for (HeaderVec::const_iterator iter = headers.begin();
             iter != headers.end();
             iter++)
        {
            if (!strcasecmp(iter->first.c_str(), "Credential")) {
                set_credential = true;
            }
        }
        if (!set_credential) {
            req_params->addHeader("Credential", "gridsite");
       }
    } else {
        req_params->addHeader("Credential", "none");
        req_params->addHeader("X-No-Delegate", "true");
    }

}

void GfalHttpPluginData::get_params(Davix::RequestParams* req_params,
                                    const Davix::Uri& uri)
{
    *req_params = reference_params;

    gfal_http_get_cred(*req_params, handle, uri);
    gfal_http_get_params(*req_params, handle, uri);
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

    gchar *escaped_msg = gfal2_utf8escape_string(msg, strlen(msg), "\n\r\t\\");
    gfal2_log(gfal_level, "Davix: %s", escaped_msg);
    g_free(escaped_msg);
}


GfalHttpPluginData::GfalHttpPluginData(gfal2_context_t handle):
    context(), posix(&context), handle(handle), reference_params()
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
    	case GFAL_PLUGIN_QOS_CHECK_CLASSES:
    	case GFAL_PLUGIN_CHECK_FILE_QOS:
    	case GFAL_PLUGIN_CHECK_QOS_AVAILABLE_TRANSITIONS:
    	case GFAL_PLUGIN_CHECK_TARGET_QOS:
    	case GFAL_PLUGIN_CHANGE_OBJECT_QOS:
    		return true;
        case GFAL_PLUGIN_ACCESS:
        case GFAL_PLUGIN_OPEN:
        case GFAL_PLUGIN_STAT:
        case GFAL_PLUGIN_MKDIR:
        case GFAL_PLUGIN_OPENDIR:
        case GFAL_PLUGIN_RMDIR:
        case GFAL_PLUGIN_UNLINK:
        case GFAL_PLUGIN_CHECKSUM:
        case GFAL_PLUGIN_RENAME:
            return (strncmp("http:", url, 5) == 0 || strncmp("https:", url, 6) == 0 ||
                 strncmp("dav:", url, 4) == 0 || strncmp("davs:", url, 5) == 0 ||
                 strncmp("s3:", url, 3) == 0 || strncmp("s3s:", url, 4) == 0 ||
                 strncmp("gcloud:", url, 7) == 0 || strncmp("gclouds:", url, 8) == 0 ||
                 strncmp("http+3rd:", url, 9) == 0 || strncmp("https+3rd:", url, 10) == 0 ||
                 strncmp("dav+3rd:", url, 8) == 0 || strncmp("davs+3rd:", url, 9) == 0);
      default:
        return false;
    }
}

gboolean gfal_should_fallback(int error_code)
{

	switch(error_code) {
	    case ECANCELED:
	    	return false;
	default:
	    return true;

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
        case StatusCode::DelegationError:
            errcode = EACCES;
            break;

        case StatusCode::FileNotFound:
            errcode = ENOENT;
            break;

        case StatusCode::FileExist:
            errcode = EEXIST;
            break;
        case StatusCode::Canceled:
            errcode = ECANCELED;
            break;
        default:
            errcode = EIO;
            break;
    }

    return errcode;
}


void davix2gliberr(const DavixError* daverr, GError** err)
{
    const char *str = daverr->getErrMsg().c_str();
    size_t str_len = daverr->getErrMsg().length();
    gchar *escaped_str = gfal2_utf8escape_string(str, str_len, NULL);

    gfal2_set_error(err, http_plugin_domain, davix2errno(daverr->getStatus()), __func__,
              "%s", escaped_str);

    g_free(escaped_str);
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
    http_plugin.renameG = &gfal_http_rename;
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

    // QoS
    http_plugin.check_qos_classes = &gfal_http_check_classes;
    http_plugin.check_file_qos = &gfal_http_check_file_qos;
    http_plugin.check_qos_available_transitions = &gfal_http_check_qos_available_transitions;
    http_plugin.check_target_qos = &gfal_http_check_target_qos;
    http_plugin.change_object_qos = &gfal_http_change_object_qos;

    return http_plugin;
}
