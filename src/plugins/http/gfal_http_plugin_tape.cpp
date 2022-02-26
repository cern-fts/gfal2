/*
 * Copyright (c) CERN 2022
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
#include <sstream>

#include "gfal_http_plugin.h"

using namespace Davix;

namespace tape_rest_api {
    // Discover the Tape REST API endpoint starting from one of the URLs of the request
    std::string discover_tape_endpoint(gfal2_context_t context, const char* url, const char* method, GError** err) {
        Davix::Uri uri(url);

        if (uri.getStatus() != StatusCode::OK) {
            gfal2_set_error(err, http_plugin_domain, EINVAL, __func__, "Invalid URL: %s", url);
            return "";
        }

        std::stringstream endpoint;
        endpoint << uri.getProtocol() << "://" << uri.getHost();

        if (uri.getPort()) {
            endpoint << ":" << uri.getPort();
        }

        gchar *tape_prefix = gfal2_get_opt_string(context, "HTTP PLUGIN",
                                                  "TAPE_REST_API_PREFIX", NULL);

        if (tape_prefix == NULL) {
            gfal2_set_error(err, http_plugin_domain, EINVAL, __func__,
                            "Tape REST API endpoint not configured");
            return "";
        }

        if (tape_prefix[0] != '/') {
            endpoint << "/";
        }

        endpoint << tape_prefix;

        if (endpoint.str().back() != '/') {
            endpoint << "/";
        }

        if (method[0] == '/') {
            endpoint.seekp(-1, std::ios_base::end);
        }

        endpoint << method;

        g_free(tape_prefix);
        return endpoint.str();
    }

    // Construct the request body for a release operation from the given set of URLs
    std::string release_body(int nbfiles, const char *const *urls) {
        std::stringstream body;
        body << "{\"paths\": [";

        for (int i = 0; i < nbfiles; i++) {
            if (i != 0) {
                body << ", ";
            }

            body << "\"" << Davix::Uri(urls[i]).getPath() << "\"";
        }

        body << "]}";
        return body.str();
    }
}

int gfal_http_release_file(plugin_handle plugin_data, const char* url,
                           const char* request_id, GError** err)
{
    GError* errors[1] = {NULL};
    const char* const urls[1] = {url};
    int ret = gfal_http_release_file_list(plugin_data, 1, urls, request_id, errors);

    if (errors[0] != NULL) {
        *err = errors[0];
    }

    return ret;
}

int gfal_http_release_file_list(plugin_handle plugin_data, int nbfiles, const char* const* urls,
                                const char* request_id, GError** err)
{
    if (nbfiles <= 0) {
        return -1;
    }

    GError* tmp_err = NULL;

    auto copyErrors = [&tmp_err](int n, GError** err) -> void {
        for (int i = 0; i < n; i++) {
            err[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
    };

    std::stringstream method;
    method << "/release/" << ((request_id && strlen(request_id) > 0) ? request_id : "gfal2-placeholder-id");

    // Find out Tape REST API endpoint
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    std::string tapeEndpoint = tape_rest_api::discover_tape_endpoint(davix->handle, urls[0],
                                                                     method.str().c_str(), &tmp_err);

    if (tmp_err != NULL) {
        copyErrors(nbfiles, err);
        return -1;
    }

    // Construct and send "POST /release" request
    Davix::DavixError* reqerr = NULL;
    Davix::Uri uri(tapeEndpoint);
    Davix::RequestParams params;

    PostRequest request(davix->context, uri, &reqerr);
    davix->get_params(&params, uri, GfalHttpPluginData::OP::TAPE);
    params.addHeader("Content-Type", "application/json");
    request.setParameters(params);
    request.setRequestBody(tape_rest_api::release_body(nbfiles, urls));

    if (request.executeRequest(&reqerr)) {
        gfal2_set_error(&tmp_err, http_plugin_domain, davix2errno(reqerr->getStatus()), __func__,
                        "[Tape REST API] Release call failed: %s", reqerr->getErrMsg().c_str());
        copyErrors(nbfiles, err);
        return -1;
    }

    if (request.getRequestCode() != 200) {
        gfal2_set_error(&tmp_err, http_plugin_domain, davix2errno(reqerr->getStatus()), __func__,
                        "[Tape REST API] Release call failed: %s", reqerr->getErrMsg().c_str());
        copyErrors(nbfiles, err);
        return -1;
    }

    return 0;
}
