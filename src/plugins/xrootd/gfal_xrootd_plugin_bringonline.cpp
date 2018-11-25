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
#include <XrdCl/XrdClFileSystem.hh>
#include <XrdSys/XrdSysPthread.hh>

class PollErrorResponseHandler: public XrdCl::ResponseHandler {
private:
    XrdSysCondVar &condVar;
    GError **error;
    int &finishedCounter, &errCounter, &notAnsweredCounter;
public:
    PollErrorResponseHandler( XrdSysCondVar &condVar, GError **error, int &finishedCounter, int &errCounter, int &notAnsweredCounter):
        condVar(condVar), error(error), finishedCounter(finishedCounter), errCounter(errCounter), notAnsweredCounter(notAnsweredCounter) {
    }

    ~PollErrorResponseHandler() {
    }

    void HandleResponse(XrdCl::XRootDStatus *status, XrdCl::AnyObject *res) {
        if (!status->IsOK()) {
            ++errCounter;
            gfal2_log(G_LOG_LEVEL_DEBUG, "Error doing the query");
            gfal2_set_error(error, xrootd_domain,
               xrootd_errno_to_posix_errno(status->errNo), __func__, "%s", status->GetErrorMessage().c_str());
        }
        delete status;

        condVar.Lock();
        --notAnsweredCounter;
        
        XrdCl::Buffer * response= 0;
        res->Get(response);

        if (*error) {
            ++errCounter;
        } else if (response->GetBuffer() != NULL)  { 
            int retc;
            char tag[1024];
            char error_string[1024];
            error_string[0] = 0;
            gfal2_log(G_LOG_LEVEL_DEBUG, "Response: %s", response->GetBuffer());
            sscanf(response->GetBuffer(),
                "%s retc=%i value=%[^\n]",
                tag, &retc, error_string);
            if (retc || (strlen(error_string) != 0 )) {
                gfal2_log(G_LOG_LEVEL_DEBUG, "Error reported: %s", error_string);
                gfal2_set_error(error, xrootd_domain, EIO, __func__, "%s",error_string);
                ++errCounter;
            } else {
               gfal2_log(G_LOG_LEVEL_DEBUG, "No error reported");
               gfal2_set_error(error, xrootd_domain, EAGAIN, __func__, "%s","Not online");
            }
        } else {
             gfal2_set_error(error, xrootd_domain, EAGAIN, __func__, "%s","Not online");
        }
        if (notAnsweredCounter <= 0) {
            condVar.UnLock();
            condVar.Signal();
            condVar.Lock();
        }
        condVar.UnLock();

        delete res;
    }

    // std::vector expects an = operator, and the default one is no good
    const PollErrorResponseHandler operator = (const PollErrorResponseHandler &b) {
        return *this;
    }

};


class PollResponseHandler: public XrdCl::ResponseHandler {
private:
    XrdSysCondVar &condVar;
    GError **error;
    int &finishedCounter, &errCounter, &notAnsweredCounter;
    const char* url;
public:
    PollResponseHandler( XrdSysCondVar &condVar, GError **error, int &finishedCounter, int &errCounter, int &notAnsweredCounter):
        url(url),condVar(condVar), error(error), finishedCounter(finishedCounter), errCounter(errCounter), notAnsweredCounter(notAnsweredCounter) {
    }

    ~PollResponseHandler() {
    }

    void HandleResponse(XrdCl::XRootDStatus *status, XrdCl::AnyObject *response) {
        if (!status->IsOK()) {
            ++errCounter;
            gfal2_log(G_LOG_LEVEL_DEBUG, "Error doing the query");
            gfal2_set_error(error, xrootd_domain,
               xrootd_errno_to_posix_errno(status->errNo), __func__, "%s", status->GetErrorMessage().c_str());
        }
        delete status;
        
        XrdCl::StatInfo *info = 0;
        response->Get(info);

        condVar.Lock();

        --notAnsweredCounter;
        if (*error) {
            ++errCounter;
        }
        else if (!(info->TestFlags(XrdCl::StatInfo::Offline))) {
           gfal2_log(G_LOG_LEVEL_DEBUG, "file online");  
           ++finishedCounter;
        }
        else { 
           gfal2_set_error(error, xrootd_domain, EAGAIN, __func__, "%s","Not online");
        }
        if (notAnsweredCounter <= 0) {
            condVar.UnLock();
            condVar.Signal();
            condVar.Lock();
        }
        condVar.UnLock();
        delete response;
    }

    // std::vector expects an = operator, and the default one is no good
    const PollResponseHandler operator = (const PollResponseHandler &b) {
        return *this;
    }
};


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

    XrdCl::Buffer *responsePtr;
    XrdCl::Status st = fs.Prepare(fileList, XrdCl::PrepareFlags::Flags::Stage, 0, responsePtr, timeout);

    if (!st.IsOK()) {
        GError *tmp_err = NULL;
        gfal2_set_error(&tmp_err, xrootd_domain, xrootd_errno_to_posix_errno(st.errNo),
            __func__, "%s", st.ToString().c_str());
        for (int i = 0; i < nbfiles; ++i) {
            err[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        delete responsePtr;
        return -1;
    }
    if (responsePtr && responsePtr->GetBuffer()) {
        g_strlcpy(token, responsePtr->GetBuffer(), tsize);
    } else {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Empty response from the server");
        delete responsePtr;
        return -1;
    }
    delete responsePtr;
    return 0;
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
    std::vector<PollResponseHandler> handlers;
    int finishedCounter = 0, errCounter = 0, notAnsweredCounter = nbfiles;
    XrdSysCondVar condVar(0);

    // Make sure all handlers are in place before calling async code
    for (int i = 0; i < nbfiles; i++) {
        handlers.emplace_back(condVar, &err[i], finishedCounter, errCounter, notAnsweredCounter);
    }
    for (int i = 0; i < nbfiles; i++) {
        XrdCl::URL file(prepare_url(context, urls[i]));
        gfal2_log(G_LOG_LEVEL_DEBUG, "Doing file stat to check if the file is online: %s ", file.GetPath().c_str());
        XrdCl::Status st = fs.Stat(file.GetPath(), &handlers[i]);
        if (!st.IsOK()) {
            condVar.Lock();
            errCounter++;
            condVar.UnLock();
            gfal2_set_error(&err[i], xrootd_domain, xrootd_errno_to_posix_errno(st.errNo),
                __func__, "%s", st.ToString().c_str());
        }
    }

    condVar.Lock();
    condVar.Wait(300);
    condVar.UnLock();
 
    notAnsweredCounter = 0;
   
    for (int i = 0; i < nbfiles; i++) {
        if (err[i]){
            notAnsweredCounter++;
        } 
    }
    
    if (notAnsweredCounter > 0) {
      std::vector<PollErrorResponseHandler> errorHandlers;
      for (int i = 0; i < nbfiles; i++) {
          errorHandlers.emplace_back(condVar, &err[i], finishedCounter, errCounter, notAnsweredCounter);
      } 
      for (int i = 0 ; i < nbfiles; i++) {
          if (!err[i]) {
            continue;
          }
          g_clear_error(&err[i]); 
          gfal2_log(G_LOG_LEVEL_DEBUG, "Invoke the query for the error attribute for file: %s", urls[i]);
          //invoke the query for the error attribute
          XrdCl::Buffer arg; 
          XrdCl::URL file(prepare_url(context, urls[i]));
          //build the opaque
          std::ostringstream sstr;
          sstr << file.GetPath() << "?mgm.pcmd=xattr&mgm.subcmd=get&mgm.xattrname=sys.retrieve.error";
          arg.FromString(sstr.str());
          XrdCl::Status status = fs.Query(XrdCl::QueryCode::Code::OpaqueFile , arg, &errorHandlers[i]);

          if (!status.IsOK()) {
                gfal2_log(G_LOG_LEVEL_DEBUG, "Error submitting query for extended attribute");
                gfal2_set_error(&err[i], xrootd_domain, EAGAIN, __func__, "%s","Not online");
          } 
      
      } 
    
      condVar.Lock();
      condVar.Wait(300);
      condVar.UnLock();
     
    }
    if (finishedCounter == nbfiles) {
        return 1;
    }
    else if (errCounter > 0) {
        return -1;
    }
    return 0;
}


int gfal_xrootd_release_file_list(plugin_handle plugin_data,
    int nbfiles, const char* const* urls, const char* token, GError** err)
{
    // Noop
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
    for (int i = 0; i < nbfiles; ++i) {
        XrdCl::URL file(prepare_url(context, urls[i]));
        fileList.emplace_back(file.GetPath());
    }

    XrdCl::Buffer *reponsePtr;
    //TODO : we use Fresh as a flag now, to change to Abort once it's implemented in xrootd
    XrdCl::Status st = fs.Prepare(fileList, XrdCl::PrepareFlags::Flags::Fresh, 0, reponsePtr);
    std::unique_ptr<XrdCl::Buffer> response(reponsePtr);

    if (!st.IsOK()) {
        GError *tmp_err = NULL;
        gfal2_set_error(&tmp_err, xrootd_domain, xrootd_errno_to_posix_errno(st.errNo),
            __func__, "%s", st.ToString().c_str());
        for (int i = 0; i < nbfiles; ++i) {
            err[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }
    return 0;
}

