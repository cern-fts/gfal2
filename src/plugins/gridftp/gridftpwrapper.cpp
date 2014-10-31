/*
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <memory>
#include <fstream>
#include <sstream>
#include <uri/gfal_uri.h>
#include <exceptions/gfalcoreexception.hpp>
#include "gridftp_plugin.h"
#include "gridftpwrapper.h"

#include <globus_ftp_client_debug_plugin.h>


struct RwStatus
{
    off_t init;
    off_t finish;
    bool ops_state;
};


static Glib::Quark GFAL_GRIDFTP_SCOPE_REQ_STATE("GridFTPModule::RequestState");


static std::string gridftp_hostname_from_url(const std::string& url)
{
    GError * tmp_err = NULL;
    char buffer[GFAL_URL_MAX_LEN];
    buffer[0] = '\0';
    const int res = gfal_hostname_from_uri(url.c_str(), buffer, GFAL_URL_MAX_LEN, &tmp_err);
    if (res < 0)
        throw Glib::Error(tmp_err);
    return std::string(buffer);
}


GassCopyAttrHandler::GassCopyAttrHandler(globus_ftp_client_operationattr_t* ftp_operation_attr)
{
    // initialize gass copy attr
    globus_result_t res = globus_gass_copy_attr_init(&(attr_gass));
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);
    globus_ftp_client_operationattr_init(&(operation_attr_ftp_for_gass));
    globus_ftp_client_operationattr_copy(&(operation_attr_ftp_for_gass), ftp_operation_attr);
    res = globus_gass_copy_attr_set_ftp(&(attr_gass), &operation_attr_ftp_for_gass);
    gfal_globus_check_result("GridFTPFactory::globus_gass_copy_handleattr_set_ftp_attr", res);

}


GassCopyAttrHandler::~GassCopyAttrHandler()
{
    globus_ftp_client_operationattr_destroy(&(operation_attr_ftp_for_gass));
}


GridFTPSessionHandler::GridFTPSessionHandler(GridFTPFactory* f, const std::string &uri) :
        _isDirty(false), factory(f), hostname(gridftp_hostname_from_url(uri))
{
    this->session = f->get_session(this->hostname);
}


GridFTPSessionHandler::~GridFTPSessionHandler()
{
    try {
        factory->release_session(this->session, this->_isDirty);
    }
    catch (const std::exception& e) {
        gfal_log(GFAL_VERBOSE_NORMAL,
                "Caught an exception inside ~GridFTP_session()!! %s", e.what());
    }
    catch (...) {
        gfal_log(GFAL_VERBOSE_NORMAL,
                "Caught an unknown exception inside ~GridFTP_session()!!");
    }
}


GridFTPSession::GridFTPSession(const std::string& hostname): hostname(hostname)
{
    globus_result_t res;

    res = globus_ftp_client_debug_plugin_init(&debug_ftp_plugin, stderr, "gridftp debug :");
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_ops_attr", res);

    res = globus_ftp_client_operationattr_init(&operation_attr_ftp);
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_ops_attr", res);

    res = globus_ftp_client_handleattr_init(&attr_handle);
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle_attr", res);

    globus_ftp_client_handleattr_set_cache_all(&attr_handle, GLOBUS_TRUE); // enable session re-use
    if (gfal_get_verbose() & GFAL_VERBOSE_TRACE_PLUGIN)
        globus_ftp_client_handleattr_add_plugin(&attr_handle, &debug_ftp_plugin);

    res = globus_gass_copy_handleattr_init(&gass_handle_attr);
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);

    res = globus_gass_copy_handleattr_set_ftp_attr(&gass_handle_attr, &attr_handle);
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);

    res = globus_gass_copy_handle_init(&gass_handle, &gass_handle_attr);
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);

    res = globus_gass_copy_get_ftp_handle(&gass_handle, &handle_ftp);
    gfal_globus_check_result("GridFTPFactory::GridFTP_session", res);

    this->set_nb_streams(0);
}


GridFTPSession::~GridFTPSession()
{
    globus_ftp_client_debug_plugin_destroy(&debug_ftp_plugin);
    globus_gass_copy_handle_destroy(&gass_handle);
    globus_ftp_client_operationattr_destroy(&operation_attr_ftp);
    globus_gass_copy_handleattr_destroy(&gass_handle_attr);
    globus_ftp_client_handleattr_destroy(&attr_handle);
}


void GridFTPSession::set_gridftpv2(bool v2)
{
    globus_ftp_client_handleattr_set_gridftp2(&attr_handle, v2);
}


void GridFTPSession::set_ipv6(bool ipv6)
{
    globus_ftp_client_operationattr_set_allow_ipv6(&operation_attr_ftp, ipv6);
}


void GridFTPSession::set_delayed_pass(bool delayed)
{
    globus_ftp_client_operationattr_set_delayed_pasv(&operation_attr_ftp, delayed);
}


void GridFTPSession::set_dcau(bool dcau)
{
    if (dcau)
        dcau_control.mode = GLOBUS_FTP_CONTROL_DCAU_DEFAULT;
    else
        dcau_control.mode = GLOBUS_FTP_CONTROL_DCAU_NONE;
    globus_ftp_client_operationattr_set_dcau(&operation_attr_ftp, &dcau_control);
}


void GridFTPSession::set_nb_streams(unsigned int nbstream)
{
    if (nbstream == 0) {
        parallelism.fixed.size = 1;
        parallelism.mode = GLOBUS_FTP_CONTROL_PARALLELISM_NONE;
        mode = GLOBUS_FTP_CONTROL_MODE_NONE;
    }
    else {
        parallelism.fixed.size = nbstream;
        parallelism.mode = GLOBUS_FTP_CONTROL_PARALLELISM_FIXED;
        mode = GLOBUS_FTP_CONTROL_MODE_EXTENDED_BLOCK;
    }
    globus_ftp_client_operationattr_set_mode(&operation_attr_ftp, mode);
    globus_ftp_client_operationattr_set_parallelism(&operation_attr_ftp, &parallelism);
}


void GridFTPSession::set_tcp_buffer_size(guint64 buffersize)
{
    if (buffersize == 0) {
        tcp_buffer_size.mode = GLOBUS_FTP_CONTROL_TCPBUFFER_DEFAULT;
        tcp_buffer_size.fixed.size = 0ul;
    }
    else {
        tcp_buffer_size.mode = GLOBUS_FTP_CONTROL_TCPBUFFER_FIXED;
        tcp_buffer_size.fixed.size = buffersize;
    }
    globus_ftp_client_operationattr_set_tcp_buffer(&operation_attr_ftp, &tcp_buffer_size);
}


void GridFTPSession::set_udt(bool udt)
{
    if (udt)
        globus_ftp_client_operationattr_set_net_stack(&operation_attr_ftp, "udt");
    else
        globus_ftp_client_operationattr_set_net_stack(&operation_attr_ftp, "default");
}


void gfal_globus_set_credentials(gfal2_context_t context, globus_ftp_client_operationattr_t* opattr)
{
    gchar* ucert = gfal2_get_opt_string(context, "X509", "CERT", NULL);
    gchar* ukey = gfal2_get_opt_string(context, "X509", "KEY", NULL);
    if (ucert) {
        gfal_log(GFAL_VERBOSE_TRACE, "GSIFTP using certificate %s", ucert);
        if (ukey)
            gfal_log(GFAL_VERBOSE_TRACE, "GSIFTP using private key %s", ukey);
        gfal_globus_set_credentials(ucert, ukey, opattr);
        g_free(ucert);
        g_free(ukey);
    }
}


void gfal_globus_set_credentials(const char* ucert, const char* ukey, globus_ftp_client_operationattr_t* opattr)
{
    std::stringstream buffer;
    std::ifstream cert_stream(ucert);
    if (!cert_stream.good()) {
        throw Glib::Error(GFAL_GRIDFTP_SCOPE_REQ_STATE, errno,
                "Could not open the user certificate");
    }

    buffer << cert_stream.rdbuf();
    if (ukey && strcmp(ucert, ukey) != 0) {
        std::ifstream key_stream(ukey);
        if (key_stream.bad())
            throw Glib::Error(GFAL_GRIDFTP_SCOPE_REQ_STATE, errno,
                    "Could not open the user private key");
        buffer << key_stream.rdbuf();
    }

    gss_buffer_desc_struct buffer_desc;
    buffer_desc.value = g_strdup(buffer.str().c_str());
    buffer_desc.length = buffer.str().size();

    OM_uint32 minor_status, major_status;
    gss_cred_id_t cred_id;
    major_status = gss_import_cred(&minor_status, &cred_id,
            GSS_C_NO_OID, 0, // 0 = Pass credentials; 1 = Pass path as X509_USER_PROXY=...
            &buffer_desc, 0, NULL);
    g_free(buffer_desc.value);

    if (major_status != GSS_S_COMPLETE) {
        std::stringstream err_buffer;

        err_buffer << "Could not load the user credentials: ";

        globus_object_t * error = globus_error_get(minor_status);
        char* globus_errstr;
        int globus_errno = gfal_globus_error_convert(error, &globus_errstr);
        if (globus_errstr) {
            err_buffer << globus_errstr;
            g_free (globus_errstr);
        }
        globus_object_free(error);

        err_buffer << " (" << globus_errno << ")";

        throw Glib::Error(GFAL_GRIDFTP_SCOPE_REQ_STATE, globus_errno,
                err_buffer.str());
    }
    globus_ftp_client_operationattr_set_authorization(
            opattr, cred_id, NULL, NULL, NULL, NULL);
}


globus_ftp_client_handle_t* GridFTPSessionHandler::get_ftp_client_handle()
{
    globus_result_t res = globus_gass_copy_get_ftp_handle(&(session->gass_handle),
            &(session->handle_ftp));
    gfal_globus_check_result("GridFTPFactory::GridFTP_session", res);
    return &(session->handle_ftp);
}


globus_gass_copy_handle_t* GridFTPSessionHandler::get_gass_copy_handle()
{
    return &(session->gass_handle);
}


globus_ftp_client_operationattr_t* GridFTPSessionHandler::get_ftp_client_operationattr()
{
    return &(session->operation_attr_ftp);
}


globus_gass_copy_handleattr_t* GridFTPSessionHandler::get_gass_copy_handleattr()
{
    return &(session->gass_handle_attr);
}

globus_ftp_client_handleattr_t* GridFTPSessionHandler::get_ftp_client_handleattr()
{
    return &(session->attr_handle);
}

GridFTPFactory* GridFTPSessionHandler::get_factory()
{
    return factory;
}


void GridFTPSessionHandler::disable_reuse()
{
    _isDirty = true;
}


GridFTPFactory::GridFTPFactory(gfal2_context_t handle) :
        gfal2_context(handle)
{
    GError * tmp_err = NULL;
    session_reuse = gfal2_get_opt_boolean(gfal2_context, GRIDFTP_CONFIG_GROUP,
            GRIDFTP_CONFIG_SESSION_REUSE, &tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, " define GSIFTP session re-use to %s",
            (session_reuse) ? "TRUE" : "FALSE");
    if (tmp_err)
        throw Glib::Error(tmp_err);
    size_cache = 400;
}


void GridFTPFactory::clear_cache()
{
    gfal_log(GFAL_VERBOSE_TRACE, "gridftp session cache garbage collection ...");
    std::multimap<std::string, GridFTPSession*>::iterator it;
    for (it = session_cache.begin(); it != session_cache.end(); ++it) {
        delete (*it).second;
    }
    session_cache.clear();
}


void GridFTPFactory::recycle_session(GridFTPSession* session)
{
    Glib::Mutex::Lock l(mux_cache);

    if (session_cache.size() > size_cache)
        clear_cache();

    gfal_log(GFAL_VERBOSE_TRACE, "insert gridftp session for %s in cache ...", session->hostname.c_str());
    session_cache.insert(std::pair<std::string, GridFTPSession*>(session->hostname, session));
}


// recycle a gridftp session object from cache if exist, return NULL else
GridFTPSession* GridFTPFactory::get_recycled_handle(
        const std::string & hostname)
{
    Glib::Mutex::Lock l(mux_cache);
    GridFTPSession* session = NULL;
    std::multimap<std::string, GridFTPSession*>::iterator it = session_cache.find(
            hostname); // try to find a session explicitly associated with this handle
    if (it == session_cache.end()) { // if no session found, take a generic one
        gfal_log(GFAL_VERBOSE_TRACE,
                "no session associated with this hostname, try find generic one .... ");
        it = session_cache.begin();
    }
    if (it != session_cache.end()) {
        gfal_log(GFAL_VERBOSE_TRACE,"gridftp session for: %s found in  cache !", hostname.c_str());
        session = (*it).second;
        session_cache.erase(it);
    }
    else {
        gfal_log(GFAL_VERBOSE_TRACE, "no session found in cache for %s!", hostname.c_str());
    }
    return session;
}


GridFTPFactory::~GridFTPFactory()
{
    try {
        Glib::Mutex::Lock l(mux_cache);
        clear_cache();
    }
    catch (const std::exception & e) {
        gfal_log(GFAL_VERBOSE_NORMAL,
                "Caught an exception inside ~GridFTPFactory()!! %s", e.what());
    }
    catch (...) {
        gfal_log(GFAL_VERBOSE_NORMAL,
                "Caught an unknown exception inside ~GridFTPFactory()!!");
    }
}


gfal2_context_t GridFTPFactory::get_gfal2_context()
{
    return gfal2_context;
}

/*
 *  dirty function to convert error code from globus
 *  In the current state, globus provides no way to convert gridftp error code to errno properly....
 * */
static int scan_errstring(const char *p) {

    int ret = ECOMM;
    if (p == NULL) return ret;

    if (strstr(p, "o such file") || strstr(p, "not found") || strstr(p, "error 3011"))
        ret = ENOENT;
    else if (strstr(p, "ermission denied") || strstr(p, "credential"))
        ret = EACCES;
    else if ( (strstr(p, "exists")) || strstr(p, "error 3006"))
        ret = EEXIST;
    else if (strstr(p, "ot a direct"))
		ret = ENOTDIR;
    else if (strstr(p, "ation not sup"))
        ret = ENOTSUP;
    else if (strstr(p, "Login incorrect") || strstr(p, "Could not get virtual id"))
        ret = EACCES;
    else if (strstr(p, "the operation was aborted"))
        ret = ECANCELED;
    return ret;
}


int gfal_globus_error_convert(globus_object_t * error, char ** str_error)
{
    if (error) {
        *str_error = globus_error_print_friendly(error);
        char * p = *str_error;
        while (*p != '\0') { // string normalization of carriage return
            *p = (*p == '\n' || *p == '\r') ? ' ' : *p;
            ++p;
        }
        int errn = scan_errstring(*str_error); // try to get errno
        if (errn == 0) {
            globus_free(*str_error);
            *str_error = NULL;
        }
        return errn;
    }
    return 0;
}


static
void gfal_globus_check_error(const Glib::Quark & scope, globus_object_t * error)
{
    if (error != GLOBUS_SUCCESS) {
        int globus_errno;
        char errbuff[GFAL_URL_MAX_LEN];
        char * glob_str = NULL;
        *errbuff = '\0';

        globus_errno = gfal_globus_error_convert(error, &glob_str);
        if (glob_str) { // security
            g_strlcpy(errbuff, glob_str, GFAL_URL_MAX_LEN);
            g_free(glob_str);
        }
        globus_object_free(error);
        throw Gfal::CoreException(scope, errbuff, globus_errno);
    }
}


void gfal_globus_check_result(const Glib::Quark & scope, globus_result_t res)
{
    if (res != GLOBUS_SUCCESS) {

        globus_object_t * error = globus_error_get(res); // get error from result code
        if (error == NULL)
            throw Gfal::CoreException(scope,
                    "Unknown error: unable to map result code to globus error",
                    EINVAL);
        gfal_globus_check_error(scope, error);
    }
}


GridFTPSession* GridFTPFactory::get_new_handle(const std::string & hostname)
{

    bool gridftp_v2 = gfal2_get_opt_boolean_with_default(gfal2_context, GRIDFTP_CONFIG_GROUP,
            GRIDFTP_CONFIG_V2, true);
    bool ipv6 = gfal2_get_opt_boolean_with_default(gfal2_context,
            GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_IPV6, false);
    bool delay_passv = gfal2_get_opt_boolean_with_default(gfal2_context,
            GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_DELAY_PASSV, true);
    bool dcau = gfal2_get_opt_boolean_with_default(gfal2_context, GRIDFTP_CONFIG_GROUP,
            GRIDFTP_CONFIG_DCAU, false);

    std::auto_ptr<GridFTPSession> session(new GridFTPSession(hostname));

    session->set_gridftpv2(gridftp_v2);
    session->set_dcau(dcau);
    session->set_ipv6(ipv6);
    session->set_delayed_pass(delay_passv);

    gfal_globus_set_credentials(gfal2_context, &session->operation_attr_ftp);

    return session.release();
}


GridFTPSession* GridFTPFactory::get_session(const std::string &hostname)
{
    GridFTPSession* session = NULL;
    if ((session = get_recycled_handle(hostname)) == NULL)
        session = get_new_handle(hostname);
    return session;
}


void GridFTPFactory::release_session(GridFTPSession* session, bool destroy)
{
    session_reuse = gfal2_get_opt_boolean_with_default(gfal2_context, GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_SESSION_REUSE, FALSE);
    if (session_reuse && !destroy)
        recycle_session(session);
    else {
        gfal_log(GFAL_VERBOSE_TRACE, "destroy gridftp session for %s ...", session->hostname.c_str());
        delete session;
    }
}


static
void gfal_globus_done_callback(void* user_args,
        globus_object_t *globus_error)
{
    GridFTPRequestState* state = (GridFTPRequestState*) user_args;

    globus_mutex_lock(&state->lock);
    if (globus_error != GLOBUS_SUCCESS) {
        char *err_buffer;
        int err_code = gfal_globus_error_convert(globus_error, &err_buffer);
        char err_static[128];
        g_strlcpy(err_static, err_buffer, sizeof(err_static));
        g_free(err_buffer);
        state->error = new Gfal::CoreException(Glib::Quark(""), err_static, err_code);
    }
    state->done = true;
    globus_cond_signal(&state->cond);
    globus_mutex_unlock(&state->lock);
}


// gridftp callback generic implementation
void globus_ftp_client_done_callback(void * user_arg,
        globus_ftp_client_handle_t * handle, globus_object_t * error)
{
    gfal_log(GFAL_VERBOSE_TRACE, " gridFTP operation done");
    gfal_globus_done_callback(user_arg, error);
}

// gass operation callback implementation
void globus_gass_client_done_callback(void * callback_arg,
        globus_gass_copy_handle_t * handle, globus_object_t * error)
{
    gfal_log(GFAL_VERBOSE_TRACE, "gass operation done");
    gfal_globus_done_callback(callback_arg, error);
}


GridFTPRequestState::GridFTPRequestState(GridFTPSessionHandler* s,
        GridFTPRequestType request_type) :
        handler(s), request_type(request_type), error(NULL), done(false)
{
    this->default_timeout = gfal2_get_opt_integer_with_default(
            s->get_factory()->get_gfal2_context(), GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_OP_TIMEOUT, 300);
    globus_mutex_init(&lock, NULL);
    globus_cond_init(&cond, NULL);
}


GridFTPRequestState::~GridFTPRequestState()
{
    globus_mutex_destroy(&lock);
    globus_cond_destroy(&cond);
    delete error;
    if (!done) {
        this->cancel(GFAL_GRIDFTP_SCOPE_REQ_STATE,
                "GridFTPRequestState destructor called before the operation finished!");
    }
}


static
void gridftp_cancel(gfal2_context_t context, void* userdata)
{
    GridFTPRequestState* state = (GridFTPRequestState*)userdata;
    state->cancel(g_quark_to_string(gfal_cancel_quark()), "Operation canceled from gfal2_cancel");
}


void GridFTPRequestState::wait(const Glib::Quark &scope, time_t timeout)
{
    if (timeout < 0)
        timeout = default_timeout;
    gfal_log(GFAL_VERBOSE_TRACE,
            "   [GridFTP_Request_state::wait_callback] setup gsiftp timeout to %ld seconds",
            timeout);

    globus_abstime_t timeout_expires;
            GlobusTimeAbstimeGetCurrent(timeout_expires);
            timeout_expires.tv_sec += timeout;

    gfal_cancel_token_t cancel_token;
    cancel_token = gfal2_register_cancel_callback(handler->get_factory()->get_gfal2_context(), gridftp_cancel, this);

    globus_mutex_lock(&lock);
    int wait_ret = 0;
    while (!done) {
        wait_ret = globus_cond_timedwait(&cond, &lock, &timeout_expires);
    }
    globus_mutex_unlock(&lock);

    gfal2_remove_cancel_callback(handler->get_factory()->get_gfal2_context(), cancel_token);

    if (wait_ret == ETIMEDOUT)
        throw Gfal::CoreException(scope, "Operation timed out", ETIMEDOUT);

    if (error) {
        if (error->domain() != Glib::Quark("").id())
            throw Gfal::CoreException(scope, error->what(), error->code());
        else
            throw *error;
    }
}


void GridFTPRequestState::cancel(const Glib::Quark &scope, const std::string& msg)
{
    if (request_type == GRIDFTP_REQUEST_FTP) {
        globus_ftp_client_abort(handler->get_ftp_client_handle());
    }
    else {
        globus_gass_copy_cancel(handler->get_gass_copy_handle(),
                globus_gass_client_done_callback, this);
    }
    error = new Gfal::CoreException(scope, msg, ECANCELED);
}


GridFTPStreamState::GridFTPStreamState(GridFTPSessionHandler * s): GridFTPRequestState(s), offset(0), eof(false)
{
}


GridFTPStreamState::~GridFTPStreamState()
{
}


static
void gfal_stream_done_callback(void *user_arg,
        globus_ftp_client_handle_t *handle, globus_object_t *globus_error,
        globus_byte_t *buffer, globus_size_t length, globus_off_t offset,
        globus_bool_t eof, const char* err_msg_offset)
{
    GridFTPStreamState* state = static_cast<GridFTPStreamState*>(user_arg);

    globus_mutex_lock(&state->lock);
    if (globus_error != GLOBUS_SUCCESS) {
        char *err_buffer;
        int err_code = gfal_globus_error_convert(globus_error, &err_buffer);
        char err_static[128];
        g_strlcpy(err_static, err_buffer, sizeof(err_static));
        g_free(err_buffer);
        state->error = new Gfal::CoreException(Glib::Quark(""), err_static, err_code);
    }

    state->offset += length;
    state->eof = eof;

    state->done = true;
    globus_cond_signal(&state->cond);
    globus_mutex_unlock(&state->lock);
}


static
void gfal_griftp_stream_read_done_callback(void *user_arg,
        globus_ftp_client_handle_t *handle, globus_object_t *error,
        globus_byte_t *buffer, globus_size_t length, globus_off_t offset,
        globus_bool_t eof)
{

    gfal_stream_done_callback(user_arg, handle, error, buffer, length,
            offset, eof,
            " Invalid read callback call from globus, out of order");
}


static
void gfal_griftp_stream_write_done_callback(void *user_arg,
        globus_ftp_client_handle_t *handle, globus_object_t *error,
        globus_byte_t *buffer, globus_size_t length, globus_off_t offset,
        globus_bool_t eof)
{
    gfal_stream_done_callback(user_arg, handle, error, buffer, length,
            offset, eof,
            " Invalid write callback call from globus, out of order");
}


ssize_t gridftp_read_stream(const Glib::Quark & scope,
        GridFTPStreamState* stream, void* buffer, size_t s_read)
{
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gridftp_read_stream]");

    off_t initial_offset = stream->offset;

    if (stream->eof)
        return 0;
    stream->done = false;
    globus_result_t res = globus_ftp_client_register_read(
            stream->handler->get_ftp_client_handle(), (globus_byte_t*) buffer, s_read,
            gfal_griftp_stream_read_done_callback, stream);
    gfal_globus_check_result(scope, res);
    stream->wait(scope);
    return stream->offset - initial_offset;
}


ssize_t gridftp_write_stream(const Glib::Quark & scope,
        GridFTPStreamState* stream, const void* buffer, size_t s_write,
        bool eof)
{
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gridftp_write_stream]");
    off_t initial_offset = stream->offset;

    stream->done = false;
	globus_result_t res = globus_ftp_client_register_write( stream->handler->get_ftp_client_handle(),
		(globus_byte_t*) buffer,
		s_write,
		initial_offset,
		eof,
		gfal_griftp_stream_write_done_callback,
		stream
	);
	gfal_globus_check_result(scope, res);
	stream->wait(scope);
	return stream->offset - initial_offset;
}
