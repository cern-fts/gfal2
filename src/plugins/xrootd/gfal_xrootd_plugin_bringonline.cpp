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


class PollResponseHandler: public XrdCl::ResponseHandler {
private:
    XrdSysCondVar &condVar;
    GError **error;
    int &finishedCounter, &errCounter, &notAnsweredCounter;

public:
    PollResponseHandler(XrdSysCondVar &condVar, GError **error, int &finishedCounter, int &errCounter, int &notAnsweredCounter):
        condVar(condVar), error(error), finishedCounter(finishedCounter), errCounter(errCounter), notAnsweredCounter(notAnsweredCounter) {
    }

    ~PollResponseHandler() {
    }

    void HandleResponse(XrdCl::XRootDStatus *status, XrdCl::AnyObject *response) {
        if (!status->IsOK()) {
            ++errCounter;
            gfal2_set_error(error, xrootd_domain,
                xrootd_errno_to_posix_errno(status->errNo), __func__, "%s", status->GetErrorMessage().c_str());
        }
        delete status;

        XrdCl::StatInfo *info = (XrdCl::StatInfo*)(response);

        condVar.Lock();

        --notAnsweredCounter;
        if (*error) {
            ++errCounter;
        }
        else if (!info->TestFlags(XrdCl::StatInfo::Offline)) {
            ++finishedCounter;
        }
        else {
            gfal2_set_error(error, xrootd_domain, EAGAIN, __func__, "Not online");
        }
        if (notAnsweredCounter <= 0) {
            condVar.Signal();
        }

        condVar.UnLock();
        delete info;
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

    XrdCl::URL endpoint(urls[0]);
    endpoint.SetPath(std::string());
    XrdCl::FileSystem fs(endpoint);

    std::vector<std::string> fileList;
    for (int i = 0; i < nbfiles; ++i) {
        XrdCl::URL file(prepare_url(context, urls[i]));
        fileList.emplace_back(file.GetPath());
    }

    XrdCl::Buffer *reponsePtr;
    XrdCl::Status st = fs.Prepare(fileList, XrdCl::PrepareFlags::Flags::Stage, 0, reponsePtr, timeout);
    std::unique_ptr<XrdCl::Buffer> response(reponsePtr);

    if (!st.IsOK()) {
        GError *tmp_err = NULL;
        gfal2_set_error(&tmp_err, xrootd_domain, xrootd_errno_to_posix_errno(st.errNo),
            __func__, "%s", st.ToString().c_str());
        for (int i = 0; i < 0; ++i) {
            err[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        return -1;
    }

    g_strlcpy(token, response->ToString().c_str(), tsize);
    return 0;
}


int gfal_xrootd_bring_online_poll_list(plugin_handle plugin_data,
    int nbfiles, const char* const* urls, const char* token, GError** err)
{
    if (nbfiles <= 0) {
        return 1;
    }
    gfal2_context_t context = (gfal2_context_t)plugin_data;

    XrdCl::URL endpoint(urls[0]);
    endpoint.SetPath(std::string());
    XrdCl::FileSystem fs(endpoint);
    std::vector<PollResponseHandler> handlers;
    int finishedCounter = 0, errCounter = 0, notAnsweredCounter = nbfiles;
    XrdSysCondVar condVar(0);

    // Make sure all handlers are in place before calling async code
    for (int i = 0; i < nbfiles; ++i) {
        handlers.emplace_back(condVar, &err[i], finishedCounter, errCounter, notAnsweredCounter);
    }
    for (int i = 0; i < nbfiles; ++i) {
        XrdCl::URL file(prepare_url(context, urls[i]));
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
        *err = errors[1];
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
        *err = errors[1];
    }
    return ret;
}
