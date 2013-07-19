/*
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <logger/gfal_logger.h>
#include <pugixml.hpp>
#include <string>
#include "gfal_mds_internal.h"


const char* bdii_cache_file = "CACHE_FILE";

static mds_type_endpoint gfal_mds_cache_type(const std::string& type,
                                const std::string &version)
{
    if (strcasecmp(type.c_str(), "srm") == 0) {
        if (version.compare(0, 2, "1.") == 0)
            return SRMv1;
        else if (version.compare(0, 2, "2.") == 0)
            return SRMv2;
        else
            return UnknownEndpointType;
    }
    else if (strcasecmp(type.c_str(), "webdav") == 0) {
        return WebDav;
    }
    else {
        return UnknownEndpointType;
    }
}

static void gfal_mds_cache_insert(gfal_mds_endpoint* endpoints, size_t s_endpoints,
                                  size_t index, const pugi::xml_node& entry)
{
    if (index > s_endpoints)
        return;

    std::string endpoint = entry.child("endpoint").last_child().value();
    std::string type     = entry.child("type").last_child().value();
    std::string version  = entry.child("version").last_child().value();

    mds_type_endpoint typeEnum = gfal_mds_cache_type(type, version);

    if (!endpoint.empty() && typeEnum != UnknownEndpointType) {
        strncpy(endpoints[index].url, endpoint.c_str(), sizeof(endpoints[index].url));
        endpoints[index].type = typeEnum;
    }
}

int gfal_mds_cache_resolve_endpoint(gfal2_context_t handle, const char* host,
                                    gfal_mds_endpoint* endpoints, size_t s_endpoints,
                                    GError** err)
{
    // Cache configuration
    gchar *cache_file = gfal2_get_opt_string(handle, bdii_config_group,
            bdii_cache_file, NULL);
    if (!cache_file)
        return 0;

    gfal_log(GFAL_VERBOSE_DEBUG, "BDII CACHE_FILE set to %s", cache_file);

    // Open the file, but do not fail if it can not be open
    // (A cache may not be present!)
    pugi::xml_document cache;
    pugi::xml_parse_result loadResult = cache.load_file(cache_file);
    if (loadResult.status != pugi::status_ok) {
        gfal_log(GFAL_VERBOSE_VERBOSE, "Could not load BDII CACHE_FILE: %s",
                loadResult.description());
        return 0;
    }

    // Iterate. Get all endpoints and check if the contain the requested host
    size_t hostLen = strlen(host);

    pugi::xpath_node_set allEndpoints = cache.document_element().select_nodes("/entry/endpoint");
    pugi::xpath_node_set::const_iterator i;
    size_t endpointIndex = 0;
    for (i = allEndpoints.begin();
         i != allEndpoints.end() && endpointIndex < s_endpoints;
         ++i) {
        const char* endpoint = i->node().child_value();
        const char* hostname = strstr(endpoint, "://");
        if (hostname) hostname += 3;
        else hostname = endpoint;

        if (strncasecmp(hostname, host, hostLen) == 0) {
            std::string xpathQuery("/entry[endpoint='");
            xpathQuery += endpoint;
            xpathQuery += "']";

            pugi::xpath_node entry = cache.document_element().select_single_node(xpathQuery.c_str());
            gfal_mds_cache_insert(endpoints, s_endpoints, endpointIndex++, entry.node());
        }
    }

    // Done here
    return endpointIndex;
}
