/*
 * Copyright (c) CERN 2020
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

#include <gfal_plugins_api.h>
#include "gfal_xrootd_plugin_interface.h"
#include "gfal_xrootd_plugin_utils.h"

#include <set>
#include <json.h>
#include <uuid/uuid.h>
#include <XrdCl/XrdClFileSystem.hh>


int gfal_xrootd_archive_poll(plugin_handle plugin_data, const char* url, GError** err)
{
    GError *errors[1] = {NULL};
    const char* const urls[1] = {url};
    int ret = gfal_xrootd_archive_poll_list(plugin_data, 1, urls, errors);

    if (errors[0] != NULL) {
        *err = errors[0];
    }

    return ret;
}


int gfal_xrootd_archive_poll_list(plugin_handle plugin_data, int nbfiles, const char* const* urls,
                                  GError** errors)
{
    if (nbfiles <= 0) {
        return 1;
    }

    gfal2_context_t context = (gfal2_context_t) plugin_data;

    XrdCl::URL endpoint(prepare_url(context, urls[0]));
    endpoint.SetPath(std::string());
    XrdCl::FileSystem fs(endpoint);

    // Generate a dummy prepare id
    uuid_t uuid;
    char suuid[128];
    uuid_generate_random(uuid);
    uuid_unparse(uuid, suuid);

    std::string str_query_args(suuid);
    std::set<std::string> paths;

    for (int i = 0; i < nbfiles; i++) {
        XrdCl::URL url(prepare_url(context, urls[i]));
        std::string path = url.GetPath();
        collapse_slashes(path);
        paths.insert(path);
        str_query_args += '\n';
        str_query_args += path;
    }

    XrdCl::Buffer* resp = 0;
    XrdCl::Buffer query_args;
    query_args.FromString(str_query_args);

    gfal2_log(G_LOG_LEVEL_DEBUG, "Issuing query prepare: %s", query_args.ToString().c_str());
    XrdCl::XRootDStatus st = fs.Query(XrdCl::QueryCode::Prepare, query_args, resp);

    if (!st.IsOK()) {
        gfal2_log(G_LOG_LEVEL_WARNING, "Query prepare failed: %s", st.ToString().c_str());
        for (int i = 0; i < nbfiles; i++) {
            gfal2_set_error(&errors[i], xrootd_domain, xrootd_status_to_posix_errno(st, true),
                            __func__, "%s", st.ToString().c_str());
        }
        return -1;
    }

    std::string jsonresp = resp->ToString();
    delete resp;
    struct json_object* parsed_json = json_tokener_parse(jsonresp.c_str());

    if (!parsed_json) {
        for (int i = 0; i < nbfiles; i++) {
            gfal2_set_error(&errors[i], xrootd_domain, ENOMSG, __func__,
                            "Response from server is an invalid JSON: %s",
                            jsonresp.c_str());
        }
        return -1;
    }

    struct json_object* request_id;
    json_object_object_get_ex(parsed_json, "request_id", &request_id);
    std::string str_request_id = request_id ? json_object_get_string(request_id) : "";

    if (str_request_id.empty() || str_request_id != suuid) {
        for (int i = 0; i < nbfiles; i++) {
            gfal2_set_error(&errors[i], xrootd_domain, ENOMSG, __func__, "%s", "Request ID mismatch.");
        }
        return -1;
    }

    int ontape_count = 0;
    int error_count = 0;

    // Iterate over the file list
    struct json_object* responses = 0;
    json_object_object_get_ex(parsed_json, "responses", &responses);
    int size = responses ? json_object_array_length(responses) : 0;

    if (size != nbfiles) {
        for (int i = 0; i < nbfiles; i++) {
            gfal2_set_error(&errors[i], xrootd_domain, ENOMSG, __func__,
                            "Number of files in the request doest not match!");
        }
        return -1;
    }

    for (int i = 0 ; i < size; i++) {
        struct json_object* fileobj = json_object_array_get_idx(responses, i);

        if (!fileobj) {
            error_count++;
            gfal2_set_error(&errors[i], xrootd_domain, ENOMSG, __func__,
                            "Failed to parse responses JSON from server: %s",
                            jsonresp.c_str());
            continue;
        }

        // Retrieve "error_text" attribute
        // Note: if it exists, this text should be appended to all other error messages
        struct json_object* fileobj_error_text = 0;
        json_object_object_get_ex(fileobj, "error_text", &fileobj_error_text);
        std::string error_text;

        if (!fileobj_error_text) {
          error_count++;
          gfal2_set_error(&errors[i], xrootd_domain, ENOMSG, __func__, "Error attribute missing.");
          continue;
        } else {
          error_text = json_object_get_string(fileobj_error_text);
        }

        // Retrieve "path" attribute
        struct json_object* fileobj_path = 0;
        json_object_object_get_ex(fileobj, "path", &fileobj_path);
        std::string path = fileobj_path ? json_object_get_string(fileobj_path) : "";
        collapse_slashes(path);

        if (path.empty() || !paths.count(path)) {
            error_count++;
            gfal2_xrootd_poll_set_error(&errors[i], ENOMSG, __func__, error_text.c_str(),
                                        "Wrong path: %s", path.c_str());
            continue;
        }

        // Retrieve "path_exists" attribute
        struct json_object* fileobj_exists = 0;
        json_object_object_get_ex(fileobj, "path_exists", &fileobj_exists);
        bool path_exists = json_obj_to_bool(fileobj_exists);

        if (!path_exists) {
            error_count++;
            gfal2_xrootd_poll_set_error(&errors[i], ENOENT, __func__, error_text.c_str(),
                                        "File does not exist: %s", path.c_str());
            continue;
        }

        // Retrieve "ontape" attribute
        struct json_object* fileobj_ontape = 0;
        json_object_object_get_ex(fileobj, "on_tape", &fileobj_ontape);
        bool ontape = json_obj_to_bool(fileobj_ontape);

        if (ontape) {
            ontape_count++;
            continue;
        }

        if (!error_text.empty()) {
            error_count++;
            gfal2_set_error(&errors[i], xrootd_domain, ENOMSG, __func__, "%s", error_text.c_str());
            continue;
        }

        // In case of no errors but the file is not yet archived, set EAGAIN
        if (!ontape) {
            gfal2_set_error(&errors[i], xrootd_domain, EAGAIN, __func__,
                            "File %s is not yet archived", path.c_str());
        }
    }

    // Free the top JSON object
    json_object_put(parsed_json);

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
