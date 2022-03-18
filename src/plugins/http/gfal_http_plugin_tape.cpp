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
#include <json.h>

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

    // Construct the request body for a archive info request from the given set of URLs
    std::string archive_info_body(int nbfiles, const char *const *urls) {
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

int gfal_http_archive_poll(plugin_handle plugin_data, const char* url,
                            GError** err)
{
    GError* errors[1] = {NULL};
    const char* const urls[1] = {url};
    int ret = gfal_http_archive_poll_list(plugin_data, 1, urls, err);

    if (errors[0] != NULL) {
        *err = errors[0];
    }

    return ret;
}

int gfal_http_archive_poll_list(plugin_handle plugin_data, int nbfiles, const char* const* urls,
                                    GError** err)
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
    method << "/archiveinfo";

    // Find out Tape Rest API endpoint
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    std::string tapeEndpoint = tape_rest_api::discover_tape_endpoint(davix->handle, urls[0],
                                                                     method.str().c_str(), &tmp_err);

    if (tmp_err != NULL) {
        copyErrors(nbfiles, err);
        return -1;
    }

    // Construct and send "POST /archiveinfo" request
    Davix::DavixError* reqerr = NULL;
    Davix::Uri uri(tapeEndpoint);
    Davix::RequestParams params;

    PostRequest request(davix->context, uri, &reqerr);
    davix->get_params(&params, uri, GfalHttpPluginData::OP::TAPE);
    params.addHeader("Content-Type", "application/json");
    request.setParameters(params);
    request.setRequestBody(tape_rest_api::archive_info_body(nbfiles, urls));

    if (request.executeRequest(&reqerr)) {
        gfal2_set_error(&tmp_err, http_plugin_domain, davix2errno(reqerr->getStatus()), __func__,
                        "[Tape REST API] Archive polling call failed: %s", reqerr->getErrMsg().c_str());
        copyErrors(nbfiles, err);
        return -1;
    }

    if (request.getRequestCode() != 200) {
        gfal2_set_error(&tmp_err, http_plugin_domain, davix2errno(reqerr->getStatus()), __func__,
                        "[Tape REST API] Archive polling call failed: %s", reqerr->getErrMsg().c_str());
        copyErrors(nbfiles, err);
        return -1;
    }

    std::string content = std::string(request.getAnswerContent());

    if (content.empty()) {
        gfal2_set_error(&tmp_err, http_plugin_domain, ENOMSG, __func__,
                        "[Tape REST API] Response with no data.");
        copyErrors(nbfiles, err);
        return -1;
    }

    struct json_object* json_response = json_tokener_parse(content.c_str());

    if (!json_response) {
        gfal2_set_error(&tmp_err, http_plugin_domain, ENOMSG, __func__,
                        "[Tape REST API] Malformed served response.");
        copyErrors(nbfiles, err);
        return -1;
    }

    // Iterate over the file list
    const int len = json_object_array_length(json_response);
    int ontape_count = 0;
    int error_count = 0;

    if (len != nbfiles) {
        gfal2_set_error(&tmp_err, http_plugin_domain, ENOMSG, __func__,
                        "[Tape REST API] Number of files in the request doest not match!");
        copyErrors(nbfiles, err);
        return -1;
    }

    for (int i = 0; i < len; ++i) {
        json_object *file = json_object_array_get_idx(json_response, i);

        if (file == NULL) {
            error_count++;
            gfal2_set_error(&err[i], http_plugin_domain, ENOMSG, __func__, "[Tape REST API] Malformed server response.");
            continue;
        }

        // Check if "error" attribute exists
        struct json_object* file_error_text = 0;
        bool foundError = json_object_object_get_ex(file, "error", &file_error_text);
        if (foundError) {
            error_count++;
            std::string error_text = json_object_get_string(file_error_text);
            gfal2_set_error(&err[i], http_plugin_domain, ENOMSG, __func__, "[Tape REST API] %s", error_text.c_str());
            continue;
        }

        // Check if "path" attribute exists
        struct json_object* file_path = 0;
        bool foundPath = json_object_object_get_ex(file, "path", &file_path);
        if (!foundPath) {
            error_count++;
            gfal2_set_error(&err[i], http_plugin_domain, ENOMSG, __func__, "[Tape REST API] Path attribute missing");
            continue;
        }

        std::string path = json_object_get_string(file_path);

        // Retrieve "locality" attribute
        struct json_object* file_locality = 0;
        bool localityExist = json_object_object_get_ex(file, "locality", &file_locality);
        if (!localityExist) {
            error_count++;
            gfal2_set_error(&err[i], http_plugin_domain, ENOMSG, __func__,
                            "[Tape REST API] Locality attribute missing");
        }

        std::string locality = json_object_get_string(file_locality);
        if (locality == "TAPE" || locality == "DISK_AND_TAPE" ) {
            ontape_count++;
        } else if (locality == "DISK") {
            gfal2_set_error(&err[i], http_plugin_domain, EAGAIN, __func__,
                            "[Tape REST API] File %s is not yet archived", path.c_str());
        } else {
            gfal2_set_error(&err[i], http_plugin_domain, ENOENT, __func__,
                            "[Tape REST API] File %s is missing", path.c_str());
        }
    }

    // Free the top JSON object
    json_object_put(json_response);

    // All files are on tape: return 1
    if (ontape_count == nbfiles) {
        return 1;
    }

    // All files encountered errors: return -1
    if (error_count == nbfiles) {
        return -1;
    }

    // Some files are on tape, others encountered errors
    if (ontape_count + error_count == nbfiles) {
        return 2;
    }

    // Archiving in process: return 0
    return 0;
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
