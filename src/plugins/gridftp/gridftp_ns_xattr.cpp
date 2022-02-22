/*
 * Copyright (c) University of Nebraska-Lincoln 2016
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

#include <sstream>

#include "gridftp_namespace.h"
#include "gridftp_plugin.h"
#include <exceptions/cpp_to_gerror.hpp>
#include "space/gfal2_space.h"


#define GlobusErrorGeneric(reason)                                     \
    globus_error_put(GlobusErrorObjGeneric(reason))

#define GlobusErrorObjGeneric(reason)                                  \
    globus_error_construct_error(                                      \
        GLOBUS_NULL,                                                   \
        GLOBUS_NULL,                                                   \
        1,                                                             \
        __FILE__,                                                      \
        "GFAL GridFTP getxattr",                                       \
        __LINE__,                                                      \
        "%s",                                                          \
        (reason))


static const GQuark GFAL_GRIDFTP_SCOPE_GETXATTR = g_quark_from_static_string("GridFTPModule::getxattr");

struct XAttrState;

extern "C" {
static void gfal_globus_done_callback(void* user_args, globus_object_t *globus_error);
static void gridftp_cancel(gfal2_context_t context, void* userdata);
}
static int callback_cond_wait(XAttrState* state, time_t timeout);


void globus_ftp_control_done_callback(void * user_arg,
    globus_ftp_control_handle_t *handle,
    globus_object_t *error,
    globus_ftp_control_response_t *resp)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, "FTP control operation done");
    gfal_globus_done_callback(user_arg, error ? error : GLOBUS_SUCCESS);
}


struct XAttrState
{

    XAttrState(const char *token, GridFTPFactory* factory) :
        m_token(token), m_url(NULL), m_handle(NULL), m_factory(factory),
        m_cred(GSS_C_NO_CREDENTIAL), m_error(NULL), m_done(true), m_needs_quit(false),
        m_usage(-1), m_free(-1), m_total(-1)
    {
        int global_timeout = gfal2_get_opt_integer_with_default(
            factory->get_gfal2_context(), CORE_CONFIG_GROUP, CORE_CONFIG_NAMESPACE_TIMEOUT, 300);
        m_default_timeout = gfal2_get_opt_integer_with_default(
            factory->get_gfal2_context(), GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_OP_TIMEOUT, global_timeout);

        globus_mutex_init(&m_mutex, NULL);
        globus_cond_init(&m_cond, NULL);
        memset(&m_auth, '\0', sizeof(m_auth));
    }

    ~XAttrState()
    {
        if (!m_done) {
            cancel("XAttrState destructor called before the operation finished!");
            callback_cond_wait(this, m_default_timeout);
        }
        globus_mutex_destroy(&m_mutex);
        globus_cond_destroy(&m_cond);
        delete m_error;
        if (m_url) {
            globus_url_destroy(m_url);
        }
        delete m_url;
        if (m_handle) {
            globus_ftp_control_handle_destroy(m_handle);
        }
        delete m_handle;
    }

    void
    cancel(const std::string& msg)
    {
        //if (!m_needs_quit) return;
        globus_result_t result = globus_ftp_control_force_close(m_handle, globus_ftp_control_done_callback, this);
        gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_GETXATTR, result);
        m_error = new Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GETXATTR, ECANCELED, msg);
    }

    void
    wait(time_t timeout = -1)
    {
        if (timeout < 0)
            timeout = m_default_timeout;

        gfal2_log(G_LOG_LEVEL_DEBUG,
                "   [XAttrState::wait_callback] setup gsiftp timeout to %ld seconds",
                timeout);

        gfal_cancel_token_t cancel_token;
        cancel_token = gfal2_register_cancel_callback(m_factory->get_gfal2_context(), gridftp_cancel, this);

        int wait_ret = callback_cond_wait(this, timeout);

        gfal2_remove_cancel_callback(m_factory->get_gfal2_context(), cancel_token);

        // Operation expired, so cancel and raise an error
        if (wait_ret == ETIMEDOUT) {
            gfal2_log(G_LOG_LEVEL_DEBUG,
                    "   [XAttrState::wait_callback] Operation timeout of %d seconds expired, canceling...",
                    timeout);
            gridftp_cancel(m_factory->get_gfal2_context(), this);

            // Wait again for the callback, ignoring timeout this time
            callback_cond_wait(this, timeout);

            throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GETXATTR, ETIMEDOUT, "Operation timed out");
        }

        if (m_error) {

            if (m_needs_quit)
            {
                m_done = false;
                globus_result_t result = globus_ftp_control_force_close(m_handle, globus_ftp_control_done_callback, this);
                gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_GETXATTR, result);
                callback_cond_wait(this, timeout);
            }

            if (m_error->domain() != 0)
                throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GETXATTR, m_error->code(), m_error->what());
            else
                throw *m_error;
        }
    }

    const char *m_token;
    globus_url_t *m_url;
    globus_ftp_control_handle_t *m_handle;
    GridFTPFactory *m_factory;
    globus_ftp_control_auth_info_t m_auth;
    gss_cred_id_t m_cred;
    globus_mutex_t m_mutex;
    globus_cond_t m_cond;
    Gfal::CoreException *m_error;
    bool m_done;
    bool m_needs_quit;

    time_t m_default_timeout;

    long long m_usage;
    long long m_free;
    long long m_total;
};


extern "C" {

static
void gfal_globus_done_callback(void* user_args,
        globus_object_t *globus_error)
{
    XAttrState* state = (XAttrState*) user_args;

    globus_mutex_lock(&state->m_mutex);
    if (globus_error != GLOBUS_SUCCESS) {
        char *err_buffer;
        int err_code = gfal_globus_error_convert(globus_error, &err_buffer);
        char err_static[2048];
        g_strlcpy(err_static, err_buffer, sizeof(err_static));
        g_free(err_buffer);
        state->m_error = new Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GETXATTR, err_code, err_static);

        // Log complete error dump
        char *chain = globus_error_print_chain(globus_error);
        if (chain != NULL) {
            gfal2_log(G_LOG_LEVEL_DEBUG, chain);
            globus_free(chain);
        }
    }
    state->m_done = true;
    globus_cond_signal(&state->m_cond);
    globus_mutex_unlock(&state->m_mutex);
}


static
void gridftp_cancel(gfal2_context_t context, void* userdata)
{
    XAttrState* state = (XAttrState*)userdata;
    state->cancel("Operation canceled from gfal2_cancel");
}


static void
site_usage_callback(void *arg,
                    globus_ftp_control_handle_t *handle,
                    globus_object_t *err,
                    globus_ftp_control_response_t *resp)
{
    if (resp == GLOBUS_NULL)
    {
        gfal_globus_done_callback(arg, err ? err : GlobusErrorObjGeneric("Site usage invoked with null response"));
        return;
    }
    if (resp->code != 250)
    {
        gfal_globus_done_callback(arg, err ? err : GlobusErrorObjGeneric(resp->response_buffer));
        return;
    }
    //printf("Response: %s", resp->response_buffer);

    XAttrState *state = (XAttrState*)arg;
    if (3 != sscanf((const char *)resp->response_buffer, "250 USAGE %lld FREE %lld TOTAL %lld", &state->m_usage, &state->m_free, &state->m_total))
    {
        gfal_globus_done_callback(arg, GlobusErrorObjGeneric("Invalid SITE USAGE response from server."));
        return;
    }
    if ((state->m_total < 0) && (state->m_free >= 0) && (state->m_usage >= 0))
    {
        state->m_total = state->m_free + state->m_usage;
    }

    gfal_globus_done_callback(arg, GLOBUS_SUCCESS);
}

static void
authenticate_callback(void *arg,
                      globus_ftp_control_handle_t *handle,
                      globus_object_t *err,
                      globus_ftp_control_response_t *resp)
{
    if (resp == GLOBUS_NULL)
    {
        gfal_globus_done_callback(arg, err ? err : GlobusErrorObjGeneric("Authenticate invoked with null response"));
        return;
    }
    if (resp->code != 230)
    {
        gfal_globus_done_callback(arg, err ? err : GlobusErrorObjGeneric("Authentication failed."));
        return;
    }
    XAttrState *state = (XAttrState*)arg;

    globus_result_t result;
    if (state->m_token)
    {
        //printf("SITE USAGE TOKEN %s /%s\n", state->m_token, state->m_url->url_path);
        result = globus_ftp_control_send_command(handle,
            "SITE USAGE TOKEN %s /%s\r\n",
            site_usage_callback,
            state,
            state->m_token,
            state->m_url->url_path);
    }
    else
    {
        //printf("SITE USAGE /%s\n", state->m_url->url_path);
        result = globus_ftp_control_send_command(handle,
            "SITE USAGE /%s\r\n",
            site_usage_callback,
            state,
            state->m_url->url_path);
    }
    if (result != GLOBUS_SUCCESS) {gfal_globus_done_callback(arg, globus_error_get(result));}
}


static void
connect_callback(void *arg,
                 globus_ftp_control_handle_t *handle,
                 globus_object_t *err,
                 globus_ftp_control_response_t *resp)
{
    if (resp == GLOBUS_NULL)
    {
        gfal_globus_done_callback(arg, (err != GLOBUS_SUCCESS) ? err : GlobusErrorObjGeneric("Connect invoked with null response"));
        return;
    }
    //printf("Login message: %s\n", resp->response_buffer);

    XAttrState *state = (XAttrState*)arg;
    globus_mutex_lock(&state->m_mutex);
    {
        state->m_needs_quit = true;
    }
    globus_mutex_unlock(&state->m_mutex);

    if (resp->code != 220)
    {
        gfal_globus_done_callback(arg, GlobusErrorObjGeneric("Server did not indicate successful connection."));
        return;
    }

    globus_result_t result = globus_ftp_control_auth_info_init(
                         &state->m_auth,
                         state->m_cred,
                         GLOBUS_FALSE,
                         NULL,
                         NULL,
                         NULL,
                         NULL);
    if (result != GLOBUS_SUCCESS) {gfal_globus_done_callback(arg, globus_error_get(result));}

    result = globus_ftp_control_authenticate(
                     handle,
                     &state->m_auth,
                     GLOBUS_TRUE,
                     authenticate_callback,
                     arg);
    if (result != GLOBUS_SUCCESS) {gfal_globus_done_callback(arg, globus_error_get(result));}
}

}


static int callback_cond_wait(XAttrState* state, time_t timeout)
{
    globus_abstime_t timeout_expires;
    GlobusTimeAbstimeGetCurrent(timeout_expires);
    timeout_expires.tv_sec += timeout;

    globus_mutex_lock(&state->m_mutex);
    int wait_ret = 0;
    while (!state->m_done && wait_ret != ETIMEDOUT) {
        wait_ret = globus_cond_timedwait(&state->m_cond, &state->m_mutex, &timeout_expires);
    }
    globus_mutex_unlock(&state->m_mutex);
    return wait_ret;
}

ssize_t GridFTPModule::getxattr(const char *path,
    const char *name, void *buff, size_t s_buff)
{
    if (path == NULL) {
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GETXATTR, EINVAL,
                "Invalid path argument");
    }

    if (strncmp(name, GFAL_XATTR_SPACETOKEN, 10) != 0) {
        std::stringstream msg;
        msg << "'" << name << "' extended attributed not supported by GridFTP plugin";
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GETXATTR, ENOATTR, msg.str());
    }
    const char *qmark = strchr(name, '?');
    const char *token = NULL;
    if (qmark) {
        token = qmark+1;
    }

    bool is_descr = false;
    const char *dot = strchr(name, '.');
    if ((dot != NULL) && (strncmp(dot, ".description", 12) == 0))
    {
        is_descr = true;
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, " -> [GridFTPModule::getxattr] ");

    XAttrState handler(token, _handle_factory);

    OM_uint32 min;
    OM_uint32 maj = gss_acquire_cred(
                    &min,
                    GSS_C_NO_NAME,
                    0,
                    GSS_C_NO_OID_SET,
                    GSS_C_BOTH,
                    &handler.m_cred,
                    NULL,
                    NULL);
    if (maj != GSS_S_COMPLETE)
    {
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_GETXATTR, ENOATTR,
                "failed to acquire client credential");
    }


    handler.m_url = new globus_url_t;
    globus_result_t result = globus_url_parse_rfc1738(path, handler.m_url);
    gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_GETXATTR, result);

    handler.m_handle = new globus_ftp_control_handle_t;
    result = globus_ftp_control_handle_init(handler.m_handle);
    gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_GETXATTR, result);

    short int port = handler.m_url->port == 0 ? 2811 : handler.m_url->port;
    handler.m_done = false;
    result = globus_ftp_control_connect(handler.m_handle, handler.m_url->host, port, connect_callback, &handler);
    if (result != GLOBUS_SUCCESS) {handler.m_done = true;}
    gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_GETXATTR, result);

    handler.wait();

    if (handler.m_needs_quit)
    {
        handler.m_done = false;
        result = globus_ftp_control_quit(handler.m_handle, globus_ftp_control_done_callback, &handler);
        gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_GETXATTR, result);

        handler.wait();
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, " <- [GridFTPModule::getxattr] ");

    struct space_report report = {0};

    report.used = handler.m_usage;
    report.free = handler.m_free;
    report.total = handler.m_total;

    return gfal2_space_generate_json(&report, (char*)buff, s_buff);
}


extern "C" ssize_t gfal_gridftp_getxattrG(plugin_handle handle, const char* path,
        const char *name, void *buff, size_t s_buff, GError** err)
{
    g_return_val_err_if_fail(handle != NULL && path != NULL && name != NULL && buff != NULL, -1, err,
            "[gfal_gridftp_getxattrG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal2_log(G_LOG_LEVEL_DEBUG, "  -> [gfal_gridftp_getxattrG]");
    CPP_GERROR_TRY
            ret = (static_cast<GridFTPModule*>(handle))->getxattr(path, name, buff, s_buff);
    CPP_GERROR_CATCH(&tmp_err);
    gfal2_log(G_LOG_LEVEL_DEBUG, "  [gfal_gridftp_getxattrG]<-");
    G_RETURN_ERR(ret, tmp_err, err);
}


extern "C" ssize_t gfal_gridftp_listxattrG(plugin_handle handle,
    const char* url, char* list, size_t s_list, GError** err)
{
    return g_strlcpy(list, GFAL_XATTR_SPACETOKEN, s_list);
}
