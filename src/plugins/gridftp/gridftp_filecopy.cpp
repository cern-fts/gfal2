/*
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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

#include <string>
#include <sstream>
#include <ctime>
#include <csignal>

#include "gridftp_namespace.h"
#include "gridftp_filecopy.h"

#include <checksums/checksums.h>
#include <uri/uri_util.h>
#include <transfer/gfal_transfer_types_internal.h>
#include <file/gfal_file_api.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <exceptions/gerror_to_cpp.h>
#include <exceptions/cpp_to_gerror.hpp>

static Glib::Quark GFAL_GRIDFTP_SCOPE_FILECOPY("GridFTPFileCopyModule::FileCopy");

static Glib::Quark GFAL_GRIDFTP_DOMAIN_GSIFTP("GSIFTP");

/*IPv6 compatible lookup*/
static
std::string lookup_host(const char *host, gboolean use_ipv6)
{
    struct addrinfo hints, *addresses = NULL;
    int errcode;
    char addrstr[100] = { 0 };
    char ip4str[16] = { 0 };
    char ip6str[46] = { 0 };
    void *ptr = NULL;

    if (!host) {
        return std::string("cant.be.resolved");
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    errcode = getaddrinfo(host, NULL, &hints, &addresses);
    if (errcode != 0) {
        return std::string("cant.be.resolved");
    }

    struct addrinfo *i = addresses;
    while (i) {
        inet_ntop(i->ai_family, i->ai_addr->sa_data, addrstr, sizeof(addrstr));

        switch (i->ai_family) {
        case AF_INET:
            ptr = &((struct sockaddr_in *) i->ai_addr)->sin_addr;
            if (ptr)
                inet_ntop(i->ai_family, ptr, ip4str, sizeof(ip4str));
            break;
        case AF_INET6:
            ptr = &((struct sockaddr_in6 *) i->ai_addr)->sin6_addr;
            if (ptr)
                inet_ntop(i->ai_family, ptr, ip6str, sizeof(ip6str));
            break;
        }
        i = i->ai_next;
    }

    if (addresses)
        freeaddrinfo(addresses);

    if (use_ipv6 && ip6str[0])
        return std::string(ip6str);
    else if (ip4str[0])
        return std::string(ip4str);
    else
        return std::string("cant.be.resolved");
}


std::string return_hostname(const std::string &uri, gboolean use_ipv6)
{
    GError* error = NULL;
    gfal_uri parsed;
    gfal_parse_uri(uri.c_str(), &parsed, &error);
    if (error)
        throw Glib::Error(error);
    std::ostringstream str;
    str << lookup_host(parsed.domain, use_ipv6) << ":" << parsed.port;
    return str.str();
}


const char * gridftp_checksum_transfer_config   = "COPY_CHECKSUM_TYPE";
const char * gridftp_perf_marker_timeout_config = "PERF_MARKER_TIMEOUT";
const char * gridftp_skip_transfer_config       = "SKIP_SOURCE_CHECKSUM";
const char * gridftp_enable_udt                 = "ENABLE_UDT";


// return 1 if deleted something
int gridftp_filecopy_delete_existing(GridFTPModule* module,
        gfalt_params_t params, const char * url)
{
    const bool replace = gfalt_get_replace_existing_file(params, NULL);
    bool exist = module->exists(url);
    if (exist) {
        if (replace) {
            gfal_log(GFAL_VERBOSE_TRACE,
                    " File %s already exist, delete it for override ....", url);
            module->unlink(url);
            gfal_log(GFAL_VERBOSE_TRACE,
                    " File %s deleted with success, proceed to copy ....", url);
            plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP,
                    GFAL_EVENT_DESTINATION, GFAL_EVENT_OVERWRITE_DESTINATION,
                    "Deleted %s", url);
            return 1;
        }
        else {
            char err_buff[GFAL_ERRMSG_LEN];
            snprintf(err_buff, GFAL_ERRMSG_LEN,
                    " Destination already exist %s, Cancel", url);
            throw Gfal::TransferException(GFAL_GRIDFTP_SCOPE_FILECOPY,
                    err_buff,
                    EEXIST, GFALT_ERROR_DESTINATION, GFALT_ERROR_EXISTS);
        }
    }
    return 0;
}


// create the parent directory
void gridftp_create_parent_copy(GridFTPModule* module, gfalt_params_t params,
        const char * gridftp_url)
{
    const gboolean create_parent = gfalt_get_create_parent_dir(params, NULL);
    if (create_parent) {
        gfal_log(GFAL_VERBOSE_TRACE, " -> [gridftp_create_parent_copy]");
        char current_uri[GFAL_URL_MAX_LEN];
        g_strlcpy(current_uri, gridftp_url, GFAL_URL_MAX_LEN);
        const size_t s_uri = strlen(current_uri);
        char* p_uri = current_uri + s_uri - 1;

        while (p_uri > current_uri && *p_uri == '/') { // remove trailing '/'
            *p_uri = '\0';
            p_uri--;
        }
        while (p_uri > current_uri && *p_uri != '/') { // find the parent directory
            p_uri--;
        }
        if (p_uri > current_uri) {
            struct stat st;
            *p_uri = '\0';

            try {
                module->stat(current_uri, &st);
                if (!S_ISDIR(st.st_mode))
                    throw Gfal::TransferException(GFAL_GRIDFTP_SCOPE_FILECOPY,
                            "The parent of the destination file exists, but it is not a directory",
                            ENOTDIR, GFALT_ERROR_DESTINATION);
                return;
            }
            catch (Gfal::CoreException& e) {
                if (e.code() != ENOENT)
                    throw;
            }

            GError* tmp_err = NULL;
            (void) gfal2_mkdir_rec(module->get_session_factory()->get_handle(),
                    current_uri, 0755, &tmp_err);
            Gfal::gerror_to_cpp(&tmp_err);
        }
        else {
            throw Gfal::TransferException(GFAL_GRIDFTP_SCOPE_FILECOPY,
                    "Impossible to create directory " + std::string(current_uri)
                            + " : invalid path",
                    EINVAL, GFALT_ERROR_DESTINATION);
        }
        gfal_log(GFAL_VERBOSE_TRACE, " [gridftp_create_parent_copy] <-");
    }
}

void gsiftp_3rd_callback(void* user_args, globus_gass_copy_handle_t* handle, globus_off_t total_bytes, float throughput, float avg_throughput);

//
// Performance callback object
// contain the performance callback parameter
// an the auto cancel logic on performance callback inaticity
struct CallbackHandler{

    CallbackHandler(gfal2_context_t context, gfalt_params_t params,
            GridFTPRequestState* req, const char* src, const char* dst,
            size_t src_size) :
            args(NULL)
    {
        GError * tmp_err = NULL;
        gfalt_monitor_func callback = gfalt_get_monitor_callback(params,
                &tmp_err);
        Gfal::gerror_to_cpp(&tmp_err);
        gpointer user_args = gfalt_get_user_data(params, &tmp_err);
        Gfal::gerror_to_cpp(&tmp_err);

        if (callback) {
            args = new callback_args(context, callback, user_args, src, dst,
                    req, src_size);
        }
    }

    static void* func_timer(void* v)
    {
        callback_args* args = (callback_args*) v;
        while (time(NULL) < args->timeout_time) {
            if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0) {
                gfal_log(GFAL_VERBOSE_TRACE,
                        " thread setcancelstate error, interrupt performance marker timer");
                return NULL;
            }
            usleep(500000);
            if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) != 0) {
                gfal_log(GFAL_VERBOSE_TRACE,
                        " thread setcancelstate error, interrupt performance marker timer");
                return NULL;
            }
        }

        std::stringstream msg;
        msg << "Transfer canceled because the gsiftp performance marker timeout of "
            << args->timeout_value
            << " seconds has been exceeded, or all performance markers during that period indicated zero bytes transferred";
        args->req->cancel_operation_async(GFAL_GRIDFTP_SCOPE_FILECOPY, msg.str());

        return NULL;
    }

    virtual ~CallbackHandler()
    {
        if (args) {
            delete args;
        }
    }

    struct callback_args {
        callback_args(gfal2_context_t context,
                const gfalt_monitor_func mcallback, gpointer muser_args,
                const char* msrc, const char* mdst, GridFTPRequestState* mreq,
                size_t src_size) :
                callback(mcallback), user_args(muser_args), req(mreq), src(
                        msrc), dst(mdst), start_time(time(NULL)), timeout_value(
                        gfal2_get_opt_integer_with_default(context,
                                GRIDFTP_CONFIG_GROUP,
                                gridftp_perf_marker_timeout_config, 180)), timeout_time(
                        time(NULL) + timeout_value), timer_pthread()
        {
            Glib::RWLock::ReaderLock l(req->mux_req_state);
            globus_gass_copy_register_performance_cb(
                    req->sess->get_gass_handle(), gsiftp_3rd_callback,
                    (gpointer) this);

            if (timeout_value > 0) {
                pthread_create(&timer_pthread, NULL,
                        CallbackHandler::func_timer, this);
            }

            source_size = src_size;
        }

        virtual ~callback_args()
        {
            if (timeout_value > 0) {
                void * res;
                pthread_cancel(timer_pthread);
                pthread_join(timer_pthread, &res);
            }
            Glib::RWLock::ReaderLock l(req->mux_req_state);
            globus_gass_copy_register_performance_cb(
                    req->sess->get_gass_handle(), NULL, NULL);
        }

        gfalt_monitor_func callback;
        gpointer user_args;
        GridFTPRequestState* req;
        const char* src;
        const char* dst;
        time_t start_time;
        int timeout_value;
        time_t timeout_time;
        pthread_t timer_pthread;
        globus_off_t source_size;
    }*args;
};


void gsiftp_3rd_callback(void* user_args, globus_gass_copy_handle_t* handle,
        globus_off_t total_bytes, float throughput, float avg_throughput)
{
    CallbackHandler::callback_args * args =
            (CallbackHandler::callback_args *) user_args;

    gfalt_hook_transfer_plugin_t hook;
    hook.bytes_transfered = total_bytes;
    hook.average_baudrate = (size_t) avg_throughput;
    hook.instant_baudrate = (size_t) throughput;
    hook.transfer_time = (time(NULL) - args->start_time);

    gfalt_transfer_status_t state = gfalt_transfer_status_create(&hook);
    args->callback(state, args->src, args->dst, args->user_args);
    gfalt_transfer_status_delete(state);

    // If throughput != 0, or the file has been already sent, reset timer callback
    // [LCGUTIL-440] Some endpoints calculate the checksum before closing, so we will
    //               get throughput = 0 for a while, and the transfer should not fail
    if (throughput != 0.0
            || (args->source_size > 0 && args->source_size <= total_bytes)) {
        GridFTPRequestState* req = args->req;
        Glib::RWLock::ReaderLock l(req->mux_req_state);
        if (args->timeout_value > 0) {
            gfal_log(GFAL_VERBOSE_TRACE,
                    "Performance marker received, re-arm timer");
            args->timeout_time = time(NULL) + args->timeout_value;
        }
    }
    // Otherwise, do not reset and notify
    else {
        gfal_log(GFAL_VERBOSE_NORMAL,
                "Performance marker received, but throughput is 0. Not resetting timeout!");
    }
}


static
void gridftp_do_copy(GridFTPModule* module, GridFTPFactory* factory,
        gfalt_params_t params, const char* src, const char* dst,
        GridFTPRequestState& req, time_t timeout)
{
    /*
     This will not work with Castor
     struct stat src_stat;

     try {
     module->stat(src, &src_stat);
     }
     catch (...) {
     src_stat.st_size = 0;
     }
     */

    GridFTPSession* sess = req.sess.get();

    Gass_attr_handler gass_attr_src(sess->get_op_attr_ftp());
    Gass_attr_handler gass_attr_dst(sess->get_op_attr_ftp());
    CallbackHandler callback_handler(factory->get_handle(), params, &req, src,
            dst, 0);

    req.start();
    gfal_log(GFAL_VERBOSE_TRACE,
            "   [GridFTPFileCopyModule::filecopy] start gridftp transfer %s -> %s",
            src, dst);
    GridFTPOperationCanceler canceler(factory->get_handle(), &req);
    globus_result_t res = globus_gass_copy_register_url_to_url(
            sess->get_gass_handle(), (char*) src, &(gass_attr_src.attr_gass),
            (char*) dst, &(gass_attr_dst.attr_gass),
            globus_gass_basic_client_callback, &req);

    gfal_globus_check_result("GridFTPFileCopyModule::filecopy", res);
    req.wait_callback(GFAL_GRIDFTP_SCOPE_FILECOPY, timeout);
}


static
int gridftp_filecopy_copy_file_internal(GridFTPModule* module,
        GridFTPFactory * factory, gfalt_params_t params, const char* src,
        const char* dst)
{
    GError * tmp_err = NULL;

    const time_t timeout = gfalt_get_timeout(params, &tmp_err);
    Gfal::gerror_to_cpp(&tmp_err);

    const unsigned int nbstream = gfalt_get_nbstreams(params, &tmp_err);
    Gfal::gerror_to_cpp(&tmp_err);
    const guint64 tcp_buffer_size = gfalt_get_tcp_buffer_size(params, &tmp_err);
    Gfal::gerror_to_cpp(&tmp_err);

    if (gfalt_get_strict_copy_mode(params, NULL) == false) {
        // If 1, the destination was deleted. So the parent directory is there!
        if (gridftp_filecopy_delete_existing(module, params, dst) == 0)
            gridftp_create_parent_copy(module, params, dst);
    }

    GridFTPRequestState req(
            factory->gfal_globus_ftp_take_handle(
                    gridftp_hostname_from_url(src)), true,
            GRIDFTP_REQUEST_GASS);
    GridFTPSession* sess = req.sess.get();

    sess->set_nb_stream(nbstream);
    gfal_log(GFAL_VERBOSE_TRACE,
            "   [GridFTPFileCopyModule::filecopy] setup gsiftp number of streams to %d",
            nbstream);
    sess->set_tcp_buffer_size(tcp_buffer_size);
    gfal_log(GFAL_VERBOSE_TRACE,
            "   [GridFTPFileCopyModule::filecopy] setup gsiftp buffer size to %d",
            tcp_buffer_size);

    gboolean enable_udt_transfers = gfal2_get_opt_boolean(factory->get_handle(),
    GRIDFTP_CONFIG_GROUP, gridftp_enable_udt, NULL);

    if (enable_udt_transfers) {
        gfal_log(GFAL_VERBOSE_VERBOSE, "Trying UDT transfer");
        plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP, GFAL_EVENT_NONE,
                g_quark_from_static_string("UDT:ENABLE"), "Trying UDT");
        sess->enable_udt();
    }

    try {
        gridftp_do_copy(module, factory, params, src, dst, req, timeout);
    }
    catch (Gfal::CoreException& e) {
        // Try again if the failure was related to udt
        if (e.what().find("udt driver not whitelisted")
                != Glib::ustring::npos) {
            gfal_log(GFAL_VERBOSE_VERBOSE,
                    "UDT transfer failed! Disabling and retrying...");

            plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP, GFAL_EVENT_NONE,
                    g_quark_from_static_string("UDT:DISABLE"),
                    "UDT failed. Falling back to default mode: %s",
                    e.what().c_str());

            sess->disable_udt();
            gridftp_do_copy(module, factory, params, src, dst, req, timeout);
        }
        // Else, rethrow
        else {
            throw;
        }
    }

    return 0;

}


static
void gridftp_checksum_transfer_verify(const char * src_chk, const char* dst_chk,
        const char* user_defined_chk)
{
    std::ostringstream ss;

    if (*user_defined_chk == '\0') {
        if (gfal_compare_checksums(src_chk, dst_chk, GFAL_URL_MAX_LEN) != 0) {
            ss << "SRC and DST checksum are different. Source: " << src_chk
                    << " Destination: " << dst_chk;
            throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_FILECOPY, ss.str(),
            EIO);
        }
    }
    else {
        if (src_chk[0] != '\0'
                && gfal_compare_checksums(src_chk, user_defined_chk,
                        GFAL_URL_MAX_LEN) != 0) {
            ss << "USER_DEFINE and SRC checksums are different. "
                    << user_defined_chk << " != " << src_chk;
            throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_FILECOPY, ss.str(),
            EIO);
        }

        if (gfal_compare_checksums(dst_chk, user_defined_chk, GFAL_URL_MAX_LEN)
                != 0) {
            ss << "USER_DEFINE and DST checksums are different. "
                    << user_defined_chk << " != " << dst_chk;
            throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_FILECOPY, ss.str(),
            EIO);
        }
    }
}


// clear dest if error occures in transfer, does not clean if dest file if set as already exist before any transfer
void GridFTPModule::autoCleanFileCopy(gfalt_params_t params,
        GError* checked_error, const char* dst)
{
    if (checked_error && checked_error->code != EEXIST) {
        gfal_log(GFAL_VERBOSE_TRACE,
                "\t\tError in transfer, clean destination file %s ", dst);
        try {
            this->unlink(dst);
        }
        catch (...) {
            gfal_log(GFAL_VERBOSE_TRACE, "\t\tFailure in cleaning ...");
        }
    }
}


void GridFTPModule::filecopy(gfalt_params_t params, const char* src,
        const char* dst)
{
    char checksum_type[GFAL_URL_MAX_LEN] = { 0 };
    char checksum_user_defined[GFAL_URL_MAX_LEN];
    char checksum_src[GFAL_URL_MAX_LEN] = { 0 };
    char checksum_dst[GFAL_URL_MAX_LEN] = { 0 };

    gboolean checksum_check = gfalt_get_checksum_check(params, NULL);
    gboolean skip_source_checksum = gfal2_get_opt_boolean(
            _handle_factory->get_handle(),
            GRIDFTP_CONFIG_GROUP, gridftp_skip_transfer_config,
            NULL);
    gboolean use_ipv6 = gfal2_get_opt_boolean(_handle_factory->get_handle(),
    GRIDFTP_CONFIG_GROUP, gridftp_ipv6_config,
    NULL);

    if (checksum_check) {
        gfalt_get_user_defined_checksum(params, checksum_type,
                sizeof(checksum_type), checksum_user_defined,
                sizeof(checksum_user_defined),
                NULL);

        if (checksum_user_defined[0] == '\0' && checksum_type[0] == '\0') {
            GError *get_default_error = NULL;
            char *default_checksum_type;

            default_checksum_type = gfal2_get_opt_string(
                    _handle_factory->get_handle(),
                    GRIDFTP_CONFIG_GROUP, gridftp_checksum_transfer_config,
                    &get_default_error);
            Gfal::gerror_to_cpp(&get_default_error);

            g_strlcpy(checksum_type, default_checksum_type,
                    sizeof(checksum_type));
            g_free(default_checksum_type);

            gfal_log(GFAL_VERBOSE_TRACE,
                    "\t\tNo user defined checksum, fetch the default one from configuration");
        }

        gfal_log(GFAL_VERBOSE_DEBUG,
                "\t\tChecksum Algorithm for transfer verification %s",
                checksum_type);
    }

    // Retrieving the source checksum and doing the transfer can be, potentially,
    // done in parallel. But not for now.
    // (That's why the brackets: they are marking potential parallelizable sections)
    // But remember to modify the catches when you make them parallel!

    // Source checksum
    {
        try {
            if (checksum_check && !skip_source_checksum) {
                plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP,
                        GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_ENTER, "%s",
                        checksum_type);

                checksum(src, checksum_type, checksum_src, sizeof(checksum_src),
                        0, 0);

                plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP,
                        GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_EXIT, "%s=%s",
                        checksum_type, checksum_src);
            }
        }
        catch (const Glib::Error &e) {
            throw Gfal::TransferException(e.domain(), e.what(), e.code(),
                    GFALT_ERROR_SOURCE);
        }
        catch (...) {
            throw Gfal::TransferException(GFAL_GRIDFTP_DOMAIN_GSIFTP,
                    "Undefined Exception caught while getting the source checksum!!",
                    EIO, GFALT_ERROR_SOURCE);
        }
    }

    // Transfer
    GError* transfer_error = NULL;
    {
        plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP, GFAL_EVENT_NONE,
                GFAL_EVENT_TRANSFER_ENTER, "(%s) %s => (%s) %s",
                return_hostname(src, use_ipv6).c_str(), src,
                return_hostname(dst, use_ipv6).c_str(), dst);
        try {
            gridftp_filecopy_copy_file_internal(this, _handle_factory, params,
                    src, dst);
        }
        catch (Gfal::TransferException & e) {
            throw;
        }
        catch (Glib::Error & e) {
            autoCleanFileCopy(params, transfer_error, dst);
            throw Gfal::TransferException(e.domain(), e.what(), e.code(),
                    GFALT_ERROR_TRANSFER);
        }
        catch (std::exception & e) {
            autoCleanFileCopy(params, transfer_error, dst);
            throw Gfal::TransferException(GFAL_GRIDFTP_DOMAIN_GSIFTP, e.what(), EIO,
                    GFALT_ERROR_TRANSFER, "UNEXPECTED");
        }
        catch (...) {
            autoCleanFileCopy(params, transfer_error, dst);
            throw;
        }

        plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP, GFAL_EVENT_NONE,
                GFAL_EVENT_TRANSFER_EXIT, "(%s) %s => (%s) %s",
                return_hostname(src, use_ipv6).c_str(), src,
                return_hostname(dst, use_ipv6).c_str(), dst);
    }

    // Validate destination checksum
    if (checksum_check) {
        plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP,
                GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_ENTER, "%s",
                checksum_type);

        try {
            checksum(dst, checksum_type, checksum_dst, sizeof(checksum_dst), 0,
                    0);
            gridftp_checksum_transfer_verify(checksum_src, checksum_dst,
                    checksum_user_defined);
        }
        catch (Glib::Error & e) {
            throw Gfal::TransferException(e.domain(), e.what(), e.code(),
                    GFALT_ERROR_TRANSFER, GFALT_ERROR_CHECKSUM);
        }
        catch (...) {
            throw Gfal::TransferException(GFAL_GRIDFTP_DOMAIN_GSIFTP,
                    "Unexpected exception", EIO, GFALT_ERROR_TRANSFER);
        }

        plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP,
                GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_EXIT, "%s",
                checksum_type);
    }
}


/**
 * initiaize a file copy from the given source to the given dest with the parameters params
 */
extern "C" int gridftp_plugin_filecopy(plugin_handle handle, gfal2_context_t context,
        gfalt_params_t params, const char* src, const char* dst, GError ** err)
{
    g_return_val_err_if_fail(handle != NULL && src != NULL && dst != NULL, -1,
            err, "[plugin_filecopy][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gridftp_plugin_filecopy]");
    CPP_GERROR_TRY
                (static_cast<GridFTPModule*>(handle))->filecopy(params, src,
                        dst);
                ret = 0;
            CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gridftp_plugin_filecopy]<-");
    G_RETURN_ERR(ret, tmp_err, err);
}
