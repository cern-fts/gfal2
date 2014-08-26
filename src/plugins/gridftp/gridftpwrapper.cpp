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
#include <uri/uri_util.h>
#include <config/gfal_config.h>
#include <cancel/gfal_cancel.h>
#include <exceptions/gfalcoreexception.hpp>
#include "gridftpwrapper.h"

#include <globus_ftp_client_debug_plugin.h>

const char* gridftp_version_config = "GRIDFTP_V2";
const char* gridftp_session_reuse_config = "SESSION_REUSE";
const char* gridftp_timeout = "OPERATION_TIMEOUT";
const char* gridftp_dcau_config = "DCAU";
const char* gridftp_ipv6_config = "IPV6";
const char* gridftp_delay_passv_config = "DELAY_PASSV";

struct RwStatus
{
    off_t init;
    off_t finish;
    bool ops_state;
};


static Glib::Quark GFAL_GRIDFTP_SCOPE_REQ_STATE("GridFTPModule::RequestState");


Gass_attr_handler::Gass_attr_handler(globus_ftp_client_operationattr_t* ftp_operation_attr)
{
    // initialize gass copy attr
    globus_result_t res = globus_gass_copy_attr_init(&(attr_gass));
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);
    globus_ftp_client_operationattr_init(&(operation_attr_ftp_for_gass));
    globus_ftp_client_operationattr_copy(&(operation_attr_ftp_for_gass), ftp_operation_attr);
    res = globus_gass_copy_attr_set_ftp(&(attr_gass), &operation_attr_ftp_for_gass);
    gfal_globus_check_result("GridFTPFactory::globus_gass_copy_handleattr_set_ftp_attr", res);

}


Gass_attr_handler::~Gass_attr_handler()
{
    // initialize gass copy attr
    globus_ftp_client_operationattr_destroy(&(operation_attr_ftp_for_gass));
}


GridFTPSession::GridFTPSession(GridFTPFactory* f, const std::string & thostname) :
        _isDirty(false), factory(f), hostname(thostname)
{
    init();
}


GridFTPSession::GridFTPSession(GridFTPSession *src) :
        _isDirty(false), factory(src->factory), hostname(src->hostname), _sess(src->_sess)
{
}


GridFTPSession::~GridFTPSession()
{
    try {
        if (_sess != NULL) {
            clean();
            if (_isDirty)
                this->purge();
            else
                factory->gfal_globus_ftp_release_handle_internal(this);
        }
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


void GridFTPSession::init()
{
    _sess = new Session_handler();
    globus_result_t res;

    // init debug plugin
    res = globus_ftp_client_debug_plugin_init(&(_sess->debug_ftp_plugin),
            stderr, "gridftp debug :");
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_ops_attr",
            res);

    // init operation attr
    res = globus_ftp_client_operationattr_init(&(_sess->operation_attr_ftp));
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_ops_attr",
            res);

    // initialize ftp attributes
    res = globus_ftp_client_handleattr_init(&(_sess->attr_handle));
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle_attr",
            res);
    configure_gridftp_handle_attr();

    // create gass handle attribute
    res = globus_gass_copy_handleattr_init(&(_sess->gass_handle_attr));
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle",
            res);

    // associate ftp attributes to gass attributes
    res = globus_gass_copy_handleattr_set_ftp_attr(&(_sess->gass_handle_attr),
            &(_sess->attr_handle));
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle",
            res);

    // initialize gass handle
    res = globus_gass_copy_handle_init(&(_sess->gass_handle),
            &(_sess->gass_handle_attr));
    gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle",
            res);

    configure_default_stream_attributes();
    apply_default_stream_attribute();
}


void GridFTPSession::configure_gridftp_handle_attr()
{
    globus_ftp_client_handleattr_set_cache_all(&(_sess->attr_handle),
            GLOBUS_TRUE);	// enable session re-use
    if (gfal_get_verbose() & GFAL_VERBOSE_TRACE_PLUGIN) {
        globus_ftp_client_handleattr_add_plugin(&(_sess->attr_handle),
                &(_sess->debug_ftp_plugin));
    }
}


void GridFTPSession::configure_default_stream_attributes()
{
    _sess->parallelism.fixed.size = 1;
    _sess->parallelism.mode = GLOBUS_FTP_CONTROL_PARALLELISM_NONE;
    _sess->mode = GLOBUS_FTP_CONTROL_MODE_NONE;
}


void GridFTPSession::apply_default_stream_attribute()
{
    globus_ftp_client_operationattr_set_mode(&(_sess->operation_attr_ftp),
            _sess->mode);
    globus_ftp_client_operationattr_set_parallelism(
            &(_sess->operation_attr_ftp), &(_sess->parallelism));
}


void GridFTPSession::apply_default_tcp_buffer_attributes()
{
    globus_ftp_client_operationattr_set_tcp_buffer(&(_sess->operation_attr_ftp),
            &(_sess->tcp_buffer_size));
}


void GridFTPSession::set_gridftpv2(bool v2)
{
    globus_ftp_client_handleattr_set_gridftp2(&(_sess->attr_handle), v2); // define GridFTP 2
}


void GridFTPSession::set_ipv6(bool enable)
{
    globus_ftp_client_operationattr_set_allow_ipv6(&(_sess->operation_attr_ftp),
            (globus_bool_t) enable);
}


void GridFTPSession::set_delayed_pass(bool enable)
{
    globus_ftp_client_operationattr_set_delayed_pasv(
            &(_sess->operation_attr_ftp),
            (globus_bool_t) (enable) ? GLOBUS_TRUE : GLOBUS_FALSE);
}


void GridFTPSession::set_dcau(const globus_ftp_control_dcau_t & _dcau)
{
    _sess->dcau.mode = _dcau.mode;
    globus_ftp_client_operationattr_set_dcau(&(_sess->operation_attr_ftp),
            &(_sess->dcau));
}


void GridFTPSession::set_nb_stream(const unsigned int nbstream)
{
    if (nbstream == 0) {
        configure_default_stream_attributes();
    }
    else {
        _sess->parallelism.fixed.size = nbstream;
        _sess->parallelism.mode = GLOBUS_FTP_CONTROL_PARALLELISM_FIXED;
        _sess->mode = GLOBUS_FTP_CONTROL_MODE_EXTENDED_BLOCK;
    }
    apply_default_stream_attribute();
}


void GridFTPSession::set_tcp_buffer_size(const guint64 tcp_buffer_size)
{
    if (tcp_buffer_size == 0) {
        _sess->tcp_buffer_size.mode = GLOBUS_FTP_CONTROL_TCPBUFFER_DEFAULT;
    }
    else {
        _sess->tcp_buffer_size.mode = GLOBUS_FTP_CONTROL_TCPBUFFER_FIXED;
        _sess->tcp_buffer_size.fixed.size = tcp_buffer_size;
    }
    apply_default_tcp_buffer_attributes();
}


void GridFTPSession::enable_udt()
{
    globus_ftp_client_operationattr_set_net_stack(&(_sess->operation_attr_ftp),
            "udt");
}


void GridFTPSession::disable_udt()
{
    globus_ftp_client_operationattr_set_net_stack(&(_sess->operation_attr_ftp),
            "default");
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
    if (cert_stream.bad()) {
        throw Glib::Error(GFAL_GRIDFTP_SCOPE_REQ_STATE, errno,
                "Could not open the user certificate");
    }

    buffer << cert_stream.rdbuf();
    if (ukey) {
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
        err_buffer << "Could not load the user credentials (" << major_status
                << ":" << minor_status << ")";
        throw Glib::Error(GFAL_GRIDFTP_SCOPE_REQ_STATE, EINVAL,
                err_buffer.str());
    }
    globus_ftp_client_operationattr_set_authorization(
            opattr, cred_id, NULL, NULL, NULL, NULL);
}


globus_ftp_client_handle_t* GridFTPSession::get_ftp_handle()
{
    globus_result_t res = globus_gass_copy_get_ftp_handle(&(_sess->gass_handle),
            &(_sess->handle_ftp));
    gfal_globus_check_result("GridFTPFactory::GridFTP_session", res);
    return &(_sess->handle_ftp);
}


globus_gass_copy_handle_t* GridFTPSession::get_gass_handle()
{
    return &(_sess->gass_handle);
}


globus_ftp_client_operationattr_t* GridFTPSession::get_op_attr_ftp()
{
    return &(_sess->operation_attr_ftp);
}


globus_gass_copy_handleattr_t* GridFTPSession::get_gass_handle_attr()
{
    return &(_sess->gass_handle_attr);
}

globus_ftp_client_handleattr_t* GridFTPSession::get_ftp_handle_attr()
{
    return &(_sess->attr_handle);
}

GridFTPFactory* GridFTPSession::get_factory()
{
    return factory;
}

void GridFTPSession::clean()
{
    // clean performance markers
    globus_result_t res = globus_gass_copy_register_performance_cb(
            &(_sess->gass_handle),
            NULL, NULL);
    gfal_globus_check_result("GridFTPFactory::GridFTP_session", res);
    configure_default_stream_attributes();
}


void GridFTPSession::purge()
{
    globus_ftp_client_debug_plugin_destroy(&(_sess->debug_ftp_plugin)); // destruct the debug plugin
    globus_gass_copy_handle_destroy(&(_sess->gass_handle));
    globus_ftp_client_operationattr_destroy(&(_sess->operation_attr_ftp));
    globus_gass_copy_handleattr_destroy(&(_sess->gass_handle_attr));
    globus_ftp_client_handleattr_destroy(&(_sess->attr_handle));
    delete _sess;
    _sess = NULL;
}


void GridFTPSession::disable_reuse()
{
    _isDirty = true;
}


GridFTPFactory::GridFTPFactory(gfal2_context_t handle) :
        _handle(handle)
{
    GError * tmp_err = NULL;
    session_reuse = gfal2_get_opt_boolean(_handle, GRIDFTP_CONFIG_GROUP,
            gridftp_session_reuse_config, &tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, " define GSIFTP session re-use to %s",
            (session_reuse) ? "TRUE" : "FALSE");
    if (tmp_err)
        throw Glib::Error(tmp_err);
    size_cache = 400;
}


GridFTPRequestState::GridFTPRequestState(GridFTPSession * s, bool own_session,
        GridFTPRequestType request_type) :
        errcode(0), cancel_token(NULL), sess(s), end_time(0, 0)
{
    req_status = GRIDFTP_REQUEST_NOT_LAUNCHED;
    this->own_session = own_session;
    this->request_type = request_type;
    this->default_timeout = gfal2_get_opt_integer_with_default(
            s->get_factory()->get_handle(), GRIDFTP_CONFIG_GROUP, gridftp_timeout, 300);
    canceling = false;
}


GridFTPRequestState::~GridFTPRequestState()
{
    try {
        if (req_status == GRIDFTP_REQUEST_RUNNING)
            cancel_operation(GFAL_GRIDFTP_SCOPE_REQ_STATE,
                    "ReqState Destroyer");
        Glib::RWLock::WriterLock l(mux_req_state);
        if (!own_session)
            sess.release(); // cancel the automatic memory management
    }
    catch (const std::exception& e) {
        gfal_log(GFAL_VERBOSE_NORMAL,
                "Caught an exception inside ~GridFTP_Request_state()!! %s",
                e.what());
    }
    catch (...) {
        gfal_log(GFAL_VERBOSE_NORMAL,
                "Caught an unknown exception inside ~GridFTP_Request_state()!!");
    }
}


static void gfal_gridftp_cancel_slot(gfal2_context_t context, void* userdata)
{
    GridFTPRequestState* r = (GridFTPRequestState*) userdata;
    r->cancel_operation_async(g_quark_to_string(gfal_cancel_quark()),
            "Operation canceled from gfal2_cancel");
}


GridFTPOperationCanceler::GridFTPOperationCanceler(gfal2_context_t context,
        GridFTPRequestState *state) :
        _state(state), _cancel_token(
                gfal2_register_cancel_callback(context,
                        &gfal_gridftp_cancel_slot, state)), _context(context)
{
}


GridFTPOperationCanceler::~GridFTPOperationCanceler()
{
    gfal2_remove_cancel_callback(_context, _cancel_token);
}


void GridFTPFactory::clear_cache()
{
    gfal_log(GFAL_VERBOSE_TRACE,
            "gridftp session cache garbage collection ...");
    std::multimap<std::string, GridFTPSession*>::iterator it;
    for (it = sess_cache.begin(); it != sess_cache.end(); ++it) {
        GridFTPSession *sess = static_cast<GridFTPSession *>((*it).second);
        sess->purge();
        delete sess;
    }
    sess_cache.clear();
}


void GridFTPFactory::recycle_session(GridFTPSession* sess)
{
    Glib::Mutex::Lock l(mux_cache);

    GridFTPSession * my_sess = static_cast<GridFTPSession *>(sess);
    const char* c_hostname = my_sess->hostname.c_str();

    if (sess_cache.size() > size_cache)
        clear_cache();

    gfal_log(GFAL_VERBOSE_TRACE, "insert gridftp session for %s in cache ...",
            c_hostname);
    sess_cache.insert(
            std::pair<std::string, GridFTPSession*>(c_hostname,
                    new GridFTPSession(my_sess)));
}


// recycle a gridftp session object from cache if exist, return NULL else
GridFTPSession* GridFTPFactory::get_recycled_handle(
        const std::string & hostname)
{
    Glib::Mutex::Lock l(mux_cache);
    GridFTPSession* res = NULL;
    std::multimap<std::string, GridFTPSession*>::iterator it = sess_cache.find(
            hostname); // try to find a session explicitly associated with this handle
    if (it == sess_cache.end()) { // if no session found, take a generic one
        gfal_log(GFAL_VERBOSE_TRACE,
                "no session associated with this hostname, try find generic one .... ");
        it = sess_cache.begin();
    }
    if (it != sess_cache.end()) {
        gfal_log(GFAL_VERBOSE_TRACE,
                "gridftp session for: %s found in  cache !", hostname.c_str());
        res = (*it).second;
        sess_cache.erase(it);
    }
    else {
        gfal_log(GFAL_VERBOSE_TRACE, "no session found in cache for %s!",
                hostname.c_str());
    }
    return res;
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


gfal2_context_t GridFTPFactory::get_handle()
{
    return _handle;
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


void gfal_globus_check_result(const Glib::Quark & scope, globus_result_t res)
{
    if (res != GLOBUS_SUCCESS) {

        globus_object_t * error = globus_error_get(res); // get error from result code
        if (error == NULL)
            throw Gfal::CoreException(scope,
                    "Unknown error  unable to map result code to globus error",
                    ENOENT);

        gfal_globus_check_error(scope, error);
    }
}


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


GridFTPSession* GridFTPFactory::get_new_handle(const std::string & hostname)
{
    GError * tmp_err = NULL;
    globus_ftp_control_dcau_t dcau_param;
    bool gridftp_v2 = gfal2_get_opt_boolean(_handle, GRIDFTP_CONFIG_GROUP,
            gridftp_version_config, &tmp_err);
    if (tmp_err)
        throw Glib::Error(tmp_err);

    const bool ipv6 = gfal2_get_opt_boolean_with_default(_handle,
            GRIDFTP_CONFIG_GROUP, gridftp_ipv6_config, false);
    const bool delay_passv = gfal2_get_opt_boolean_with_default(_handle,
            GRIDFTP_CONFIG_GROUP, gridftp_delay_passv_config, true);

    dcau_param.mode =
            (gfal2_get_opt_boolean(_handle, GRIDFTP_CONFIG_GROUP,
                    gridftp_dcau_config, &tmp_err)) ?
                    GLOBUS_FTP_CONTROL_DCAU_DEFAULT :
                    GLOBUS_FTP_CONTROL_DCAU_NONE;
    if (tmp_err)
        throw Glib::Error(tmp_err);

    std::auto_ptr<GridFTPSession> sess(new GridFTPSession(this, hostname));

    sess->set_gridftpv2(gridftp_v2);
    sess->set_dcau(dcau_param);
    sess->set_ipv6(ipv6);
    sess->set_delayed_pass(delay_passv);

    gfal_globus_set_credentials(_handle, &sess->_sess->operation_attr_ftp);

    return sess.release();
}


// store the related globus error to the current handle
void gfal_globus_store_error(GridFTPRequestState * state,
        globus_object_t *error)
{
    char * glob_str = NULL;
    state->set_error_code(gfal_globus_error_convert(error, &glob_str));
    if (glob_str) {
        if (state->get_error().empty())
            state->set_error(glob_str);
        g_free(glob_str);
    }
    else {
        state->set_error("Unknown Globus Error, bad error report");
        state->set_error_code(EFAULT);
    }
}


GridFTPSession* GridFTPFactory::gfal_globus_ftp_take_handle(
        const std::string & hostname)
{
    GridFTPSession * res = NULL;
    if ((res = get_recycled_handle(hostname)) == NULL)
        res = get_new_handle(hostname);
    return res;
}


void GridFTPFactory::gfal_globus_ftp_release_handle_internal(
        GridFTPSession* sess)
{
    session_reuse = gfal2_get_opt_boolean_with_default(_handle,
            GRIDFTP_CONFIG_GROUP, gridftp_session_reuse_config, FALSE);
    if (session_reuse)
        recycle_session(sess);
    else {
        GridFTPSession * s = static_cast<GridFTPSession *>(sess);
        s->purge();
    }
}


void GridFTPFactory::gfal_globus_ftp_release_handle(GridFTPSession* h)
{
    delete h;
}


static
void gfal_globus_prototype_callback(void* user_args,
        globus_object_t *error)
{
    GridFTPRequestState* state = (GridFTPRequestState*) user_args;
    Glib::RWLock::ReaderLock l(state->mux_req_state);
    Glib::Mutex::Lock l_call(state->mux_callback_lock);

    if (state->get_req_status() == GRIDFTP_REQUEST_FINISHED) {
        gfal_log(GFAL_VERBOSE_TRACE,
                "gridFTP operation already finished ! error !");
    }
    else {
        if (error != GLOBUS_SUCCESS) {
            gfal_globus_store_error(state, error);
        }
        else if (state->canceling == FALSE) {
            state->set_error_code(0);
        }
        state->set_req_status(GRIDFTP_REQUEST_FINISHED);
        state->signal_callback_main.broadcast();
    }
}

// gridftp callback generic implementation
void globus_basic_client_callback(void * user_arg,
        globus_ftp_client_handle_t * handle, globus_object_t * error)
{
    gfal_log(GFAL_VERBOSE_TRACE, " gridFTP operation done");
    gfal_globus_prototype_callback(user_arg, error);
}

// gass operation callback implementation
void globus_gass_basic_client_callback(void * callback_arg,
        globus_gass_copy_handle_t * handle, globus_object_t * error)
{
    gfal_log(GFAL_VERBOSE_TRACE, "gass operation done");
    gfal_globus_prototype_callback(callback_arg, error);
}


void GridFTPRequestState::poll_callback(const Glib::Quark &scope)
{
    gfal_log(GFAL_VERBOSE_TRACE, " -> go internal polling for request ");
    bool timeout = false;
    Glib::RWLock::ReaderLock l(mux_req_state);
    {

        Glib::Mutex::Lock l(mux_callback_lock);
        // wait for a globus signal or for a timeout
        // if canceling logic -> wait until end
        while (this->req_status != GRIDFTP_REQUEST_FINISHED
                && (timeout == FALSE || this->canceling == TRUE)) {
            if (end_time == Glib::TimeVal(0, 0) || this->canceling == TRUE) {
                signal_callback_main.wait(mux_callback_lock);
            }
            else {
                timeout = !(signal_callback_main.timed_wait(mux_callback_lock,
                        end_time));
            }
        }
    }

    if (timeout && this->canceling == FALSE) {
        gfal_log(GFAL_VERBOSE_TRACE,
                "gfal gridftp operation timeout occurred ! cancel the operation ...");
        cancel_operation(scope,
                "gfal gridftp internal operation timeout, operation canceled");
        this->set_error_code(ETIMEDOUT);
    }

    gfal_log(GFAL_VERBOSE_TRACE, " <- out of gass polling for request ");
}


void GridFTPRequestState::err_report(const Glib::Quark &scope)
{
    if (this->get_error_code() != 0)
        throw Gfal::CoreException(scope, this->get_error(),
                this->get_error_code());
}


void GridFTPRequestState::wait_callback(const Glib::Quark &scope,
        time_t timeout)
{
    struct timespec st_timeout;

    st_timeout.tv_nsec = 0;
    if (timeout >= 0)
        st_timeout.tv_sec = timeout;
    else
        st_timeout.tv_sec = default_timeout;

    gfal_log(GFAL_VERBOSE_TRACE,
            "   [GridFTP_Request_state::wait_callback] setup gsiftp timeout to %ld seconds",
            timeout);

    init_timeout(&st_timeout);
    poll_callback(scope);
    err_report(scope);
}


void GridFTPRequestState::cancel_operation(const Glib::Quark &scope,
        const std::string &msg)
{
    if (cancel_operation_async(scope, msg) == 0)
        this->poll_callback(scope);

}


int GridFTPRequestState::cancel_operation_async(const Glib::Quark &scope,
        const std::string & msg)
{
    globus_result_t res;
    int ret = 0;
    Glib::RWLock::ReaderLock l(this->mux_req_state);
    Glib::Mutex::Lock l_call(this->mux_callback_lock);
    this->canceling = TRUE;
    if (this->get_req_status() == GRIDFTP_REQUEST_FINISHED) // already finished before canceling -> return
        return 0;

    if (this->request_type == GRIDFTP_REQUEST_GASS) {
        gfal_log(GFAL_VERBOSE_TRACE, " -> gass operation cancel  ");
        res = globus_gass_copy_cancel(this->sess->get_gass_handle(),
                globus_gass_basic_client_callback, this);
        gfal_log(GFAL_VERBOSE_TRACE, "    gass operation cancel <-");
    }
    else {
        res = globus_ftp_client_abort(this->sess->get_ftp_handle());
    }
    try {
        gfal_globus_check_result(scope, res);
    }
    catch (Gfal::CoreException & e) {
        gfal_log(GFAL_VERBOSE_TRACE, "gridftp error triggered while cancel: %s",
                e.message_only());
        this->sess->disable_reuse();
        ret = -1;
    }
    catch (...) {
        gfal_log(GFAL_VERBOSE_TRACE,
                "gridftp error triggered while cancel: Unknown");
        this->sess->disable_reuse();
        ret = -1;
    }

    this->set_error_code(ECANCELED);
    this->set_error(msg);
    return ret;
}


void GridFTPStreamState::poll_callback_stream(const Glib::Quark & scope)
{
    gfal_log(GFAL_VERBOSE_TRACE, " -> go polling for request ");
    {

        Glib::Mutex::Lock l(mux_stream_callback);
        // wait for a globus signal or for a timeout
        // if canceling logic -> wait until end
        while (this->get_stream_status() != GRIDFTP_REQUEST_FINISHED) {
            cond_stream_callback.wait(mux_stream_callback);
        }
    }
    while (this->stream_status != GRIDFTP_REQUEST_FINISHED)
        usleep(10);
    gfal_log(GFAL_VERBOSE_TRACE, " <- out of polling for request ");
}


void GridFTPStreamState::wait_callback_stream(const Glib::Quark & scope)
{
    poll_callback_stream(scope);
    err_report(scope);
}


GridFTPStreamState::~GridFTPStreamState()
{
    try {
        if (req_status == GRIDFTP_REQUEST_RUNNING) {
            cancel_operation(GFAL_GRIDFTP_SCOPE_REQ_STATE,
                    "ReqStream Destroyer");
            poll_callback(GFAL_GRIDFTP_SCOPE_REQ_STATE);
        }
        while (this->stream_status == GRIDFTP_REQUEST_RUNNING)
            usleep(1);
    }
    catch (const std::exception & e) {
        gfal_log(GFAL_VERBOSE_NORMAL,
                "Caught an exception inside ~GridFTP_stream_state()!! %s",
                e.what());
    }
    catch (...) {
        gfal_log(GFAL_VERBOSE_NORMAL,
                "Caught an unknown exception inside ~GridFTP_stream_state()!!");
    }
}


void gridftp_wait_for_read(const Glib::Quark & scope, GridFTPStreamState* state,
        off_t end_read)
{
    state->wait_callback_stream(scope);
}


void gridftp_wait_for_write(const Glib::Quark & scope,
        GridFTPStreamState* state, off_t end_write)
{
    state->wait_callback_stream(scope);
}



void gfal_stream_callback_prototype(void *user_arg,
        globus_ftp_client_handle_t *handle, globus_object_t *error,
        globus_byte_t *buffer, globus_size_t length, globus_off_t offset,
        globus_bool_t eof, const char* err_msg_offset)
{
    GridFTPStreamState* state = static_cast<GridFTPStreamState*>(user_arg);
    Glib::Mutex::Lock l(state->mux_stream_callback);

    if (error != GLOBUS_SUCCESS) {	// check error status
        gfal_globus_store_error(state, error);
        // gfal_log(GFAL_VERBOSE_TRACE," read error %s , code %d", state->error, state->errcode);
    }
    else {
        // verify read
        //gfal_log(GFAL_VERBOSE_TRACE," read %d bytes , eof %d %d,%d", length, eof, state->offset, offset);
        if (state->get_offset() != offset) {
            state->set_error(err_msg_offset);
            state->set_error_code(EIO);
        }
        else {
            state->increase_offset(length);
            state->set_eof(eof);
            state->set_error_code(0);
        }
    }
    state->set_stream_status(GRIDFTP_REQUEST_FINISHED);
    state->cond_stream_callback.broadcast();
}


void gfal_griftp_stream_read_callback(void *user_arg,
        globus_ftp_client_handle_t *handle, globus_object_t *error,
        globus_byte_t *buffer, globus_size_t length, globus_off_t offset,
        globus_bool_t eof)
{

    gfal_stream_callback_prototype(user_arg, handle, error, buffer, length,
            offset, eof,
            " Invalid read callback call from globus, out of order");
}


static void gfal_griftp_stream_write_callback(void *user_arg,
        globus_ftp_client_handle_t *handle, globus_object_t *error,
        globus_byte_t *buffer, globus_size_t length, globus_off_t offset,
        globus_bool_t eof)
{
    gfal_stream_callback_prototype(user_arg, handle, error, buffer, length,
            offset, eof,
            " Invalid write callback call from globus, out of order");
}


ssize_t gridftp_read_stream(const Glib::Quark & scope,
        GridFTPStreamState* stream, void* buffer, size_t s_read)
{
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gridftp_read_stream]");
    off_t initial_offset = stream->get_offset();

    if (stream->is_eof())
        return 0;
    stream->set_stream_status(GRIDFTP_REQUEST_RUNNING);
    globus_result_t res = globus_ftp_client_register_read(
            stream->sess->get_ftp_handle(), (globus_byte_t*) buffer, s_read,
            gfal_griftp_stream_read_callback, stream);
    gfal_globus_check_result(scope, res);
    gridftp_wait_for_read(scope, stream, initial_offset + s_read);
    stream->set_stream_status(GRIDFTP_REQUEST_NOT_LAUNCHED);
    return stream->get_offset() - initial_offset;
}


ssize_t gridftp_write_stream(const Glib::Quark & scope,
        GridFTPStreamState* stream, const void* buffer, size_t s_write,
        bool eof)
{
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gridftp_write_stream]");
    off_t initial_offset = stream->get_offset();

    stream->set_stream_status(GRIDFTP_REQUEST_RUNNING);
	globus_result_t res = globus_ftp_client_register_write( stream->sess->get_ftp_handle(),
		(globus_byte_t*) buffer,
		s_write,
		initial_offset,
		eof,
		gfal_griftp_stream_write_callback,
		stream
	);
	gfal_globus_check_result(scope, res);
	gridftp_wait_for_write(scope, stream, initial_offset + s_write);
    stream->set_stream_status(GRIDFTP_REQUEST_NOT_LAUNCHED);
	return stream->get_offset() - initial_offset;
}


std::string gridftp_hostname_from_url(const char * url)
{
    GError * tmp_err = NULL;
    char buffer[GFAL_URL_MAX_LEN];
    buffer[0] = '\0';
    const int res = gfal_hostname_from_uri(url, buffer, GFAL_URL_MAX_LEN,
            &tmp_err);
    if (res < 0)
        throw Glib::Error(tmp_err);
    return std::string(buffer);
}
