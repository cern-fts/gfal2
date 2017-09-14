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

#include <string>
#include <sstream>
#include <ctime>
#include <csignal>

#include "gridftp_namespace.h"
#include "gridftp_filecopy.h"
#include "gridftp_plugin.h"

#include <checksums/checksums.h>
#include <uri/gfal2_uri.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <exceptions/gerror_to_cpp.h>
#include <exceptions/cpp_to_gerror.hpp>

static const GQuark GFAL_GRIDFTP_SCOPE_FILECOPY = g_quark_from_string("GridFTPFileCopyModule::FileCopy");
const GQuark GFAL_GRIDFTP_DOMAIN_GSIFTP = g_quark_from_string("GSIFTP");

/*IPv6 compatible lookup*/
std::string lookup_host(const char *host, bool ipv6_enabled, bool *got_ipv6)
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

    if (got_ipv6) {
        *got_ipv6 = false;
    }

    struct addrinfo *i = addresses;
    while (i) {
        inet_ntop(i->ai_family, i->ai_addr->sa_data, addrstr, sizeof(addrstr));

        switch (i->ai_family) {
        case AF_INET:
            ptr = &((struct sockaddr_in *) i->ai_addr)->sin_addr;
            if (ptr) {
                inet_ntop(i->ai_family, ptr, ip4str, sizeof(ip4str));
            }
            break;
        case AF_INET6:
            ptr = &((struct sockaddr_in6 *) i->ai_addr)->sin6_addr;
            if (ptr) {
                inet_ntop(i->ai_family, ptr, ip6str, sizeof(ip6str));
                if (got_ipv6) {
                    *got_ipv6 = true;
                }
            }
            break;
        }
        i = i->ai_next;
    }

    if (addresses) {
        freeaddrinfo(addresses);
    }

    if (ipv6_enabled && ip6str[0]) {
        return std::string("[").append(ip6str).append("]");
    }
    else if (ip4str[0]) {
        return std::string(ip4str);
    }
    else {
        return std::string("cant.be.resolved");
    }
}


std::string return_host_and_port(const std::string &uri, gboolean use_ipv6)
{
    GError* error = NULL;
    gfal2_uri *parsed = gfal2_parse_uri(uri.c_str(), &error);
    if (error) {
        throw Gfal::CoreException(error);
    }
    std::ostringstream str;
    str << lookup_host(parsed->host, use_ipv6, NULL) << ":" << parsed->port;
    gfal2_free_uri(parsed);
    return str.str();
}


// return 1 if deleted something
int gridftp_filecopy_delete_existing(GridFTPModule* module,
        gfalt_params_t params, const char * url)
{
    const bool replace = gfalt_get_replace_existing_file(params, NULL);
    bool exist = module->exists(url);
    if (exist) {
        if (replace) {
            gfal2_log(G_LOG_LEVEL_DEBUG,
                    " File %s already exist, delete it for override ....", url);
            module->unlink(url);
            gfal2_log(G_LOG_LEVEL_DEBUG,
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
            throw Gfal::TransferException(GFAL_GRIDFTP_SCOPE_FILECOPY, EEXIST,
                    err_buff, GFALT_ERROR_DESTINATION, GFALT_ERROR_EXISTS);
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
        gfal2_log(G_LOG_LEVEL_DEBUG, " -> [gridftp_create_parent_copy]");
        char current_uri[GFAL_URL_MAX_LEN];
        const size_t s_uri = g_strlcpy(current_uri, gridftp_url, GFAL_URL_MAX_LEN);
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
                if (!S_ISDIR(st.st_mode)) {
                    throw Gfal::TransferException(GFAL_GRIDFTP_SCOPE_FILECOPY, ENOTDIR,
                            "The parent of the destination file exists, but it is not a directory",
                            GFALT_ERROR_DESTINATION);
                }
                return;
            }
            catch (Gfal::CoreException& e) {
                if (e.code() != ENOENT)
                    throw;
            }

            GError* tmp_err = NULL;
            (void) gfal2_mkdir_rec(module->get_session_factory()->get_gfal2_context(),
                    current_uri, 0755, &tmp_err);
            Gfal::gerror_to_cpp(&tmp_err);
        }
        else {
            throw Gfal::TransferException(GFAL_GRIDFTP_SCOPE_FILECOPY, EINVAL,
                    "Impossible to create directory " + std::string(current_uri) + " : invalid path",
                    GFALT_ERROR_DESTINATION);
        }
        gfal2_log(G_LOG_LEVEL_DEBUG, " [gridftp_create_parent_copy] <-");
    }
}

void gsiftp_3rd_callback(void* user_args, globus_gass_copy_handle_t* handle, globus_off_t total_bytes, float throughput, float avg_throughput);

//
// Performance callback object
// contain the performance callback parameter
// and the auto cancel logic on performance callback inactivity
struct CallbackHandler {

    static void* func_timer(void* v)
    {
        CallbackHandler* args = (CallbackHandler*) v;
        while (time(NULL) < args->timeout_time) {
            if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0) {
                gfal2_log(G_LOG_LEVEL_DEBUG, "thread setcancelstate error, interrupt performance marker timer");
                return NULL;
            }
            usleep(500000);
            if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) != 0) {
                gfal2_log(G_LOG_LEVEL_DEBUG, "thread setcancelstate error, interrupt performance marker timer");
                return NULL;
            }
        }

        std::stringstream msg;
        msg << "Transfer canceled because the gsiftp performance marker timeout of "
            << args->timeout_value
            << " seconds has been exceeded, or all performance markers during that period indicated zero bytes transferred";

        try {
            args->req->cancel(GFAL_GRIDFTP_SCOPE_FILECOPY, msg.str());
        }
        catch (Gfal::CoreException& e) {
            gfal2_log(G_LOG_LEVEL_WARNING,
                    "Exception while cancelling on performance marker timeout: %s",
                    e.what());
        }
        catch (...) {
            gfal2_log(G_LOG_LEVEL_WARNING,
                    "Unknown exception while cancelling on performance marker timeout");
        }

        pthread_exit(NULL);
    }

    CallbackHandler(gfal2_context_t context, gfalt_params_t params,
            GridFTPRequestState* req, const char* src, const char* dst,
            size_t src_size):
                params(params), req(req), src(src), dst(dst), start_time(0), timeout_value(0),
                timeout_time(0), timer_pthread(0), source_size(src_size)
    {
        timeout_value = gfal2_get_opt_integer_with_default(context,
                    GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_TRANSFER_PERF_TIMEOUT, 180);

        start_time = time(NULL);

        if (timeout_value > 0) {
            timeout_time = start_time + timeout_value;
            pthread_create(&timer_pthread, NULL, CallbackHandler::func_timer, this);
        }

        globus_gass_copy_register_performance_cb(
                req->handler->get_gass_copy_handle(), gsiftp_3rd_callback,
                (gpointer) this);
    }

    virtual ~CallbackHandler()
    {
        if (timeout_value > 0) {
            pthread_cancel(timer_pthread);
            pthread_join(timer_pthread, NULL);
        }
        globus_gass_copy_register_performance_cb(req->handler->get_gass_copy_handle(), NULL, NULL);
    }

    gfalt_params_t params;
    GridFTPRequestState* req;
    const char* src;
    const char* dst;
    time_t start_time;
    int timeout_value;
    time_t timeout_time;
    pthread_t timer_pthread;
    globus_off_t source_size;
};


void gsiftp_3rd_callback(void* user_args, globus_gass_copy_handle_t* handle,
        globus_off_t total_bytes, float throughput, float avg_throughput)
{
    CallbackHandler* args = (CallbackHandler*)user_args;

    _gfalt_transfer_status status;
    status.bytes_transfered = total_bytes;
    status.average_baudrate = (size_t) avg_throughput;
    status.instant_baudrate = (size_t) throughput;
    status.transfer_time = (time(NULL) - args->start_time);

    plugin_trigger_monitor(args->params, &status, args->src, args->dst);

    if (args->timeout_time > 0) {
        // If throughput != 0, or the file has been already sent, reset timer callback
        // [LCGUTIL-440] Some endpoints calculate the checksum before closing, so we will
        //               get throughput = 0 for a while, and the transfer should not fail
        if (throughput != 0.0 || (args->source_size > 0 && args->source_size <= total_bytes)) {
            //GridFTPRequestState* req = args->req;
            //Glib::RWLock::ReaderLock l(req->mux_req_state);
            if (args->timeout_value > 0) {
                gfal2_log(G_LOG_LEVEL_DEBUG, "Performance marker received, re-arm timer");
                args->timeout_time = time(NULL) + args->timeout_value;
            }
        }
        // Otherwise, do not reset and notify
        else {
            gfal2_log(G_LOG_LEVEL_MESSAGE,
                    "Performance marker received, but throughput is 0. Not resetting timeout!");
        }
    }
}


void gridftp_set_credentials(gfal2_context_t context, GassCopyAttrHandler &gass_attr, const char *url)
{
    GError *error = NULL;
    const char *baseurl;

    gchar *ucert = gfal2_cred_get(context, GFAL_CRED_X509_CERT, url, &baseurl, &error);
    Gfal::gerror_to_cpp(&error);
    gchar *ukey = gfal2_cred_get(context, GFAL_CRED_X509_KEY, url, &baseurl, &error);
    Gfal::gerror_to_cpp(&error);

    gfal_globus_set_credentials(ucert, ukey, NULL, NULL, &gass_attr.cred_id, gass_attr.attr_gass.ftp_attr);
    gfal2_log(G_LOG_LEVEL_DEBUG, "Using %s:%s for %s", ucert, ukey, baseurl);

    g_free(ucert);
    g_free(ukey);
}


static
void gridftp_do_copy_inner(GridFTPModule* module, GridFTPFactory* factory,
        gfalt_params_t params, const char* src, const char* dst,
        GridFTPRequestState& req, time_t timeout)
{
    GassCopyAttrHandler gass_attr_src(req.handler->get_ftp_client_operationattr());
    GassCopyAttrHandler gass_attr_dst(req.handler->get_ftp_client_operationattr());

    gfal2_log(G_LOG_LEVEL_DEBUG,
            "[GridFTPFileCopyModule::filecopy] start gridftp transfer %s -> %s",
            src, dst);

    // Required for the PASV plugin to be able to trigger events
    req.handler->session->params = params;

    // Override source/destination credentials with specifics
    // Source is properly set up already, as the session is retrieved using it
    gridftp_set_credentials(factory->get_gfal2_context(), gass_attr_dst, dst);

    try {
        globus_result_t res = globus_gass_copy_register_url_to_url(
                req.handler->get_gass_copy_handle(),
                (char*) src, &(gass_attr_src.attr_gass),
                (char*) dst, &(gass_attr_dst.attr_gass),
                globus_gass_client_done_callback, &req);

        gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_FILECOPY, res);
        req.wait(GFAL_GRIDFTP_SCOPE_FILECOPY, timeout);
        req.handler->session->params = NULL;
    }
    catch (...) {
        req.handler->session->params = NULL;
        throw;
    }
}


static
void gridftp_do_copy(GridFTPModule* module, GridFTPFactory* factory,
    gfalt_params_t params, const char* src, const char* dst,
    GridFTPRequestState& req, time_t timeout)
{
    if (strncmp(src, "ftp:", 4) == 0 || strncmp(dst, "ftp:", 4) == 0) {
        gfal2_log(G_LOG_LEVEL_DEBUG,
                  "[GridFTPFileCopyModule::filecopy] start gridftp transfer without performance markers");
        gridftp_do_copy_inner(module, factory, params, src, dst, req, timeout);
    }
    else {
        CallbackHandler callback_handler(factory->get_gfal2_context(), params, &req, src, dst, 0);
        gfal2_log(G_LOG_LEVEL_DEBUG,
                  "[GridFTPFileCopyModule::filecopy] start gridftp transfer with performance markers enabled (timeout %d)",
                  callback_handler.timeout_value);

        gridftp_do_copy_inner(module, factory, params, src, dst, req, timeout);
    }
}


static
int gridftp_filecopy_copy_file_internal(GridFTPModule* module,
        GridFTPFactory * factory, gfalt_params_t params, const char* src,
        const char* dst)
{
    GError * tmp_err = NULL;

    gboolean is_strict_mode = gfalt_get_strict_copy_mode(params, NULL);

    const time_t timeout = gfalt_get_timeout(params, &tmp_err);
    Gfal::gerror_to_cpp(&tmp_err);

    const unsigned int nbstream = gfalt_get_nbstreams(params, &tmp_err);
    Gfal::gerror_to_cpp(&tmp_err);
    const guint64 tcp_buffer_size = gfalt_get_tcp_buffer_size(params, &tmp_err);
    Gfal::gerror_to_cpp(&tmp_err);

    if (!is_strict_mode) {
        // If 1, the destination was deleted. So the parent directory is there!
        if (gridftp_filecopy_delete_existing(module, params, dst) == 0)
            gridftp_create_parent_copy(module, params, dst);
    }

    GridFTPSessionHandler handler(factory, src);
    GridFTPRequestState req(&handler, GRIDFTP_REQUEST_GASS);

    handler.session->set_nb_streams(nbstream);
    gfal2_log(G_LOG_LEVEL_DEBUG,
            "   [GridFTPFileCopyModule::filecopy] setup gsiftp number of streams to %d",
            nbstream);
    handler.session->set_tcp_buffer_size(tcp_buffer_size);
    gfal2_log(G_LOG_LEVEL_DEBUG,
            "   [GridFTPFileCopyModule::filecopy] setup gsiftp buffer size to %d",
            tcp_buffer_size);

    gboolean enable_udt_transfers = gfal2_get_opt_boolean(factory->get_gfal2_context(),
            GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_TRANSFER_UDT, NULL);

    if (enable_udt_transfers) {
        gfal2_log(G_LOG_LEVEL_MESSAGE, "Trying UDT transfer");
        plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP, GFAL_EVENT_NONE,
                g_quark_from_static_string("UDT:ENABLE"), "Trying UDT");
        handler.session->set_udt(true);
    }

    try {
        gridftp_do_copy(module, factory, params, src, dst, req, timeout);
    }
    catch (Gfal::CoreException& e) {
        // Try again if the failure was related to udt
        if (e.what_str().find("udt driver not whitelisted") != std::string::npos) {
            gfal2_log(G_LOG_LEVEL_WARNING,
                    "UDT transfer failed! Disabling and retrying...");

            plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP, GFAL_EVENT_NONE,
                    g_quark_from_static_string("UDT:DISABLE"),
                    "UDT failed. Falling back to default mode: %s",
                    e.what());

            handler.session->set_udt(false);
            gridftp_do_copy(module, factory, params, src, dst, req, timeout);
        }
        // Else, rethrow
        else {
            handler.session->params = NULL;
            throw;
        }
    }

    return 0;

}

// clear dest if error occures in transfer, does not clean if dest file if set as already exist before any transfer
void GridFTPModule::autoCleanFileCopy(gfalt_params_t params,
        GError* checked_error, const char* dst)
{
    if (checked_error && checked_error->code != EEXIST) {
        gfal2_log(G_LOG_LEVEL_DEBUG,
                "\t\tError in transfer, clean destination file %s ", dst);
        try {
            this->unlink(dst);
        }
        catch (...) {
            gfal2_log(G_LOG_LEVEL_DEBUG, "\t\tFailure in cleaning ...");
        }
    }
}


void GridFTPModule::filecopy(gfalt_params_t params, const char* src,
        const char* dst)
{
    std::stringstream errstr;
    char checksum_type[GFAL_URL_MAX_LEN] = { 0 };
    char checksum_user_defined[GFAL_URL_MAX_LEN];
    char checksum_src[GFAL_URL_MAX_LEN] = { 0 };
    char checksum_dst[GFAL_URL_MAX_LEN] = { 0 };

    gboolean use_ipv6 = gfal2_get_opt_boolean(_handle_factory->get_gfal2_context(),
        GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_IPV6,
        NULL);

    gfalt_checksum_mode_t checksum_mode = GFALT_CHECKSUM_NONE;
    if (!gfalt_get_strict_copy_mode(params, NULL)) {
        checksum_mode = gfalt_get_checksum(params,
            checksum_type, sizeof(checksum_type),
            checksum_user_defined, sizeof(checksum_user_defined),
            NULL);
    }

    gboolean skip_source_checksum = gfal2_get_opt_boolean(
        _handle_factory->get_gfal2_context(),
        GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_TRANSFER_SKIP_CHECKSUM,
        NULL);
    if (skip_source_checksum) {
        checksum_mode = gfalt_checksum_mode_t(checksum_mode & ~GFALT_CHECKSUM_SOURCE);
    }

    if (checksum_mode) {
        if (checksum_user_defined[0] == '\0' && checksum_type[0] == '\0') {
            GError *get_default_error = NULL;
            char *default_checksum_type;

            default_checksum_type = gfal2_get_opt_string(
                    _handle_factory->get_gfal2_context(),
                    GRIDFTP_CONFIG_GROUP, GRIDFTP_CONFIG_TRANSFER_CHECKSUM,
                    &get_default_error);
            Gfal::gerror_to_cpp(&get_default_error);

            g_strlcpy(checksum_type, default_checksum_type,
                    sizeof(checksum_type));
            g_free(default_checksum_type);

            gfal2_log(G_LOG_LEVEL_DEBUG,
                    "\t\tNo user defined checksum, fetch the default one from configuration");
        }

        gfal2_log(G_LOG_LEVEL_DEBUG,
                "\t\tChecksum Algorithm for transfer verification %s",
                checksum_type);
    }

    // Source checksum
    if (checksum_mode & GFALT_CHECKSUM_SOURCE) {
        plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP,
            GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_ENTER, "%s",
            checksum_type);

        checksum(src, checksum_type, checksum_src, sizeof(checksum_src), 0, 0);

        plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP,
            GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_EXIT, "%s=%s",
            checksum_type, checksum_src);

        if (checksum_user_defined[0]) {
            if (gfal_compare_checksums(checksum_user_defined, checksum_src, GFAL_URL_MAX_LEN) != 0) {
                errstr << "USER_DEFINE and SRC checksums are different. "
                       << checksum_user_defined << " != " << checksum_src;
                throw Gfal::TransferException(GFAL_GRIDFTP_SCOPE_FILECOPY, EIO, errstr.str(),
                    GFALT_ERROR_TRANSFER, GFALT_ERROR_CHECKSUM_MISMATCH);
            }
        }
    }

    // Transfer
    GError* transfer_error = NULL;
    plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP, GFAL_EVENT_NONE,
            GFAL_EVENT_TRANSFER_ENTER, "(%s) %s => (%s) %s",
            return_host_and_port(src, use_ipv6).c_str(), src,
            return_host_and_port(dst, use_ipv6).c_str(), dst);
    plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP,
        GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_TYPE,
        "%s", GFAL_TRANSFER_TYPE_PUSH);

    try {
        gridftp_filecopy_copy_file_internal(this, _handle_factory, params, src, dst);
    }
    catch (Gfal::TransferException & e) {
        throw;
    }
    catch (const Gfal::CoreException & e) {
        autoCleanFileCopy(params, transfer_error, dst);
        throw Gfal::TransferException(e.domain(), e.code(), e.what(),
                GFALT_ERROR_TRANSFER);
    }
    catch (std::exception & e) {
        autoCleanFileCopy(params, transfer_error, dst);
        throw Gfal::TransferException(GFAL_GRIDFTP_DOMAIN_GSIFTP, EIO, e.what(),
                GFALT_ERROR_TRANSFER, "UNEXPECTED");
    }
    catch (...) {
        autoCleanFileCopy(params, transfer_error, dst);
        throw;
    }

    plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP, GFAL_EVENT_NONE,
            GFAL_EVENT_TRANSFER_EXIT, "(%s) %s => (%s) %s",
            return_host_and_port(src, use_ipv6).c_str(), src,
            return_host_and_port(dst, use_ipv6).c_str(), dst);

    // Validate destination checksum
    if (checksum_mode & GFALT_CHECKSUM_TARGET) {
        plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP,
                GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_ENTER, "%s",
                checksum_type);

        checksum(dst, checksum_type, checksum_dst, sizeof(checksum_dst), 0, 0);

        plugin_trigger_event(params, GFAL_GRIDFTP_DOMAIN_GSIFTP,
            GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_EXIT, "%s",
            checksum_type);

        if (checksum_mode & GFALT_CHECKSUM_SOURCE) {
            if (gfal_compare_checksums(checksum_src, checksum_dst, GFAL_URL_MAX_LEN) != 0) {
                errstr << "SRC and DST checksum are different. Source: " << checksum_src
                   << " Destination: " << checksum_dst;
                throw Gfal::TransferException(GFAL_GRIDFTP_SCOPE_FILECOPY, EIO, errstr.str(),
                    GFALT_ERROR_TRANSFER, GFALT_ERROR_CHECKSUM_MISMATCH);
            }
        }
        else {
            if (gfal_compare_checksums(checksum_user_defined, checksum_dst, GFAL_URL_MAX_LEN) != 0) {
                errstr << "USER_DEFINE and DST checksums are different. "
                   << checksum_user_defined << " != " << checksum_dst;
                throw Gfal::TransferException(GFAL_GRIDFTP_SCOPE_FILECOPY, EIO, errstr.str(),
                    GFALT_ERROR_TRANSFER, GFALT_ERROR_CHECKSUM_MISMATCH);
            }
        }
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
    gfal2_log(G_LOG_LEVEL_DEBUG, "  -> [gridftp_plugin_filecopy]");
    CPP_GERROR_TRY
        (static_cast<GridFTPModule*>(handle))->filecopy(params, src, dst);
        ret = 0;
    CPP_GERROR_CATCH(&tmp_err);
    gfal2_log(G_LOG_LEVEL_DEBUG, "  [gridftp_plugin_filecopy]<-");
    G_RETURN_ERR(ret, tmp_err, err);
}
