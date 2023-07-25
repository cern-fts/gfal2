/*
 * Copyright (c) CERN 2017
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

#include <memory>
#include <set>
#include <algorithm>
#include <json.h>
#include <XrdCl/XrdClFileSystem.hh>
#include <XrdSys/XrdSysPthread.hh>


int gfal_xrootd_bring_online_list(plugin_handle plugin_data,
    int nbfiles, const char* const* urls, time_t pintime, time_t timeout, char* token, size_t tsize,
    int async, GError** err)
{
    if (nbfiles <= 0) {
        return 1;
    }

    gfal2_context_t context = (gfal2_context_t)plugin_data;

    XrdCl::URL endpoint(prepare_url(context, urls[0]));
    endpoint.SetPath(std::string());
    XrdCl::FileSystem fs(endpoint);

    std::vector<std::string> fileList;
    for (int i = 0; i < nbfiles; ++i) {
        XrdCl::URL file(prepare_url(context, urls[i]));
        fileList.emplace_back(file.GetPath());
    }

    XrdCl::Buffer *responsePtr = 0;
    XrdCl::Status st = fs.Prepare(fileList, XrdCl::PrepareFlags::Flags::Stage, 0, responsePtr, timeout);

    if (!st.IsOK()) {
        GError *tmp_err = NULL;
        gfal2_set_error(&tmp_err, xrootd_domain, xrootd_status_to_posix_errno(st), __func__,
                        "Bringonline request failed. One or more files failed with: %s", st.ToString().c_str());
        for (int i = 0; i < nbfiles; ++i) {
            err[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        delete responsePtr;
        return -1;
    }
    if (responsePtr && responsePtr->GetBuffer()) {
        copy_to_cstring(token, tsize, responsePtr->GetBuffer(), responsePtr->GetSize());
    } else {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Empty response from the server");
        delete responsePtr;
        return -1;
    }
    delete responsePtr;
    return 0;
}


int gfal_xrootd_bring_online_list_v2(plugin_handle plugin_data,
    int nbfiles, const char* const* urls, const char* const* metadata,
    time_t pintime, time_t timeout, char* token, size_t tsize, int async, GError** err)
{
    return gfal_xrootd_bring_online_list(plugin_data, nbfiles, urls, pintime, timeout, token, tsize, async, err);
}


int gfal_xrootd_bring_online_poll_list(plugin_handle plugin_data,
    int nbfiles, const char* const* urls, const char* token, GError** err)
{
    if (nbfiles <= 0) {
        return 1;
    }
    gfal2_context_t context = (gfal2_context_t)plugin_data;

    XrdCl::URL endpoint(prepare_url(context, urls[0]));
    endpoint.SetPath(std::string());
    XrdCl::FileSystem fs(endpoint);

    std::set<std::string> paths;
    std::string strarg = token;
    for( int i = 0; i < nbfiles; ++i )
    {
      strarg += '\n';
      XrdCl::URL url( prepare_url( context, urls[i] ) );
      std::string path = url.GetPath();
      // collapse redundant slashes
      collapse_slashes( path );
      strarg += path;
      paths.insert( path );
    }

    XrdCl::Buffer  arg; arg.FromString( strarg );
    XrdCl::Buffer *resp = 0;

    gfal2_log( G_LOG_LEVEL_DEBUG, "Issuing query prepare." );
    XrdCl::XRootDStatus st = fs.Query( XrdCl::QueryCode::Prepare, arg, resp );

    if( !st.IsOK() )
    {
      gfal2_log( G_LOG_LEVEL_WARNING, "Query prepare failed: %s", st.ToString().c_str() );
      for( int i = 0; i < nbfiles; ++i )
        gfal2_set_error(&err[i], xrootd_domain, xrootd_status_to_posix_errno(st, true),
                        __func__, "%s", st.ToString().c_str() );
      return -1;
    }

    std::string jsonresp = resp->ToString();
    delete resp;

    struct json_object *parsed_json = json_tokener_parse( jsonresp.c_str() );
    if( !parsed_json )
    {
      for( int i = 0; i < nbfiles; ++i )
        gfal2_set_error(&err[i], xrootd_domain, ENOMSG, __func__,
                        "Response from server is an invalid JSON: %s",
                        jsonresp.c_str());
      return -1;
    }

    struct json_object *request_id;
    json_object_object_get_ex( parsed_json, "request_id", &request_id );
    std::string reqid = request_id ? json_object_get_string( request_id ) : "";
    if( reqid.empty() || reqid != token )
    {
      for( int i = 0; i < nbfiles; ++i )
        gfal2_set_error( &err[i], xrootd_domain, ENOMSG, __func__, "%s", "Request ID mismatch." );
      return -1;
    }

    int onlinecnt = 0;
    int errorcnt  = 0;

    // now iterate over the file list
    struct json_object *responses = 0;
    json_object_object_get_ex( parsed_json, "responses", &responses );
    int size = responses ? json_object_array_length( responses ) : 0;
    if( size != nbfiles )
    {
      for( int i = 0; i < nbfiles; ++i )
        gfal2_set_error( &err[i], xrootd_domain, ENOMSG, __func__,
                         "Number of files in the request does not match!" );
      return -1;
    }

    for( int i = 0; i < size; ++i )
    {
      // get the i-th object in the array
      struct json_object *arrobj = json_object_array_get_idx( responses, i );
      if( !arrobj )
      {
        ++errorcnt;
        gfal2_set_error(&err[i], xrootd_domain, ENOMSG, __func__,
                        "Failed to parse responses JSON from server: %s",
                        jsonresp.c_str());
        continue;
      }

      // get the error_text attribute
      // Note: if it exists, this text should be appened to all other error messages
      struct json_object *arrobj_error_text = 0;
      json_object_object_get_ex( arrobj, "error_text", &arrobj_error_text );
      std::string error_text;

      if( !arrobj_error_text )
      {
        ++errorcnt;
        gfal2_set_error( &err[i], xrootd_domain, ENOMSG, __func__, "Error attribute missing." );
        continue;
      } else {
        error_text = json_object_get_string( arrobj_error_text );
      }

      // get the path attribute
      struct json_object *arrobj_path = 0;
      json_object_object_get_ex( arrobj, "path", &arrobj_path );
      std::string path = arrobj_path ? json_object_get_string( arrobj_path ) : "";
      // collapse redundant slashes
      collapse_slashes( path );
      if( path.empty() || !paths.count( path ) )
      { // it's not our file, this is an error
        ++errorcnt;
        gfal2_xrootd_poll_set_error( &err[i], ENOMSG, __func__, error_text.c_str(),
                                     "Wrong path: %s", path.c_str() );
        continue;
      }

      // get the path_exists attribute
      // Note: CTA changed from "exists" to "path_exists".
      //       Keep a fallback to "exists" for the time being.
      struct json_object *arrobj_exists = 0;
      json_object_object_get_ex( arrobj, "path_exists", &arrobj_exists );
      bool path_exists = json_obj_to_bool(arrobj_exists);
      if( !path_exists )
      {
        // Try "exists" fallback
        json_object_object_get_ex( arrobj, "exists", &arrobj_exists );
        bool exists = json_obj_to_bool(arrobj_exists);
        if ( !exists )
        {
          ++errorcnt;
          gfal2_xrootd_poll_set_error( &err[i], ENOENT, __func__, error_text.c_str(),
                                       "File does not exist: %s", path.c_str() );
          continue;
        }
      }

      // get the online attribute
      struct json_object *arrobj_online = 0;
      json_object_object_get_ex( arrobj, "online", &arrobj_online );
      bool online = json_obj_to_bool(arrobj_online);
      if( online )
      {
        ++onlinecnt;
        continue;
      }

      // get the requested attribute
      struct json_object *arrobj_requested = 0;
      json_object_object_get_ex( arrobj, "requested", &arrobj_requested );
      bool requested = json_obj_to_bool(arrobj_requested);
      if( !requested )
      {
        ++errorcnt;
        gfal2_xrootd_poll_set_error( &err[i], ENOMSG, __func__, error_text.c_str(),
                                    "File is not being brought online: %s", path.c_str() );
        continue;
      }

      // get the has_reqid attribute
      struct json_object *arrobj_has_reqid = 0;
      json_object_object_get_ex( arrobj, "has_reqid", &arrobj_has_reqid );
      bool has_reqid = json_obj_to_bool(arrobj_has_reqid);
      if( !has_reqid )
      {
        ++errorcnt;
        gfal2_xrootd_poll_set_error( &err[i], ENOMSG, __func__, error_text.c_str(),
                                     "File (%s) is not included in the bring online request: %s",
                                     path.c_str(), token );
        continue;
      }

      // get the req_time attribute
      struct json_object *arrobj_req_time = 0;
      json_object_object_get_ex( arrobj, "req_time", &arrobj_req_time );
      std::string req_time = arrobj_req_time ? json_object_get_string( arrobj_req_time ) : "";
      if( !req_time.empty() )
        gfal2_log( G_LOG_LEVEL_DEBUG, "File (%s) has been requested at: %s",
                   path.c_str(), req_time.c_str() );
      else
      {
        ++errorcnt;
        gfal2_xrootd_poll_set_error( &err[i], ENOMSG, __func__, error_text.c_str(),
                                     "Bring-online timestamp missing." );
        continue;
      }

      if( !error_text.empty() )
      {
        ++errorcnt;
        gfal2_set_error( &err[i], xrootd_domain, ENOMSG, __func__, "%s", error_text.c_str() );
        continue;
      }

      // if there is no error but the file is not online set EAGAIN
      if( !online )
        gfal2_set_error( &err[i], xrootd_domain, EAGAIN, __func__,
                         "File (%s) is not yet online.", path.c_str() );
    }

    // Free the top JSON object
    json_object_put(parsed_json);

    // if all files are online return 1
    if( onlinecnt == nbfiles ) return 1;

    // if there were errors return -1
    if( errorcnt == nbfiles ) return -1;

    // Some files are online, others encountered errors
    if( (errorcnt + onlinecnt) == nbfiles ) return 2;

    // otherwise 0 means user still needs to wait
    return 0;
}


int gfal_xrootd_release_file_list(plugin_handle plugin_data,
    int nbfiles, const char* const* urls, const char* token, GError** err)
{
    gfal2_context_t context = (gfal2_context_t)plugin_data;

    XrdCl::URL endpoint(prepare_url(context, urls[0]));
    endpoint.SetPath(std::string());
    XrdCl::FileSystem fs(endpoint);

    std::vector<std::string> fileList;
    for(int i = 0; i < nbfiles; ++i) {
      XrdCl::URL file(prepare_url(context, urls[i]));
      fileList.emplace_back(file.GetPath());
    }

    XrdCl::Buffer *responsePtr = 0;
    XrdCl::Status st = fs.Prepare(fileList, XrdCl::PrepareFlags::Flags::Evict, 0, responsePtr, 30);

    if (!st.IsOK()) {
        GError *tmp_err = NULL;
        gfal2_set_error(&tmp_err, xrootd_domain, xrootd_status_to_posix_errno(st),
                        __func__, "%s", st.ToString().c_str());
        for (int i = 0; i < nbfiles; ++i) {
            err[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        delete responsePtr;
        return -1;
    }
    delete responsePtr;
    return 0;
}


int gfal_xrootd_bring_online(plugin_handle plugin_data,
    const char* url, time_t pintime, time_t timeout, char* token, size_t tsize, int async, GError** err)
{
    GError *errors[1] = {NULL};
    const char* const urls[1] = {url};
    int ret = gfal_xrootd_bring_online_list(plugin_data, 1, urls, pintime, timeout, token, tsize, async, errors);
    if (errors[0] != NULL) {
        *err = errors[0];
    }
    return ret;
}


int gfal_xrootd_bring_online_v2(plugin_handle plugin_data,
    const char* url, const char* metadata, time_t pintime, time_t timeout, char* token, size_t tsize, int async, GError** err)
{
    return gfal_xrootd_bring_online(plugin_data, url, pintime, timeout, token, tsize, async, err);
}


int gfal_xrootd_bring_online_poll(plugin_handle plugin_data,
    const char* url, const char* token, GError** err)
{
    GError *errors[1] = {NULL};
    const char* const urls[1] = {url};
    int ret = gfal_xrootd_bring_online_poll_list(plugin_data, 1, urls, token, errors);
    if (errors[0] != NULL) {
        *err = errors[0];
    }
    return ret;
}


int gfal_xrootd_release_file(plugin_handle plugin_data,
    const char* url, const char* token, GError** err)
{
    GError *errors[1] = {NULL};
    const char* const urls[1] = {url};
    int ret = gfal_xrootd_release_file_list(plugin_data, 1, urls, token, errors);
    if (errors[0] != NULL) {
        *err = errors[0];
    }
    return ret;
}

int gfal_xrootd_abort_files(plugin_handle plugin_data,
    int nbfiles, const char* const* urls, const char* token, GError** err)
{
    if (nbfiles <= 0) {
        return 1;
    }
    gfal2_context_t context = (gfal2_context_t)plugin_data;

    XrdCl::URL endpoint(prepare_url(context, urls[0]));
    endpoint.SetPath(std::string());
    XrdCl::FileSystem fs(endpoint);

    std::vector<std::string> fileList;
    fileList.emplace_back(token);
    for (int i = 0; i < nbfiles; ++i) {
        XrdCl::URL file(prepare_url(context, urls[i]));
        fileList.emplace_back(file.GetPath());
    }

    XrdCl::Buffer *reponsePtr = 0;
    XrdCl::Status st = fs.Prepare(fileList, XrdCl::PrepareFlags::Flags::Cancel, 0, reponsePtr);
    std::unique_ptr<XrdCl::Buffer> response(reponsePtr);

    if (!st.IsOK()) {
        GError *tmp_err = NULL;
        gfal2_set_error(&tmp_err, xrootd_domain, xrootd_status_to_posix_errno(st),
                        __func__, "%s", st.ToString().c_str());
        for (int i = 0; i < nbfiles; ++i) {
            err[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }
    return 0;
}

