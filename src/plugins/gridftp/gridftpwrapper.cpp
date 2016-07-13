/*
 * Copyright (c) CERN 2013-2015
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
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

#include <memory>
#include <fstream>
#include <sstream>
#include <uri/gfal2_uri.h>
#include <exceptions/gfalcoreexception.hpp>
#include <globus_ftp_client_debug_plugin.h>
#include "gridftp_plugin.h"
#include "gridftpwrapper.h"
#include "gridftp_pasv_plugin.h"


struct RwStatus
{
    off_t init;
    off_t finish;
    bool ops_state;
};


static const GQuark GFAL_GRIDFTP_SCOPE_REQ_STATE = g_quark_from_static_string("GridFTPModule::RequestState");
static const GQuark GFAL_GRIDFTP_GASS_COPY_HANDLER = g_quark_from_static_string("GridFTPModule::GassCopyAttrHandler");
static const GQuark GFAL_GRIDFTP_SESSION = g_quark_from_static_string("GridFTPModule::GridFTPSession");
static const GQuark GFAL_GLOBUS_DONE_SCOPE = g_quark_from_static_string("GridFTPModule::Done");

static std::string gridftp_hostname_from_url(const std::string& url)
{
    GError *err = NULL;
    gfal2_uri *parsed = gfal2_parse_uri(url.c_str(), &err);
    if (err != NULL) {
        throw Gfal::CoreException(err);
    }
    char buffer[GFAL_URL_MAX_LEN];
    snprintf(buffer, sizeof(buffer), "%s://%s:%d", parsed->scheme, parsed->host, parsed->port);
    gfal2_free_uri(parsed);
    return std::string(buffer);
}


GassCopyAttrHandler::GassCopyAttrHandler(globus_ftp_client_operationattr_t* ftp_operation_attr)
{
    // initialize gass copy attr
    globus_result_t res = globus_gass_copy_attr_init(&(attr_gass));
    gfal_globus_check_result(GFAL_GRIDFTP_GASS_COPY_HANDLER, res);
    globus_ftp_client_operationattr_init(&(operation_attr_ftp_for_gass));
    globus_ftp_client_operationattr_copy(&(operation_attr_ftp_for_gass), ftp_operation_attr);
    res = globus_gass_copy_attr_set_ftp(&(attr_gass), &operation_attr_ftp_for_gass);
    gfal_globus_check_result(GFAL_GRIDFTP_GASS_COPY_HANDLER, res);

}


GassCopyAttrHandler::~GassCopyAttrHandler()
{
    globus_ftp_client_operationattr_destroy(&(operation_attr_ftp_for_gass));
}


GridFTPSessionHandler::GridFTPSessionHandler(GridFTPFactory* f, const std::string &uri) :
        factory(f), hostname(gridftp_hostname_from_url(uri))
{
    this->session = f->get_session(this->hostname);

    GridFTPRequestState req(this);
    globus_ftp_client_feat(&this->session->handle_ftp, (char*)uri.c_str(), &this->session->operation_attr_ftp,
                           &this->session->ftp_features, globus_ftp_client_done_callback, &req);
    req.wait(GFAL_GLOBUS_DONE_SCOPE);
}


GridFTPSessionHandler::~GridFTPSessionHandler()
{
    try {
        factory->release_session(this->session);
    }
    catch (const std::exception& e) {
        gfal2_log(G_LOG_LEVEL_MESSAGE,
                "Caught an exception inside ~GridFTP_session()!! %s", e.what());
    }
    catch (...) {
        gfal2_log(G_LOG_LEVEL_MESSAGE,
                "Caught an unknown exception inside ~GridFTP_session()!!");
    }
}


GridFTPSession::GridFTPSession(gfal2_context_t context, const std::string& hostname):
        hostname(hostname), pasv_plugin(NULL), context(context), params(NULL) 
{
    globus_result_t res;

    res = globus_ftp_client_debug_plugin_init(&debug_ftp_plugin, stderr, "gridftp debug :");
    gfal_globus_check_result(GFAL_GRIDFTP_SESSION, res);

    res = globus_ftp_client_operationattr_init(&operation_attr_ftp);
    gfal_globus_check_result(GFAL_GRIDFTP_SESSION, res);

    res = globus_ftp_client_handleattr_init(&attr_handle);
    gfal_globus_check_result(GFAL_GRIDFTP_SESSION, res);

    globus_ftp_client_handleattr_set_cache_all(&attr_handle, GLOBUS_TRUE); // enable session re-use
    if (getenv("GFAL2_GRIDFTP_DEBUG")) {
        globus_ftp_client_handleattr_add_plugin(&attr_handle, &debug_ftp_plugin);
    }

    gboolean register_pasv_plugin = gfal2_get_opt_boolean_with_default(context,
            GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_ENABLE_PASV_PLUGIN, FALSE);

    if (register_pasv_plugin) {
        res = gfal2_ftp_client_pasv_plugin_init(&pasv_plugin, this);
        gfal_globus_check_result(GFAL_GRIDFTP_SESSION, res);
        res = globus_ftp_client_handleattr_add_plugin(&attr_handle, &pasv_plugin);
        gfal_globus_check_result(GFAL_GRIDFTP_SESSION, res);
    }

    this->set_user_agent(context);

    res = globus_gass_copy_handleattr_init(&gass_handle_attr);
    gfal_globus_check_result(GFAL_GRIDFTP_SESSION, res);

    res = globus_gass_copy_handleattr_set_ftp_attr(&gass_handle_attr, &attr_handle);
    gfal_globus_check_result(GFAL_GRIDFTP_SESSION, res);

    res = globus_gass_copy_handle_init(&gass_handle, &gass_handle_attr);
    gfal_globus_check_result(GFAL_GRIDFTP_SESSION, res);

    int block_size = gfal2_get_opt_integer_with_default(context,
        GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_BLOCK_SIZE, 0);
    if (block_size > 0) {
        res = globus_gass_copy_set_buffer_length(&gass_handle, 0);
        gfal_globus_check_result(GFAL_GRIDFTP_SESSION, res);
    }

    res = globus_gass_copy_get_ftp_handle(&gass_handle, &handle_ftp);
    gfal_globus_check_result(GFAL_GRIDFTP_SESSION, res);

    res = globus_gass_copy_set_allocate(&gass_handle, GLOBUS_TRUE);
    gfal_globus_check_result(GFAL_GRIDFTP_SESSION, res);

    this->set_nb_streams(0);

    globus_ftp_client_features_init(&this->ftp_features);
}


GridFTPSession::~GridFTPSession()
{
    globus_ftp_client_debug_plugin_destroy(&debug_ftp_plugin);
    globus_gass_copy_handle_destroy(&gass_handle);
    globus_ftp_client_operationattr_destroy(&operation_attr_ftp);
    globus_gass_copy_handleattr_destroy(&gass_handle_attr);
    globus_ftp_client_handleattr_destroy(&attr_handle);
    globus_ftp_client_features_destroy(&this->ftp_features);
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


void GridFTPSession::set_user_agent(gfal2_context_t context)
{
    const char *agent, *version;
    gfal2_get_user_agent(context, &agent, &version);

    // Client information
    char* client_info = gfal2_get_client_info_string(context);

    if (agent) {
        std::ostringstream full_version;
        full_version << version << " (gfal2 " << gfal2_version() << ")";
        globus_ftp_client_handleattr_set_clientinfo(&attr_handle, agent, full_version.str().c_str(), client_info);
    }
    else {
        globus_ftp_client_handleattr_set_clientinfo(&attr_handle, "gfal2", gfal2_version(), client_info);
    }

    g_free(client_info);
}


void GridFTPSession::set_udt(bool udt)
{
    if (udt)
        globus_ftp_client_operationattr_set_net_stack(&operation_attr_ftp, "udt");
    else
        globus_ftp_client_operationattr_set_net_stack(&operation_attr_ftp, "default");
}


void gfal_globus_set_credentials(gfal2_context_t context, const char *url, globus_ftp_client_operationattr_t* opattr)
{
    // X509
    gchar* ucert = gfal2_get_opt_string(context, "X509", "CERT", NULL);
    gchar* ukey = gfal2_get_opt_string(context, "X509", "KEY", NULL);

    // User/password
    gchar *user = NULL, *passwd = NULL;
    if (strncmp(url, "ftp://", 6) == 0) {
        user = gfal2_get_opt_string_with_default(context, "FTP", "USER", "anonymous");
        passwd = gfal2_get_opt_string_with_default(context, "FTP", "PASSWORD", "anonymous");
    }

    // Set credentials
    if (ucert) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "GSIFTP using certificate %s", ucert);
    }
    if (ukey) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "GSIFTP using private key %s", ukey);
    }
    if (user) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "FTP using user %s", user);
    }

    gfal_globus_set_credentials(ucert, ukey, user, passwd, opattr);

    // Release memory
    g_free(ucert);
    g_free(ukey);
    g_free(user);
    g_free(passwd);
}


void gfal_globus_set_credentials(const char* ucert, const char* ukey,
    const char *user, const char *passwd,
    globus_ftp_client_operationattr_t* opattr)
{
    gss_cred_id_t cred_id = GSS_C_NO_CREDENTIAL;

    if (ucert) {
        std::stringstream buffer;
        std::ifstream cert_stream(ucert);
        if (!cert_stream.good()) {
            throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_REQ_STATE, errno,
                "Could not open the user certificate");
        }

        buffer << cert_stream.rdbuf();
        if (ukey && strcmp(ucert, ukey) != 0) {
            std::ifstream key_stream(ukey);
            if (key_stream.bad()) {
                throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_REQ_STATE, errno,
                    "Could not open the user private key");
            }
            buffer << key_stream.rdbuf();
        }

        gss_buffer_desc_struct buffer_desc;
        buffer_desc.value = g_strdup(buffer.str().c_str());
        buffer_desc.length = buffer.str().size();

        OM_uint32 minor_status, major_status;

        major_status = gss_import_cred(&minor_status, &cred_id,
            GSS_C_NO_OID, 0, // 0 = Pass credentials; 1 = Pass path as X509_USER_PROXY=...
            &buffer_desc, 0, NULL);
        g_free(buffer_desc.value);

        if (major_status != GSS_S_COMPLETE) {
            std::stringstream err_buffer;

            err_buffer << "Could not load the user credentials: ";

            globus_object_t *error = globus_error_get(major_status);
            char *globus_errstr;
            int globus_errno = gfal_globus_error_convert(error, &globus_errstr);
            if (globus_errstr) {
                err_buffer << globus_errstr;
                g_free(globus_errstr);
            }
            globus_object_free(error);

            err_buffer << " (" << globus_errno << ")";

            throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_REQ_STATE, globus_errno,
                err_buffer.str());
        }
    }
    
    globus_ftp_client_operationattr_set_authorization(
            opattr, cred_id, user, passwd, NULL, NULL);
}


globus_ftp_client_handle_t* GridFTPSessionHandler::get_ftp_client_handle()
{
    globus_result_t res = globus_gass_copy_get_ftp_handle(&(session->gass_handle),
            &(session->handle_ftp));
    gfal_globus_check_result(GFAL_GRIDFTP_SESSION, res);
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


globus_ftp_client_features_t* GridFTPSessionHandler::get_ftp_features()
{
    return (&session->ftp_features);
}


GridFTPFactory* GridFTPSessionHandler::get_factory()
{
    return factory;
}


GridFTPFactory::GridFTPFactory(gfal2_context_t handle) :
        gfal2_context(handle)
{
    GError * tmp_err = NULL;
    session_reuse = gfal2_get_opt_boolean(gfal2_context, GRIDFTP_CONFIG_GROUP,
            GRIDFTP_CONFIG_SESSION_REUSE, &tmp_err);
    gfal2_log(G_LOG_LEVEL_DEBUG, " define GSIFTP session re-use to %s",
            (session_reuse) ? "TRUE" : "FALSE");
    if (tmp_err) {
        throw Gfal::CoreException(tmp_err);
    }
    size_cache = 400;
    globus_mutex_init(&mux_cache, NULL);
}


void GridFTPFactory::clear_cache()
{
    globus_mutex_lock(&mux_cache);

    gfal2_log(G_LOG_LEVEL_DEBUG, "gridftp session cache garbage collection ...");
    std::multimap<std::string, GridFTPSession*>::iterator it;
    for (it = session_cache.begin(); it != session_cache.end(); ++it) {
        delete (*it).second;
    }
    session_cache.clear();
    globus_mutex_unlock(&mux_cache);
}


void GridFTPFactory::recycle_session(GridFTPSession* session)
{
    globus_mutex_lock(&mux_cache);

    if (session_cache.size() > size_cache)
        clear_cache();

    gfal2_log(G_LOG_LEVEL_DEBUG, "insert gridftp session for %s in cache ...", session->hostname.c_str());
    session_cache.insert(std::pair<std::string, GridFTPSession*>(session->hostname, session));
    globus_mutex_unlock(&mux_cache);
}


// recycle a gridftp session object from cache if exist, return NULL else
GridFTPSession* GridFTPFactory::get_recycled_handle(
        const std::string & hostname)
{
    globus_mutex_lock(&mux_cache);

    GridFTPSession* session = NULL;
    // try to find a session explicitly associated with this handle
    std::multimap<std::string, GridFTPSession*>::iterator it = session_cache.find(hostname);

    // if no session found, take a generic one
    if (it == session_cache.end()) {
        gfal2_log(G_LOG_LEVEL_DEBUG,
                "no session associated with this hostname, try find generic one .... ");
        it = session_cache.begin();
    }
    if (it != session_cache.end()) {
        gfal2_log(G_LOG_LEVEL_DEBUG,"gridftp session for: %s found in  cache !", hostname.c_str());
        session = (*it).second;
        session_cache.erase(it);
    }
    else {
        gfal2_log(G_LOG_LEVEL_DEBUG, "no session found in cache for %s!", hostname.c_str());
    }

    globus_mutex_unlock(&mux_cache);
    return session;
}


GridFTPFactory::~GridFTPFactory()
{
    try {
        clear_cache();
    }
    catch (const std::exception & e) {
        gfal2_log(G_LOG_LEVEL_MESSAGE,
                "Caught an exception inside ~GridFTPFactory()!! %s", e.what());
    }
    catch (...) {
        gfal2_log(G_LOG_LEVEL_MESSAGE,
                "Caught an unknown exception inside ~GridFTPFactory()!!");
    }
    globus_mutex_destroy(&mux_cache);
}


gfal2_context_t GridFTPFactory::get_gfal2_context()
{
    return gfal2_context;
}

/*
 *  dirty function to convert error code from globus
 *  In the current state, globus provides no way to convert gridftp error code to errno properly....
 * */
#ifndef ECOMM
#define ECOMM EIO
#endif
static int scan_errstring(const char *p) {
    int ret = ECOMM;
    if (p == NULL) return ret;

    if (strcasestr(p, "No such file") || strcasestr(p, "not found") || strcasestr(p, "error 3011"))
        ret = ENOENT;
    else if (strstr(p, "Permission denied") || strcasestr(p, "credential"))
        ret = EACCES;
    else if ( (strcasestr(p, "exists")) || strcasestr(p, "error 3006"))
        ret = EEXIST;
    else if (strcasestr(p, "Not a direct"))
		ret = ENOTDIR;
    else if (strcasestr(p, "Operation not supported"))
        ret = ENOTSUP;
    else if (strcasestr(p, "Login incorrect") || strcasestr(p, "Could not get virtual id"))
        ret = EACCES;
    else if (strcasestr(p, "the operation was aborted"))
        ret = ECANCELED;
    else if (strcasestr(p, "Is a directory"))
        ret = EISDIR;
    else if (strcasestr(p, "isk quota exceeded"))
        ret = ENOSPC;
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
    else {
        *str_error = NULL;
    }
    return 0;
}


static
void gfal_globus_check_error(GQuark scope, globus_object_t * error)
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
        throw Gfal::CoreException(scope, globus_errno, errbuff);
    }
}


void gfal_globus_check_result(GQuark scope, globus_result_t res)
{
    if (res != GLOBUS_SUCCESS) {

        globus_object_t * error = globus_error_get(res); // get error from result code
        if (error == NULL)
            throw Gfal::CoreException(scope, EINVAL,
                    "Unknown error: unable to map result code to globus error");
        gfal_globus_check_error(scope, error);
    }
}


GridFTPSession* GridFTPFactory::get_new_handle(const std::string &hostname)
{

    bool gridftp_v2 = gfal2_get_opt_boolean_with_default(gfal2_context, GRIDFTP_CONFIG_GROUP,
            GRIDFTP_CONFIG_V2, true);
    bool ipv6 = gfal2_get_opt_boolean_with_default(gfal2_context,
            GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_IPV6, false);
    bool delay_passv = gfal2_get_opt_boolean_with_default(gfal2_context,
            GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_DELAY_PASSV, true);
    bool dcau = gfal2_get_opt_boolean_with_default(gfal2_context, GRIDFTP_CONFIG_GROUP,
            GRIDFTP_CONFIG_DCAU, false);

    std::auto_ptr<GridFTPSession> session(new GridFTPSession(gfal2_context, hostname));

    session->set_gridftpv2(gridftp_v2);
    session->set_dcau(dcau);
    session->set_ipv6(ipv6);
    session->set_delayed_pass(delay_passv);

    gfal_globus_set_credentials(gfal2_context, hostname.c_str(), &session->operation_attr_ftp);

    return session.release();
}


GridFTPSession* GridFTPFactory::get_session(const std::string &hostname)
{
    GridFTPSession* session = NULL;
    if ((session = get_recycled_handle(hostname)) == NULL)
        session = get_new_handle(hostname);
    return session;
}


void GridFTPFactory::release_session(GridFTPSession* session)
{
    session_reuse = gfal2_get_opt_boolean_with_default(gfal2_context, GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_SESSION_REUSE, FALSE);
    if (session_reuse) {
        recycle_session(session);
    }
    else {
        gfal2_log(G_LOG_LEVEL_DEBUG, "destroy gridftp session for %s ...", session->hostname.c_str());
        delete session;
    }
}


static
void gfal_globus_done_callback(void* user_args,
        globus_object_t *globus_error)
{
    GridFTPRequestState* state = (GridFTPRequestState*) user_args;

    globus_mutex_lock(&state->mutex);
    if (globus_error != GLOBUS_SUCCESS) {
        char *err_buffer;
        int err_code = gfal_globus_error_convert(globus_error, &err_buffer);
        char err_static[2048];
        g_strlcpy(err_static, err_buffer, sizeof(err_static));
        g_free(err_buffer);
        state->error = new Gfal::CoreException(GFAL_GLOBUS_DONE_SCOPE, err_code, err_static);

        // Log complete error dump
        char *chain = globus_error_print_chain(globus_error);
        if (chain != NULL) {
            gfal2_log(G_LOG_LEVEL_DEBUG, chain);
            globus_free(chain);
        }   
    }
    state->done = true;
    globus_cond_signal(&state->cond);
    globus_mutex_unlock(&state->mutex);
}


// gridftp callback generic implementation
void globus_ftp_client_done_callback(void * user_arg,
        globus_ftp_client_handle_t * handle, globus_object_t * error)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, " gridFTP operation done");
    gfal_globus_done_callback(user_arg, error);
}

// gass operation callback implementation
void globus_gass_client_done_callback(void * callback_arg,
        globus_gass_copy_handle_t * handle, globus_object_t * error)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, "gass operation done");
    gfal_globus_done_callback(callback_arg, error);
}


GridFTPRequestState::GridFTPRequestState(GridFTPSessionHandler* s,
        GridFTPRequestType request_type) :
        handler(s), request_type(request_type), error(NULL), done(false)
{
    this->default_timeout = gfal2_get_opt_integer_with_default(
            s->get_factory()->get_gfal2_context(), GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_OP_TIMEOUT, 300);
    globus_mutex_init(&mutex, NULL);
    globus_cond_init(&cond, NULL);
}


GridFTPRequestState::~GridFTPRequestState()
{
    if (!done) {
        this->cancel(GFAL_GRIDFTP_SCOPE_REQ_STATE,
                "GridFTPRequestState destructor called before the operation finished!");
    }
    globus_mutex_destroy(&mutex);
    globus_cond_destroy(&cond);
    delete error;
}


static
void gridftp_cancel(gfal2_context_t context, void* userdata)
{
    GridFTPRequestState* state = (GridFTPRequestState*)userdata;
    state->cancel(gfal_cancel_quark(), "Operation canceled from gfal2_cancel");
}


static int callback_cond_wait(GridFTPRequestState* req, time_t timeout)
{
    globus_abstime_t timeout_expires;
    GlobusTimeAbstimeGetCurrent(timeout_expires);
    timeout_expires.tv_sec += timeout;

    globus_mutex_lock(&req->mutex);
    int wait_ret = 0;
    while (!req->done && wait_ret != ETIMEDOUT) {
        wait_ret = globus_cond_timedwait(&req->cond, &req->mutex, &timeout_expires);
    }
    globus_mutex_unlock(&req->mutex);
    return wait_ret;
}


void GridFTPRequestState::wait(GQuark scope, time_t timeout)
{
    if (timeout < 0)
        timeout = default_timeout;

    gfal2_log(G_LOG_LEVEL_DEBUG,
            "   [GridFTP_Request_state::wait_callback] setup gsiftp timeout to %ld seconds",
            timeout);

    gfal_cancel_token_t cancel_token;
    cancel_token = gfal2_register_cancel_callback(handler->get_factory()->get_gfal2_context(), gridftp_cancel, this);

    int wait_ret = callback_cond_wait(this, timeout);

    gfal2_remove_cancel_callback(handler->get_factory()->get_gfal2_context(), cancel_token);

    // Operation expired, so cancel and raise an error
    if (wait_ret == ETIMEDOUT) {
        gfal2_log(G_LOG_LEVEL_DEBUG,
                "   [GridFTP_Request_state::wait_callback] Operation timeout of %d seconds expired, canceling...",
                timeout);
        gridftp_cancel(handler->get_factory()->get_gfal2_context(), this);

        // Wait again for the callback, ignoring timeout this time
        callback_cond_wait(this, timeout);

        throw Gfal::CoreException(scope, ETIMEDOUT, "Operation timed out");
    }

    if (error) {
        if (error->domain() != 0)
            throw Gfal::CoreException(scope, error->code(), error->what());
        else
            throw *error;
    }
}


void GridFTPRequestState::cancel(GQuark scope, const std::string& msg)
{
    if (request_type == GRIDFTP_REQUEST_FTP) {
        globus_ftp_client_abort(handler->get_ftp_client_handle());
    }
    else {
        globus_gass_copy_cancel(handler->get_gass_copy_handle(),
                globus_gass_client_done_callback, this);
    }
    error = new Gfal::CoreException(scope, ECANCELED, msg);
}


GridFTPStreamState::GridFTPStreamState(GridFTPSessionHandler * s):
        GridFTPRequestState(s), offset(0), buffer_size(0), eof(false), expect_eof(false)
{
}


GridFTPStreamState::~GridFTPStreamState()
{
}


static
void gfal_stream_done_callback_err_handling(GridFTPStreamState* state,
        globus_ftp_client_handle_t *handle, globus_object_t *globus_error,
        globus_byte_t *buffer, globus_size_t length, globus_off_t offset,
        globus_bool_t eof)
{
    if (globus_error != GLOBUS_SUCCESS) {
        char *err_buffer;
        int err_code = gfal_globus_error_convert(globus_error, &err_buffer);
        char err_static[2048];
        g_strlcpy(err_static, err_buffer, sizeof(err_static));
        g_free(err_buffer);
        state->error = new Gfal::CoreException(GFAL_GLOBUS_DONE_SCOPE, err_code, err_static);
    }

    state->offset += length;
    state->eof = eof;
}


static
void gfal_griftp_stream_read_done_callback(void *user_arg,
        globus_ftp_client_handle_t *handle, globus_object_t *error,
        globus_byte_t *buffer, globus_size_t length, globus_off_t offset,
        globus_bool_t eof)
{
    GridFTPStreamState* state = static_cast<GridFTPStreamState*>(user_arg);
    globus_mutex_lock(&state->mutex);

    gfal_stream_done_callback_err_handling(state, handle, error, buffer, length,
            offset, eof);

    if (!state->expect_eof || eof) {
        state->done = true;
        globus_cond_signal(&state->cond);
    }
    else {
        // It may happen that the buffer size and the requested size are of
        // the same size, and there is enough data to fill it.
        // If that's the case, a second callback will be done with EOF, and we need
        // to get it, or waiting for the operation completion will block forever
        globus_ftp_client_register_read(
                            handle,
                            buffer,
                            state->buffer_size,
                            gfal_griftp_stream_read_done_callback,
                            state);
    }

    globus_mutex_unlock(&state->mutex);
}


static
void gfal_griftp_stream_write_done_callback(void *user_arg,
        globus_ftp_client_handle_t *handle, globus_object_t *error,
        globus_byte_t *buffer, globus_size_t length, globus_off_t offset,
        globus_bool_t eof)
{
    GridFTPStreamState* state = static_cast<GridFTPStreamState*>(user_arg);
    globus_mutex_lock(&state->mutex);

    gfal_stream_done_callback_err_handling(state, handle, error, buffer, length,
            offset, eof);

    state->done = true;

    globus_cond_signal(&state->cond);
    globus_mutex_unlock(&state->mutex);
}


ssize_t gridftp_read_stream(GQuark scope,
        GridFTPStreamState* stream, void* buffer, size_t s_read, bool expect_eof)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, "  -> [gridftp_read_stream]");

    off_t initial_offset = stream->offset;

    if (stream->eof)
        return 0;
    stream->done = false;
    stream->buffer_size = s_read;
    stream->expect_eof = expect_eof;
    globus_result_t res = globus_ftp_client_register_read(
            stream->handler->get_ftp_client_handle(),
            (globus_byte_t*) buffer,
            s_read,
            gfal_griftp_stream_read_done_callback,
            stream);
    gfal_globus_check_result(scope, res);
    stream->wait(scope);
    return stream->offset - initial_offset;
}


ssize_t gridftp_write_stream(GQuark scope,
        GridFTPStreamState* stream, const void* buffer, size_t s_write,
        bool eof)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, "  -> [gridftp_write_stream]");
    off_t initial_offset = stream->offset;

    stream->done = false;
	globus_result_t res = globus_ftp_client_register_write(
	    stream->handler->get_ftp_client_handle(),
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
