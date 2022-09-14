/*
* Copyright @ CERN, 2015.
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

#include <regex.h>

#include <gfal_api.h>
#include "uri/gfal2_uri.h"

#include "gridftp_pasv_plugin.h"
#include "gridftp_filecopy.h"
#include "gridftp_plugin.h"


static const GQuark GFAL_GRIDFTP_PASV_STAGE_QUARK = g_quark_from_static_string("PASV");


static globus_ftp_client_plugin_t* gfal2_ftp_client_pasv_plugin_copy(
        globus_ftp_client_plugin_t* plugin_template, void* plugin_specific)
{
    globus_ftp_client_plugin_t* plugin = (globus_ftp_client_plugin_t*) globus_malloc(
            sizeof(globus_ftp_client_plugin_t));
    gfal2_ftp_client_pasv_plugin_init(plugin, reinterpret_cast<GridFTPSession*>(plugin_specific));
    return plugin;
}


static void gfal2_ftp_client_pasv_plugin_destroy(globus_ftp_client_plugin_t* plugin,
        void* plugin_specific)
{
    globus_ftp_client_plugin_destroy(plugin);
    globus_free(plugin);
}


static void gfal2_ftp_client_pasv_command(globus_ftp_client_plugin_t* plugin,
        void* plugin_specific, globus_ftp_client_handle_t* handle, const char* url,
        const char* command)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, ">> %s", command);
}


static void gfal2_ftp_client_pasv_fire_event(GridFTPSession* session,
        const char* hostname, const char* ip, unsigned port, bool is_ipv6)
{
    if (session->params) {
        plugin_trigger_event(session->params, GFAL_GRIDFTP_DOMAIN_GSIFTP,
                GFAL_EVENT_DESTINATION, GFAL_GRIDFTP_PASV_STAGE_QUARK,
                "%s:%s:%u", hostname, ip, port);
        GQuark ipevent = (is_ipv6) ? GFAL_EVENT_IPV6 : GFAL_EVENT_IPV4;
        plugin_trigger_event(session->params, GFAL_GRIDFTP_DOMAIN_GSIFTP,
                             GFAL_EVENT_DESTINATION, ipevent, "%s:%u", ip, port);
    }
}


// Entering Passive Mode (h1,h2,h3,h4,p1,p2).
// Parenthesis are not guaranteed!
static int parse_27(const char *resp, char *ip, size_t ip_size, unsigned *port, bool *is_ipv6)
{
    static const char *regex_str = "[12]27 [^[0-9]+\\(?([0-9]+),([0-9]+),([0-9]+),([0-9]+),([0-9]+),([0-9]+)\\)?";
    regex_t preg;

    // This response can not return never IPv6 values
    *is_ipv6 = false;

    assert(regcomp(&preg, regex_str, REG_EXTENDED | REG_ICASE) == 0);

    regmatch_t matches[7];
    int ret = regexec(&preg, resp, 7, matches, 0);
    regfree(&preg);

    if (ret == REG_NOMATCH) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Failed to apply regex to 227 response");
        return -1;
    }

    unsigned h1, h2, h3, h4, p1, p2;
    h1 = atoi(resp + matches[1].rm_so);
    h2 = atoi(resp + matches[2].rm_so);
    h3 = atoi(resp + matches[3].rm_so);
    h4 = atoi(resp + matches[4].rm_so);
    p1 = atoi(resp + matches[5].rm_so);
    p2 = atoi(resp + matches[6].rm_so);

    snprintf(ip, ip_size, "%u.%u.%u.%u", h1, h2, h3, h4);
    *port = (p1 * 256) + p2;

    return 0;
}


// Entering Long Passive Mode (long address, port).
// Parenthesis are not guaranteed!
static int parse_28(const char *, char *, size_t, unsigned *, bool*)
{
    gfal2_log(G_LOG_LEVEL_WARNING, "Long Passive Mode not supported!");
    return -1;
}

// Parse IPv6 replies
static int parse_29_ipv6(const char *msg, char *ip, size_t ip_size, unsigned *port, bool *is_ipv6)
{
    regex_t ipv6regex;
    int retregex = regcomp(&ipv6regex, "\\|([0-9]*)\\|([^|]*)\\|([0-9]+)\\|", REG_EXTENDED);
    g_assert(retregex == 0);

    regmatch_t matches[4];
    retregex = regexec(&ipv6regex, msg, 4, matches, 0);
    regfree(&ipv6regex);

    if (retregex == REG_NOMATCH) {
        return -1;
    }

    // Type
    if (matches[1].rm_eo != matches[1].rm_so) {
        int type = atol(msg + matches[1].rm_so);
        if (type == 2) {
            *is_ipv6 = true;
        }
    }
    // Ip
    if (matches[2].rm_eo != matches[2].rm_so) {
        size_t len = matches[2].rm_eo - matches[2].rm_so;
        if (len > ip_size) {
            len = ip_size;
        }
        if (*is_ipv6) {
            char *buffer = g_strndup(msg + matches[2].rm_so, len);
            snprintf(ip, ip_size, "[%s]", buffer);
            g_free(buffer);
        }
        else {
            g_strlcpy(ip, msg + matches[2].rm_so, len);
        }
    }
    // Port
    *port = atoi(msg + matches[3].rm_so);

    return 0;
}

// Parse IPv4 replies
static int parse_29_ipv4(const char *msg, char *ip, size_t ip_size, unsigned *port, bool *is_ipv6)
{
    regex_t ipv4regex;
    int retregex = regcomp(&ipv4regex, "([0-9]+),([0-9]+),([0-9]+),([0-9]+),([0-9]+),([0-9]+)", REG_EXTENDED);
    g_assert(retregex == 0);

    regmatch_t matches[6];
    retregex = regexec(&ipv4regex, msg, 6, matches, 0);
    regfree(&ipv4regex);

    if (retregex == REG_NOMATCH) {
        return -1;
    }

    *is_ipv6 = false;

    unsigned h1, h2, h3, h4, p1, p2;
    h1 = atoi(msg + matches[0].rm_so);
    h2 = atoi(msg + matches[1].rm_so);
    h3 = atoi(msg + matches[2].rm_so);
    h4 = atoi(msg + matches[3].rm_so);
    p1 = atoi(msg + matches[4].rm_so);
    p2 = atoi(msg + matches[5].rm_so);

    snprintf(ip, ip_size, "%u.%u.%u.%u", h1, h2, h3, h4);
    *port = (p1 * 256) + p2;
    return 0;
}

// Entering Extended Passive Mode (|protocol|ip|port|).
// Parenthesis are standardized for EPSV, but they are not for SPAS
static int parse_29(const char *msg, char *ip, size_t ip_size, unsigned *port, bool *is_ipv6)
{
    *is_ipv6 = false;
    if (parse_29_ipv6(msg, ip, ip_size, port, is_ipv6) == 0) {
        return 0;
    }
    else if (parse_29_ipv4(msg, ip, ip_size, port, is_ipv6) == 0) {
        return 0;
    }
    gfal2_log(G_LOG_LEVEL_WARNING, "The passive mode response could not be parsed: %s", msg);
    return -1;
}


// Handle PASV responses
static void gfal2_ftp_client_pasv_response(globus_ftp_client_plugin_t* plugin,
        void* plugin_specific, globus_ftp_client_handle_t* handle, const char* url,
        globus_object_t* error, const globus_ftp_control_response_t* ftp_response)
{
    GridFTPSession* session = reinterpret_cast<GridFTPSession*>(plugin_specific);

    const char *p = reinterpret_cast<const char*>(ftp_response->response_buffer);
    gfal2_log(G_LOG_LEVEL_DEBUG, ">> %s", p);

    char ip[65] = {0};
    unsigned port = 0;
    bool got_pasv_ip = false, is_ipv6 = false;

    switch (ftp_response->response_class) {
        case GLOBUS_FTP_POSITIVE_PRELIMINARY_REPLY:
        case GLOBUS_FTP_POSITIVE_COMPLETION_REPLY:
            switch (ftp_response->code % 100) {
                case 27:
                    got_pasv_ip = (parse_27(p, ip, sizeof(ip), &port, &is_ipv6) == 0);
                    break;
                case 28:
                    got_pasv_ip = (parse_28(p, ip, sizeof(ip), &port, &is_ipv6) == 0);
                    break;
                case 29:
                    got_pasv_ip = (parse_29(p, ip, sizeof(ip), &port, &is_ipv6) == 0);
                    break;
            }
            break;
        default:
            break;
    }

    if (got_pasv_ip) {
        GError* err = NULL;
        gfal2_uri *parsed = gfal2_parse_uri(url, &err);
        if (parsed == NULL) {
            gfal2_log(G_LOG_LEVEL_WARNING, "Could not parse the URL: %s (%s)", url, err->message);
            g_error_free(err);
        }
        else {
            // Not specified in the response, so figure it out
            if (ip[0] == '\0') {
                bool ipv6_enabled = gfal2_get_opt_boolean_with_default(session->context, GRIDFTP_CONFIG_GROUP,
                    GRIDFTP_CONFIG_IPV6, FALSE);

                g_strlcpy(ip, lookup_host(parsed->host, ipv6_enabled, &is_ipv6).c_str(), sizeof(ip));
            }
            gfal2_ftp_client_pasv_fire_event(session, parsed->host, ip, port, is_ipv6);
            gfal2_free_uri(parsed);
        }
    }
}


static void gfal2_ftp_client_pasv_transfer(globus_ftp_client_plugin_t* plugin,
        void* plugin_specific, globus_ftp_client_handle_t* handle,
        const char* source_url, const globus_ftp_client_operationattr_t* source_attr,
        const char* dest_url, const globus_ftp_client_operationattr_t* dest_attr,
        globus_bool_t restart)
{
    // NOOP
}


globus_result_t gfal2_ftp_client_pasv_plugin_init(globus_ftp_client_plugin_t* plugin,
        GridFTPSession* session)
{
    globus_result_t result = GLOBUS_SUCCESS;

    result = globus_ftp_client_plugin_init(plugin, "gfal2_ftp_client_pasv_plugin",
            GLOBUS_FTP_CLIENT_CMD_MASK_ALL, session);
    if (result != GLOBUS_SUCCESS) {
        goto failure;
    }

    result = globus_ftp_client_plugin_set_copy_func(plugin, gfal2_ftp_client_pasv_plugin_copy);
    if (result != GLOBUS_SUCCESS) {
        goto failure;
    }

    result = globus_ftp_client_plugin_set_destroy_func(plugin, gfal2_ftp_client_pasv_plugin_destroy);
    if (result != GLOBUS_SUCCESS) {
        goto failure;
    }

    result = globus_ftp_client_plugin_set_command_func(plugin, gfal2_ftp_client_pasv_command);
    if (result != GLOBUS_SUCCESS) {
        goto failure;
    }

    result = globus_ftp_client_plugin_set_response_func(plugin, gfal2_ftp_client_pasv_response);
    if (result != GLOBUS_SUCCESS) {
        goto failure;
    }

    // If we do not register these two, the others will not be called for the operations we want
    result = globus_ftp_client_plugin_set_third_party_transfer_func(plugin, gfal2_ftp_client_pasv_transfer);
    if (result != GLOBUS_SUCCESS) {
        goto failure;
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, "gfal2_ftp_client_pasv_plugin registered");
failure:
    return result;
}
