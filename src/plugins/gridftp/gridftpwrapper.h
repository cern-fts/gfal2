#ifndef GRIDFTPWRAPPER_H
#define GRIDFTPWRAPPER_H
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

#include <gfal_api.h>
#include <time_utils.h>

#include <ctime>
#include <algorithm>
#include <memory>

#include <glib.h>
#include <glibmm.h>

#include <globus_ftp_client.h>
#include <globus_gass_copy.h>


#define GRIDFTP_CONFIG_GROUP "GRIDFTP PLUGIN"

enum GridFTPRequestStatus{
	GRIDFTP_REQUEST_NOT_LAUNCHED,
	GRIDFTP_REQUEST_RUNNING,
	GRIDFTP_REQUEST_FINISHED,
};

enum GridFTPRequestType{
    GRIDFTP_REQUEST_GASS,
    GRIDFTP_REQUEST_FTP
};

class GridFTPFactory;
class GridFTPSession;


struct GridFTPRequestState {
 protected:
   Glib::Mutex internal_lock;
   
   int         errcode;
   std::string error;
   
   GridFTPRequestStatus req_status;
   
   inline void init_timeout(struct timespec * time_offset){
       if( time_offset  && timespec_isset(time_offset)){
           end_time.assign_current_time();
           end_time.add_seconds(time_offset->tv_sec);
           end_time.add_microseconds(time_offset->tv_nsec/1000);
       }else{
         end_time = Glib::TimeVal(0,0);
       }
   }

   inline void set_end_time(struct timespec * my_end_time){
       if(my_end_time && timespec_isset(my_end_time)){
           end_time = Glib::TimeVal(my_end_time->tv_sec, my_end_time->tv_nsec/1000);
       }else{
         end_time = Glib::TimeVal(0,0);
       }
   }

   void poll_callback(const Glib::Quark & scope);

 public:
    GridFTPRequestState(GridFTPSession * s, bool own_session=true,  GridFTPRequestType request_type = GRIDFTP_REQUEST_FTP);
	virtual ~GridFTPRequestState();

    // cancel token
    gfal_cancel_token_t cancel_token;
    // ftp session
	std::auto_ptr<GridFTPSession> sess;  
    // request type
    GridFTPRequestType request_type;   
    // params
    Glib::TimeVal end_time; // timeout trigger -> 0 if not enabled, usage of CLOCK_MONOTONIC is required.
    // enable/disable destroy when out of scope
    bool own_session;
    // bool canceling
    bool canceling;
    // mutex for state checking
    Glib::RWLock mux_req_state;
    Glib::Mutex mux_callback_lock;
    Glib::Cond signal_callback_main;

    inline void start(){
        this->req_status = GRIDFTP_REQUEST_RUNNING;
    }
    
    inline int get_error_code(void) {
      Glib::Mutex::Lock locker(internal_lock);
      return errcode;
    }
    
    inline void set_error_code(int code) {
      Glib::Mutex::Lock locker(internal_lock);
      errcode = code;
    }
    
    inline std::string get_error(void) {
      Glib::Mutex::Lock locker(internal_lock);
      return error;
    }
    
    inline void set_error(const std::string & errstr) {
      Glib::Mutex::Lock locker(internal_lock);
      error = errstr;
    }
    
    inline GridFTPRequestStatus get_req_status(void) {
      Glib::Mutex::Lock locker(internal_lock);
      return  req_status;
    }
    
    inline void set_req_status(const GridFTPRequestStatus & st) {
      Glib::Mutex::Lock locker(internal_lock);
      req_status = st;
    }

    void wait_callback(const Glib::Quark & scope, time_t timeout = 300);

    void err_report(const Glib::Quark &scope);

    void cancel_operation(const Glib::Quark &scope, const std::string & msg = "");

    int cancel_operation_async(const Glib::Quark &scope, const std::string & msg = "");
};

struct GridFTPStreamState : public GridFTPRequestState{
 protected:
    off_t offset; // file offset in the stream
	bool eof;     // end of file reached
    GridFTPRequestStatus stream_status;
    Glib::Mutex mux_stream_callback;
    Glib::Cond  cond_stream_callback;
    
 public:
   
   // ownership lock
   Glib::Mutex lock;
   
	GridFTPStreamState(GridFTPSession * s) : GridFTPRequestState(s) {
		offset =0;
		eof = false;
        stream_status = GRIDFTP_REQUEST_NOT_LAUNCHED;
	}

    virtual ~GridFTPStreamState();
	
	bool finished(){
        Glib::Mutex::Lock locker(internal_lock);
        return (stream_status==GRIDFTP_REQUEST_FINISHED);
	}
    
    inline bool is_eof(void) {
        Glib::Mutex::Lock locker(internal_lock);
        return eof;
    }
    
    inline void set_eof(bool end) {
        Glib::Mutex::Lock locker(internal_lock);
        eof = end;
    }
    
    inline off_t get_offset(void) {
        Glib::Mutex::Lock locker(internal_lock);
        return offset;
    }
       
    void set_offset(off_t off) {
        Glib::Mutex::Lock locker(internal_lock);
        offset = off;
    }
    
    void increase_offset(off_t diff) {
        Glib::Mutex::Lock locker(internal_lock);
        offset += diff;
    }
    
    GridFTPRequestStatus get_stream_status(void) {
        Glib::Mutex::Lock locker(internal_lock);
        return stream_status;
    }
    
    void set_stream_status(const GridFTPRequestStatus & st) {
        Glib::Mutex::Lock locker(internal_lock);
        stream_status = st;
    }

    void poll_callback_stream(const Glib::Quark & scope);
    void wait_callback_stream(const Glib::Quark & scope);

    friend void gfal_stream_callback_prototype(void *user_arg, globus_ftp_client_handle_t *handle,
            globus_object_t *error, globus_byte_t *buffer, globus_size_t length,
            globus_off_t offset, globus_bool_t eof, const char* err_msg_offset);
};


class GridFTPOperationCanceler{
public:
    GridFTPOperationCanceler(gfal2_context_t context, GridFTPRequestState* state);
    ~GridFTPOperationCanceler();

private:
    GridFTPRequestState* _state;
    gfal_cancel_token_t _cancel_token;
    gfal2_context_t _context;
};


struct Gass_attr_handler {
    Gass_attr_handler(globus_ftp_client_operationattr_t* ftp_operation_attr);
    ~Gass_attr_handler();
    globus_gass_copy_attr_t attr_gass;
    globus_ftp_client_operationattr_t operation_attr_ftp_for_gass;
};


class GridFTPSession {
public:
    GridFTPSession(GridFTPFactory* f, const std::string & thostname);

    GridFTPSession(GridFTPSession *src);
    ~GridFTPSession();

    globus_ftp_client_handle_t* get_ftp_handle();
    globus_gass_copy_handle_t* get_gass_handle();
    globus_ftp_client_operationattr_t* get_op_attr_ftp();
    globus_gass_copy_handleattr_t* get_gass_handle_attr();
    Gass_attr_handler* generate_gass_copy_attr();

    void set_nb_stream(const unsigned int nbstream);
    void set_tcp_buffer_size(const guint64 tcp_buffer_size);
    void enable_udt();
    void disable_udt();

    void disable_reuse();

private:
    void init();

    void configure_gridftp_handle_attr();
    void configure_default_stream_attributes();
    void apply_default_stream_attribute();
    void apply_default_tcp_buffer_attributes();
    void set_gridftpv2(bool v2);
    void set_ipv6(bool enable);
    void set_delayed_pass(bool enable);
    void set_dcau(const globus_ftp_control_dcau_t & _dcau );

    void set_credentials(const char* ucert, const char* ukey);

    void clean();
    void purge();

    bool _isDirty;
    // handle ftp

    struct Session_handler{
        globus_ftp_client_handle_t handle_ftp;
        globus_ftp_client_plugin_t debug_ftp_plugin;
        globus_ftp_client_handleattr_t attr_handle;
        globus_ftp_client_operationattr_t operation_attr_ftp;
        globus_gass_copy_handle_t gass_handle;
        globus_gass_copy_handleattr_t gass_handle_attr;
        globus_ftp_control_dcau_t dcau;

        // options
        globus_ftp_control_parallelism_t parallelism;
        globus_ftp_control_mode_t mode;
        globus_ftp_control_tcpbuffer_t tcp_buffer_size;
    };


    // internal fields
    GridFTPFactory* factory;
    std::string hostname;

    // sess
    Session_handler* _sess;

    friend class GridFTPFactory;
};


class GridFTPFactory
{
	public:
		GridFTPFactory(gfal2_context_t handle );
		virtual ~GridFTPFactory();

		/**
		 * provide session handle for most of the operations
		 * 
		 * */
		virtual  GridFTPSession* gfal_globus_ftp_take_handle(const std::string & hostname);
		/**
		 * destruct an existing session handle, close the connection and desallocate memory
		 * 
		 * */		
		virtual void gfal_globus_ftp_release_handle(GridFTPSession* h) ;
		
		virtual gfal2_context_t get_handle();

	private:
		gfal2_context_t _handle;
        // session re-use management
		bool session_reuse;
		unsigned int size_cache;
		// session cache
		std::multimap<std::string, GridFTPSession*> sess_cache;
		Glib::Mutex mux_cache;
		void recycle_session(GridFTPSession* sess);
		void clear_cache();
		GridFTPSession* get_recycled_handle(const std::string & hostname);
		GridFTPSession* get_new_handle(const std::string & hostname);
		
		void gfal_globus_ftp_release_handle_internal(GridFTPSession* sess);

		void configure_gridftp_handle_attr(globus_ftp_client_handleattr_t * attrs);
	
	friend class GridFTPSession;
};

void globus_basic_client_callback (void * user_arg, 
				globus_ftp_client_handle_t *		handle,
				globus_object_t *				error);


void globus_gass_basic_client_callback(
        void * callback_arg,
        globus_gass_copy_handle_t * handle,
        globus_object_t * error);

// do atomic read operation from globus async call
ssize_t gridftp_read_stream(const Glib::Quark & scope, GridFTPStreamState* stream,
				void* buffer, size_t s_read);	
				
// do atomic write operation from globus async call
ssize_t gridftp_write_stream(const Glib::Quark & scope, GridFTPStreamState* stream,
				const void* buffer, size_t s_write, bool eof);


// return 0 if no error, or return errno and set the error string properly
// error allocation is dynamic
int gfal_globus_error_convert(globus_object_t * error, char ** str_error);

// throw Glib::Error if error associated with this result
void gfal_globus_check_result(const Glib::Quark & scope, globus_result_t res);

// throw Glib::Error if error is present
void gfal_globus_check_error(const Glib::Quark & scope,  globus_object_t *	error);

std::string gridftp_hostname_from_url(const char * url);

#endif /* GRIDFTPWRAPPER_H */ 
