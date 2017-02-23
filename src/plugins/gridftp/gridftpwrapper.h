/*
 * Copyright (c) CERN 2013-2017
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

#ifndef GRIDFTPWRAPPER_H
#define GRIDFTPWRAPPER_H

#include <gfal_api.h>
#include <exceptions/gfalcoreexception.hpp>
#include <time_utils.h>

#include <ctime>
#include <algorithm>
#include <map>
#include <memory>

#include <glib.h>

#include <globus_ftp_client.h>
#include <globus_gass_copy.h>


// Forward declarations
class GridFTPFactory;
class GridFTPSessionHandler;
class GridFTPSessionHandler;
class GridFTPRequestState;
class GridFTPStreamState;


enum GridFTPRequestStatus {
    GRIDFTP_REQUEST_NOT_LAUNCHED,
    GRIDFTP_REQUEST_RUNNING,
    GRIDFTP_REQUEST_FINISHED,
};

enum GridFTPRequestType {
    GRIDFTP_REQUEST_GASS, GRIDFTP_REQUEST_FTP
};



class GridFTPRequestState {
 public:
    GridFTPRequestState(GridFTPSessionHandler * s, GridFTPRequestType request_type = GRIDFTP_REQUEST_FTP);
	virtual ~GridFTPRequestState();

	void wait(GQuark scope, time_t timeout = -1);
	void cancel(GQuark scope, const std::string& msg);

	GridFTPSessionHandler* handler;
	GridFTPRequestType request_type;

    globus_mutex_t mutex;
    globus_cond_t cond;
    Gfal::CoreException* error;
    bool done;

    time_t default_timeout;
};

class GridFTPStreamState: public GridFTPRequestState {
public:
    off_t offset;    // file offset in the stream
    globus_size_t buffer_size;
	bool eof;        // end of file reached
	bool expect_eof; // true for partial reads, false for streamed reads

	GridFTPStreamState(GridFTPSessionHandler * s);
    virtual ~GridFTPStreamState();
};


struct GassCopyAttrHandler {
    GassCopyAttrHandler(globus_ftp_client_operationattr_t* ftp_operation_attr);
    ~GassCopyAttrHandler();
    globus_gass_copy_attr_t attr_gass;
    globus_ftp_client_operationattr_t operation_attr_ftp_for_gass;
    gss_cred_id_t cred_id;
};


class GridFTPSession {
public:
    GridFTPSession(gfal2_context_t context, const std::string& baseurl);
    ~GridFTPSession();

    std::string baseurl;

    gss_cred_id_t cred_id;
    globus_ftp_client_handle_t handle_ftp;
    globus_ftp_client_plugin_t debug_ftp_plugin;
    globus_ftp_client_handleattr_t attr_handle;
    globus_ftp_client_operationattr_t operation_attr_ftp;
    globus_gass_copy_handle_t gass_handle;
    globus_gass_copy_handleattr_t gass_handle_attr;
    globus_ftp_control_dcau_t dcau_control;
    globus_ftp_client_features_t ftp_features;

    // options
    globus_ftp_control_parallelism_t parallelism;
    globus_ftp_control_mode_t mode;
    globus_ftp_control_tcpbuffer_t tcp_buffer_size;

    // client plugins
    globus_ftp_client_plugin_t pasv_plugin;

    // pointers to the gfal2 context and transfer params, if relevant
    gfal2_context_t context;
    gfalt_params_t params;

    // Handy option setters
    void set_gridftpv2(bool v2);
    void set_ipv6(bool ipv6);
    void set_udt(bool udt);
    void set_dcau(bool dcau);
    void set_delayed_pass(bool enable);
    void set_nb_streams(unsigned int nbstreams);
    void set_tcp_buffer_size(guint64 tcp_buffer_size);

    void set_user_agent(gfal2_context_t context);
};


class GridFTPSessionHandler {
public:
    GridFTPSessionHandler(GridFTPFactory* f, const std::string &uri);
    ~GridFTPSessionHandler();

    globus_ftp_client_handle_t* get_ftp_client_handle();
    globus_gass_copy_handle_t* get_gass_copy_handle();
    globus_ftp_client_operationattr_t* get_ftp_client_operationattr();
    globus_gass_copy_handleattr_t* get_gass_copy_handleattr();
    globus_ftp_client_handleattr_t* get_ftp_client_handleattr();
    globus_ftp_client_features_t* get_ftp_features();

    GridFTPFactory* get_factory();

    GridFTPSession* session;

private:
    GridFTPFactory* factory;
};


class GridFTPFactory {
public:
    GridFTPFactory(gfal2_context_t handle);
    ~GridFTPFactory();

    /** Get a suitable session, new or reused
     **/
    GridFTPSession* get_session(const std::string &url);

    /** Release the session, and close it if should not be reused
     **/
    void release_session(GridFTPSession* h);

    gfal2_context_t get_gfal2_context();

private:
    gfal2_context_t gfal2_context;
    // session re-use management
    bool session_reuse;
    unsigned int size_cache;
    // session cache
    std::multimap<std::string, GridFTPSession*> session_cache;
    globus_mutex_t mux_cache;

    void recycle_session(GridFTPSession* sess);
    void clear_cache();
    GridFTPSession* get_recycled_handle(const std::string &baseurl);
    GridFTPSession* get_new_handle(const std::string &baseurl);
};


void globus_ftp_client_done_callback(void * user_arg,
        globus_ftp_client_handle_t * handle, globus_object_t * error);

void globus_gass_client_done_callback(
        void * callback_arg,
        globus_gass_copy_handle_t * handle,
        globus_object_t * error);

// do atomic read operation from globus async call
ssize_t gridftp_read_stream(GQuark scope,
        GridFTPStreamState* stream, void* buffer, size_t s_read,
        bool expect_eof);

// do atomic write operation from globus async call
ssize_t gridftp_write_stream(GQuark scope,
        GridFTPStreamState* stream, const void* buffer, size_t s_write,
        bool eof);


// return 0 if no error, or return errno and set the error string properly
// error allocation is dynamic
int gfal_globus_error_convert(globus_object_t * error, char ** str_error);

// throw Glib::Error if error associated with this result
void gfal_globus_check_result(GQuark scope, globus_result_t res);

void gfal_globus_set_credentials(const char* ucert, const char* ukey,
    const char *user, const char *passwd,
    gss_cred_id_t *cred_id,
    globus_ftp_client_operationattr_t* opattr);

#endif /* GRIDFTPWRAPPER_H */
