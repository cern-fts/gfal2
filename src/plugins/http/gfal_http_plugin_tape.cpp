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

#include "uri/gfal2_parsing.h"
#include "gfal_http_plugin.h"

using namespace Davix;

// Wrapper function to provide string functionality on top of Gfal2 util functionality
static std::string collapse_slashes(const std::string& path)
{
    char* collapsed_ptr = gfal2_path_collapse_slashes(path.c_str());
    std::string collapsed(collapsed_ptr);
    g_free(collapsed_ptr);
    return collapsed;
}

namespace tape_rest_api {

    // Construct a request body that consists in a list if file paths from the given set of URLs
    std::string list_files_body(int nbfiles, const char *const *urls) {
        std::stringstream body;
        body << "{\"paths\": [";

        for (int i = 0; i < nbfiles; i++) {
            if (i != 0) {
                body << ", ";
            }

            body << "\"" << collapse_slashes(Davix::Uri(urls[i]).getPath()) << "\"";
        }

        body << "]}";
        return body.str();
    }

    // Construct targetedMetadata json
    std::string stage_request_body(int disk_lifetime, int nbfiles, const char *const *urls, const char *const *metadata) {
        std::stringstream body;
        body << "{\"files\": [";

        for (int i = 0; i < nbfiles; i++) {
            if (i != 0) {
                body << ", ";
            }

            body << "{\"path\": " << "\"" << collapse_slashes(Davix::Uri(urls[i]).getPath()) << "\"";
            if ((metadata[i] != NULL) && (metadata[i][0] != '\0')) {
                body << ", \"targetedMetadata\": " << metadata[i];
            }
            body << "}";
        }

        body << "]}";

        return body.str();
    }

    // Parse metadata and return 0 if a valid JSON is found. If metadata is not a valid JSON return -1
    int metadata_format_checker(int nbfiles, const char *const *metadata_list, GError** err) {
        struct json_object* json_metadata = 0;

        for (int i = 0; i < nbfiles; i++) {

            if ((metadata_list[i] != NULL) && (metadata_list[i][0] != '\0')) {
                json_metadata = json_tokener_parse(metadata_list[i]);

                if (!json_metadata) {
                    gfal2_set_error(err, http_plugin_domain, EINVAL, __func__, "Invalid metadata format: %s", metadata_list[i]);
                    return -1;
                }
            }
            //Free the JSON object
            json_object_put(json_metadata);
        }
        return 0;
    }

    void copyErrors(GError* tmp_err, int n, GError** errors) {
        for (int i = 0; i < n; i++) {
            errors[i] = g_error_copy(tmp_err);
        }
        // Frees tmp_err
        g_error_free(tmp_err);
    }

    // Run through the full response and identify the JSON item corresponding to our path
    // Very inefficient O(N^2) complexity but avoids complicated data structures (tape polling is time permissive)
    struct json_object* polling_get_item_by_path(struct json_object* response, int nbfiles, const std::string& surl) {
        for (int i = 0; i < nbfiles; i++) {
            auto item = json_object_array_get_idx(response, i);

            if (item != NULL) {
                struct json_object* item_path = 0;
                json_object_object_get_ex(item, "path", &item_path);
                std::string path = item_path ? json_object_get_string(item_path) : "";

                if (!path.empty()) {
                    std::string cpath = collapse_slashes(path);
                    std::string csurl = collapse_slashes(surl);

                    if (cpath == csurl) {
                        return item;
                    }
                }
            }
        }

        return NULL;
    }
}

int gfal_http_bring_online(plugin_handle plugin_data, const char* url, time_t pintime, time_t timeout, char* token,
                           size_t tsize, int async, GError** err)
{
    GError* errors[1] = {NULL};
    const char* const urls[1] = {url};
    const char* const metadata_list[1] = {0};
    int ret = gfal_http_bring_online_list_v2(plugin_data, 1, urls, metadata_list, pintime, timeout,token, tsize, async, err);

    if (errors[0] != NULL) {
        *err = errors[0];
    }

    return ret;
}

int gfal_http_bring_online_list(plugin_handle plugin_data, int nbfiles, const char* const* urls, time_t pintime, time_t timeout,
                                char* token, size_t tsize, int async, GError** errors)
{
    const char* metadata_list[nbfiles];

    for (int i = 0; i < nbfiles; ++i) {
        metadata_list[i] = {0};
    }

    int ret = gfal_http_bring_online_list_v2(plugin_data, nbfiles, urls, metadata_list, pintime, timeout, token, tsize, async, errors);

    return ret;
}

int gfal_http_bring_online_v2(plugin_handle plugin_data, const char* url, const char* metadata, time_t pintime, time_t timeout, char* token,
                            size_t tsize, int async, GError** err)
{
    GError* errors[1] = {NULL};
    const char* const urls[1] = {url};
    const char* const metadata_list[1] = {metadata};
    int ret = gfal_http_bring_online_list_v2(plugin_data, 1, urls, metadata_list, pintime, timeout,token, tsize, async, err);

    if (errors[0] != NULL) {
        *err = errors[0];
    }

    return ret;
}

int gfal_http_bring_online_list_v2(plugin_handle plugin_data, int nbfiles, const char* const* urls, const char* const* metadata, time_t pintime, time_t timeout,
                            char* token, size_t tsize, int async, GError** errors)
{
    if (nbfiles <= 0) {
        return -1;
    }

    GError* tmp_err = NULL;

    // Check if all the metadata is in a valid JSON format
    if (tape_rest_api::metadata_format_checker(nbfiles, metadata, &tmp_err)) {
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    // Find out Tape Rest API endpoint
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    std::string tapeEndpoint = gfal_http_discover_tape_endpoint(davix, urls[0], "/stage/", &tmp_err);

    if (tmp_err != NULL) {
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    // Construct and send "POST /stage/" request
    Davix::DavixError* reqerr = NULL;
    Davix::Uri uri(tapeEndpoint);
    Davix::RequestParams params;

    PostRequest request(davix->context, uri, &reqerr);
    davix->get_params(&params, uri, GfalHttpPluginData::OP::TAPE);
    params.addHeader("Content-Type", "application/json");
    request.setParameters(params);

    request.setRequestBody(tape_rest_api::stage_request_body(pintime, nbfiles, urls, metadata));

    if (request.executeRequest(&reqerr)) {
        gfal2_set_error(&tmp_err, http_plugin_domain, davix2errno(reqerr->getStatus()), __func__,
                        "[Tape REST API] Stage call failed: %s", reqerr->getErrMsg().c_str());
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        Davix::DavixError::clearError(&reqerr);
        return -1;
    }

    if (request.getRequestCode() != 201) {
        gfal2_set_error(&tmp_err, http_plugin_domain, EINVAL, __func__,
                        "[Tape REST API] Stage call failed: Expected 201 status code (received %d)", request.getRequestCode());
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        Davix::DavixError::clearError(&reqerr);
        return -1;
    }

    std::string content = std::string(request.getAnswerContent());

    if (content.empty()) {
        gfal2_set_error(&tmp_err, http_plugin_domain, ENOMSG, __func__,
                        "[Tape REST API] Response with no data");
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    struct json_object* json_response = json_tokener_parse(content.c_str());

    if (!json_response) {
        gfal2_set_error(&tmp_err, http_plugin_domain, ENOMSG, __func__,
                        "[Tape REST API] Malformed served response");
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    // Check if "requestId" attribute exists
    struct json_object* id = 0;
    bool foundId = json_object_object_get_ex(json_response, "requestId", &id);
    if (!foundId) {
        gfal2_set_error(&tmp_err, http_plugin_domain, ENOMSG, __func__,
                        "[Tape REST API] requestID attribute missing");
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    std::string reqid = json_object_get_string(id);

    // Copy request id to token buffer
    g_strlcpy(token, reqid.c_str(), tsize);

    // Free the top JSON object
    json_object_put(json_response);

    return 0;
}

int gfal_http_abort_files(plugin_handle plugin_data, int nbfiles, const char* const* urls, const char* token,
                          GError** errors)
{
    if (nbfiles <= 0) {
        return -1;
    }

    GError* tmp_err = NULL;
    if (!token || strlen(token) == 0) {
        gfal2_set_error(&tmp_err, http_plugin_domain, EINVAL, __func__,
                        "The request ID was not provided");
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    std::stringstream method;
    method << "/stage/" << token << "/cancel";

    // Find out Tape Rest API endpoint
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    std::string tapeEndpoint = gfal_http_discover_tape_endpoint(davix, urls[0], method.str().c_str(), &tmp_err);

    if (tmp_err != NULL) {
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    // Construct and send "POST /stage/<token>/cancel" request
    Davix::DavixError* reqerr = NULL;
    Davix::Uri uri(tapeEndpoint);
    Davix::RequestParams params;

    PostRequest request(davix->context, uri, &reqerr);
    davix->get_params(&params, uri, GfalHttpPluginData::OP::TAPE);
    params.addHeader("Content-Type", "application/json");
    request.setParameters(params);
    request.setRequestBody(tape_rest_api::list_files_body(nbfiles, urls));

    if (request.executeRequest(&reqerr)) {
        gfal2_set_error(&tmp_err, http_plugin_domain, davix2errno(reqerr->getStatus()), __func__,
                        "[Tape REST API] Cancel call failed: %s", reqerr->getErrMsg().c_str());
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        Davix::DavixError::clearError(&reqerr);
        return -1;
    }

    if (request.getRequestCode() != 200) {
        gfal2_set_error(&tmp_err, http_plugin_domain, EINVAL, __func__,
                        "[Tape REST API] Stage call failed: Expected 200 status code (received %d)",
                        request.getRequestCode());
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        Davix::DavixError::clearError(&reqerr);
        return -1;
    }

    return 0;
}


int gfal_http_bring_online_poll(plugin_handle plugin_data, const char* url, const char* token, GError** err)
{
    GError* errors[1] = {NULL};
    const char* const urls[1] = {url};
    int ret = gfal_http_bring_online_poll_list(plugin_data, 1, urls, token, err);

    if (errors[0] != NULL) {
        *err = errors[0];
    }

    return ret;
}

int gfal_http_bring_online_poll_list(plugin_handle plugin_data, int nbfiles, const char* const* urls,
                                     const char* token, GError** errors)
{
    if (nbfiles <= 0) {
        return -1;
    }

    GError* tmp_err = NULL;
    if (!token || strlen(token) == 0) {
        gfal2_set_error(&tmp_err, http_plugin_domain, EINVAL, __func__,
                        "The request ID was not provided");
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    std::stringstream method;
    method << "/stage/" << token;

    // Find out Tape Rest API endpoint
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    std::string tapeEndpoint = gfal_http_discover_tape_endpoint(davix, urls[0], method.str().c_str(), &tmp_err);

    if (tmp_err != NULL) {
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    // Construct and send "GET /stage/<token>" request
    Davix::DavixError* reqerr = NULL;
    Davix::Uri uri(tapeEndpoint);
    Davix::RequestParams params;

    GetRequest request(davix->context, uri, &reqerr);
    davix->get_params(&params, uri, GfalHttpPluginData::OP::TAPE);
    request.setParameters(params);

    if (request.executeRequest(&reqerr)) {
        gfal2_set_error(&tmp_err, http_plugin_domain, davix2errno(reqerr->getStatus()), __func__,
                        "[Tape REST API] Stage pooling call failed: %s", reqerr->getErrMsg().c_str());
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        Davix::DavixError::clearError(&reqerr);
        return -1;
    }

    if (request.getRequestCode() != 200) {
        gfal2_set_error(&tmp_err, http_plugin_domain, EINVAL, __func__,
                        "[Tape REST API] Stage call failed: Expected 200 status code (received %d)",
                        request.getRequestCode());
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        Davix::DavixError::clearError(&reqerr);
        return -1;
    }

    std::string content = std::string(request.getAnswerContent());

    if (content.empty()) {
        gfal2_set_error(&tmp_err, http_plugin_domain, ENOMSG, __func__,
                        "[Tape REST API] Response with no data");
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    struct json_object* json_response = json_tokener_parse(content.c_str());

    if (!json_response) {
        gfal2_set_error(&tmp_err, http_plugin_domain, ENOMSG, __func__,
                        "[Tape REST API] Malformed served response");
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    // Check if "id" attribute exists
    struct json_object* id = 0;
    bool foundId = json_object_object_get_ex(json_response, "id", &id);
    std::string reqid = foundId ? json_object_get_string(id) : "";

    // Check if "request_id" attribute matches
    if (reqid.empty()) {
        gfal2_set_error(&tmp_err, http_plugin_domain, ENOMSG, __func__,
                        "[Tape REST API] Request ID missing from polling response (expected id=%s)", token);
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    if (reqid != token) {
        gfal2_set_error(&tmp_err, http_plugin_domain, ENOMSG, __func__,
                        "[Tape REST API] Request ID mismatch. Expected id=%s but received id=%s", token, reqid.c_str());
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    // Check if "files" attribute exists
    struct json_object* files = 0;
    bool foundFiles = json_object_object_get_ex(json_response, "files", &files);

    if (!foundFiles) {
        gfal2_set_error(&tmp_err, http_plugin_domain, ENOMSG, __func__,
                        "[Tape REST API] Files attribute missing from server poll response");
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    // Iterate over the "files" list
    int online_count = 0;
    int error_count  = 0;

    for (int i = 0; i < nbfiles; ++i) {
        std::string path = Davix::Uri(urls[i]).getPath();
        struct json_object* file = tape_rest_api::polling_get_item_by_path(files, nbfiles, path);

        if (file == NULL) {
            error_count++;
            gfal2_set_error(&errors[i], http_plugin_domain, ENOMSG, __func__,
                            "[Tape REST API] Missing response item for path=%s", path.c_str());
            continue;
        }

        // Check if "error" attribute exists
        struct json_object* file_error_text = 0;
        bool foundError = json_object_object_get_ex(file, "error", &file_error_text);

        if (foundError) {
            error_count++;
            std::string error_text = json_object_get_string(file_error_text);
            gfal2_set_error(&errors[i], http_plugin_domain, ENOMSG, __func__,
                            "[Tape REST API] %s", error_text.c_str());
            continue;
        }

        // Retrieve "onDisk" attribute
        struct json_object* file_on_disk = 0;
        bool foundOnDisk = json_object_object_get_ex(file, "onDisk", &file_on_disk);

        if (foundOnDisk) {
            std::string disk = json_object_get_string(file_on_disk);
            std::transform(disk.begin(), disk.end(), disk.begin(), tolower);

            if (disk == "true") {
                online_count++;
                continue;
            } else {
                gfal2_set_error(&errors[i], http_plugin_domain, EAGAIN, __func__,
                                "[Tape REST API] File %s is not yet on disk", path.c_str());
                continue;
            }
        }

        // Retrieve "state" attribute
        struct json_object* file_state = 0;
        bool foundState = json_object_object_get_ex(file, "state", &file_state);

        if (!foundState) {
            error_count++;
            gfal2_set_error(&errors[i], http_plugin_domain, ENOMSG, __func__,
                            "[Tape REST API] State and onDisk attributes missing");
            continue;
        }

        std::string state = json_object_get_string(file_state);

        if (state == "COMPLETED") {
            online_count++;
        } else if (state == "STARTED" || state == "SUBMITTED") {
            gfal2_set_error(&errors[i], http_plugin_domain, EAGAIN, __func__,
                            "[Tape REST API] File %s is not yet on disk", path.c_str());
        } else if (state == "CANCELED") {
            gfal2_set_error(&errors[i], http_plugin_domain, ECANCELED, __func__,
                            "[Tape REST API] Staging operation cancelled. File=%s", path.c_str());
            error_count++;
        } else if (state == "FAILED") {
            gfal2_set_error(&errors[i], http_plugin_domain, ENOENT, __func__,
                            "[Tape REST API] Staging operation failed for file=%s", path.c_str());
            error_count++;
        } else {
            gfal2_set_error(&errors[i], http_plugin_domain, ENOENT, __func__,
                            "[Tape REST API] Unrecognized staging status. File=%s status=%s",
                            path.c_str(), state.c_str());
            error_count++;
        }
    }

    // Free the top JSON object
    json_object_put(json_response);

    // All files are on disk: return 1
    if (online_count == nbfiles) {
        return 1;
    }

    // All files encountered errors: return -1
    if (error_count == nbfiles) {
        return -1;
    }

    // Some files are on disk, others encountered errors
    if (online_count + error_count == nbfiles) {
        return 2;
    }

    // Staging still in process: return 0
    return 0;
}

int gfal_http_archive_poll(plugin_handle plugin_data, const char* url, GError** err)
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
                                GError** errors)
{
    if (nbfiles <= 0) {
        return -1;
    }

    GError* tmp_err = NULL;

    // Find out Tape Rest API endpoint
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    std::string tapeEndpoint = gfal_http_discover_tape_endpoint(davix, urls[0], "/archiveinfo/", &tmp_err);

    if (tmp_err != NULL) {
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
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
    request.setRequestBody(tape_rest_api::list_files_body(nbfiles, urls));

    if (request.executeRequest(&reqerr)) {
        gfal2_set_error(&tmp_err, http_plugin_domain, davix2errno(reqerr->getStatus()), __func__,
                        "[Tape REST API] Archive polling call failed: %s", reqerr->getErrMsg().c_str());
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        Davix::DavixError::clearError(&reqerr);
        return -1;
    }

    if (request.getRequestCode() != 200) {
        gfal2_set_error(&tmp_err, http_plugin_domain, EINVAL, __func__,
                        "[Tape REST API] Archive polling call failed: Expected 200 status code (received %d)",
                        request.getRequestCode());
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        Davix::DavixError::clearError(&reqerr);
        return -1;
    }

    std::string content = std::string(request.getAnswerContent());

    if (content.empty()) {
        gfal2_set_error(&tmp_err, http_plugin_domain, ENOMSG, __func__,
                        "[Tape REST API] Response with no data");
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    struct json_object* json_response = json_tokener_parse(content.c_str());

    if (!json_response) {
        gfal2_set_error(&tmp_err, http_plugin_domain, ENOMSG, __func__,
                        "[Tape REST API] Malformed server response");
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        return -1;
    }

    // Iterate over the file list
    int ontape_count = 0;
    int error_count = 0;

    for (int i = 0; i < nbfiles; ++i) {
        std::string path = Davix::Uri(urls[i]).getPath();
        struct json_object* file = tape_rest_api::polling_get_item_by_path(json_response, nbfiles, path);

        if (file == NULL) {
            error_count++;
            gfal2_set_error(&errors[i], http_plugin_domain, ENOMSG, __func__,
                            "[Tape REST API] Missing response item for path=%s", path.c_str());
            continue;
        }

        // Check if "error" attribute exists
        struct json_object* file_error_text = 0;
        bool foundError = json_object_object_get_ex(file, "error", &file_error_text);

        if (foundError) {
            error_count++;
            std::string error_text = json_object_get_string(file_error_text);
            gfal2_set_error(&errors[i], http_plugin_domain, ENOMSG, __func__, "[Tape REST API] %s", error_text.c_str());
            continue;
        }

        // Retrieve "locality" attribute
        struct json_object* file_locality = 0;
        bool localityExist = json_object_object_get_ex(file, "locality", &file_locality);

        if (!localityExist) {
            error_count++;
            gfal2_set_error(&errors[i], http_plugin_domain, ENOMSG, __func__,
                            "[Tape REST API] Locality attribute missing");
        }

        std::string locality = json_object_get_string(file_locality);

        if (locality == "TAPE" || locality == "DISK_AND_TAPE" ) {
            ontape_count++;
        } else if (locality == "LOST") {
            error_count++;
            gfal2_set_error(&errors[i], http_plugin_domain, ENOENT, __func__,
                            "[Tape REST API] File locality reported as LOST (path=%s)", path.c_str());
        } else if (locality == "NONE") {
            gfal2_set_error(&errors[i], http_plugin_domain, EPERM, __func__,
                            "[Tape REST API] File locality reported as NONE (path=%s)", path.c_str());
            error_count++;
        } else {
            gfal2_set_error(&errors[i], http_plugin_domain, EAGAIN, __func__,
                            "[Tape REST API] File %s is not yet archived", path.c_str());
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
                                const char* request_id, GError** errors)
{
    if (nbfiles <= 0) {
        return -1;
    }

    GError* tmp_err = NULL;
    std::stringstream method;
    method << "/release/" << ((request_id && strlen(request_id) > 0) ? request_id : "gfal2-placeholder-id");

    // Find out Tape REST API endpoint
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    std::string tapeEndpoint = gfal_http_discover_tape_endpoint(davix, urls[0], method.str().c_str(), &tmp_err);

    if (tmp_err != NULL) {
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
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
    request.setRequestBody(tape_rest_api::list_files_body(nbfiles, urls));

    if (request.executeRequest(&reqerr)) {
        gfal2_set_error(&tmp_err, http_plugin_domain, davix2errno(reqerr->getStatus()), __func__,
                        "[Tape REST API] Release call failed: %s", reqerr->getErrMsg().c_str());
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        Davix::DavixError::clearError(&reqerr);
        return -1;
    }

    if (request.getRequestCode() != 200) {
        gfal2_set_error(&tmp_err, http_plugin_domain, EINVAL, __func__,
                        "[Tape REST API] Release call failed: Expected 200 status code (received %d)",
                        request.getRequestCode());
        tape_rest_api::copyErrors(tmp_err, nbfiles, errors);
        Davix::DavixError::clearError(&reqerr);
        return -1;
    }

    return 0;
}
