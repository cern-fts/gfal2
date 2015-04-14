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
#include "uri/gfal_uri.h"

#include "gridftp_pasv_plugin.h"
#include "gridftp_filecopy.h"
#include "gridftpwrapper.h"


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
}


static void gfal2_ftp_client_pasv_command(globus_ftp_client_plugin_t* plugin,
        void* plugin_specific, globus_ftp_client_handle_t* handle, const char* url,
        const char* command)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, ">> %s", command);
}


static void gfal2_ftp_client_pasv_fire_event(GridFTPSession* session,
        const char* hostname, const char* ip, unsigned port)
{
    if (session->params) {
        plugin_trigger_event(session->params, GFAL_GRIDFTP_DOMAIN_GSIFTP,
                GFAL_EVENT_DESTINATION, GFAL_GRIDFTP_PASV_STAGE_QUARK,
                "%s:[%s]:%u", hostname, ip, port);
    }
}


static void gfal2_ftp_client_pasv_response(globus_ftp_client_plugin_t* plugin,
        void* plugin_specific, globus_ftp_client_handle_t* handle, const char* url,
        globus_object_t* error, const globus_ftp_control_response_t* ftp_response)
{
    GridFTPSession* session = reinterpret_cast<GridFTPSession*>(plugin_specific);

    const char *p = reinterpret_cast<const char*>(ftp_response->response_buffer);
    gfal2_log(G_LOG_LEVEL_DEBUG, ">> %s", p);

    char ip[65] = {0};
    unsigned port = 0;
    bool got_pasv_ip = false;

    switch (ftp_response->response_class) {
        case GLOBUS_FTP_POSITIVE_PRELIMINARY_REPLY:
        case GLOBUS_FTP_POSITIVE_COMPLETION_REPLY:
            switch (ftp_response->code % 100) {
                // Entering Passive Mode (h1,h2,h3,h4,p1,p2).
                // Parenthesis are not guaranteed!
                case 27:
                    while (*p && !isdigit(*p)) {
                        ++p;
                    }
                    if (*p) {
                        unsigned h1, h2, h3, h4, p1, p2;
                        sscanf(p, "(%u,%u,%u,%u,%u,%u)", &h1, &h2, &h3, &h4, &p1, &p2);
                        got_pasv_ip = true;
                    }
                    break;
                // Entering Long Passive Mode (long address, port).
                // Parenthesis are not guaranteed!
                case 28:
                    while (*p && !isdigit(*p)) {
                        ++p;
                    }
                    if (*p) {
                        sscanf(p, "(%64s, %u)", ip, &port);
                        got_pasv_ip = true;
                    }
                    break;
                // Entering Extended Passive Mode (|protocol|ip|port|).
                // Parenthesis are standarized
                case 29:
                    p = strchr(p, '(');
                    if (p) {
                        regex_t regex;
                        int retregex = regcomp(&regex, "\\(\\|([0-9]*)\\|([^|]*)\\|([0-9]+)\\|\\)", REG_EXTENDED);
                        assert(retregex == 0);
                        regmatch_t matches[4];
                        retregex = regexec(&regex, p, 4, matches, 0);
                        if (retregex == REG_NOMATCH) {
                            gfal2_log(G_LOG_LEVEL_WARNING, "The passive mode response could not be parsed: %s", p);
                        }
                        else {
                            // Ip
                            size_t len = matches[2].rm_eo - matches[2].rm_so;
                            if (len > sizeof(ip))
                                len = sizeof(ip);
                            g_strlcpy(ip, p + matches[2].rm_so, len);
                            // Port
                            port = atoi(p + matches[3].rm_so);

                            got_pasv_ip = true;
                        }
                    }
                    break;
            }
            break;
        default:
            break;
    }

    if (got_pasv_ip) {
        char hostname[512];
        GError* err = NULL;
        if (gfal2_hostname_from_uri(url, hostname, sizeof(hostname), &err) != 0) {
            gfal2_log(G_LOG_LEVEL_WARNING, "Could not parse the URL: %s (%s)", url, err->message);
            g_error_free(err);
        }
        else {
            if (ip[0] == '\0') {
                g_strlcpy(ip, lookup_host(hostname, TRUE).c_str(), sizeof(ip));
            }
            gfal2_ftp_client_pasv_fire_event(session, hostname, ip, port);
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
