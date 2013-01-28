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
#include <utils/uri_util.h>
#include <config/gfal_config.h>
#include <cancel/gfal_cancel.h>
#include "gridftpwrapper.h"

const char* gridftp_version_config = "GRIDFTP_V2";
const char* gridftp_session_reuse_config= "SESSION_REUSE";
const char* gridftp_dcau_config= "DCAU";
const char* gridftp_ipv6_config= "IPV6";

const Glib::Quark scope_readder("gfal_griftp_stream_read");
const Glib::Quark scope_request("GridftpModule::request");

struct RwStatus{
	off_t init;
	off_t finish;
	bool ops_state;
};



struct Gass_attr_handler_implem : public Gass_attr_handler{
    Gass_attr_handler_implem(globus_ftp_client_operationattr_t* ftp_operation_attr){
        // initialize gass copy attr
        globus_result_t res = globus_gass_copy_attr_init(&(attr_gass));
        gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);
        globus_ftp_client_operationattr_copy(&(operation_attr_ftp_for_gass), ftp_operation_attr);
         res = globus_gass_copy_attr_set_ftp (&(attr_gass), &operation_attr_ftp_for_gass);
        gfal_globus_check_result("GridFTPFactory::globus_gass_copy_handleattr_set_ftp_attr", res);

    }

    virtual ~Gass_attr_handler_implem(){
        // initialize gass copy attr
        globus_ftp_client_operationattr_destroy(&(operation_attr_ftp_for_gass));
    }

};

struct GridFTP_session_implem : public GridFTP_session{
	
	void init(){
		_sess = new Session_handler();
		globus_result_t res;

        // init debug plugin
        res= globus_ftp_client_debug_plugin_init(&(_sess->debug_ftp_plugin), stderr, "gridftp debug :");
        gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_ops_attr", res);

		// init operation attr
		res= globus_ftp_client_operationattr_init(&(_sess->operation_attr_ftp)); 	
		gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_ops_attr", res);	
			
		
		// initialize ftp attributes
		res = globus_ftp_client_handleattr_init(&(_sess->attr_handle));
		gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle_attr", res);	
		configure_gridftp_handle_attr();
		
		// create gass handle attribute
		res =	globus_gass_copy_handleattr_init(&(_sess->gass_handle_attr));
		gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);		
			
		// associate ftp attributes to gass attributes
		res = globus_gass_copy_handleattr_set_ftp_attr (&(_sess->gass_handle_attr), &(_sess->attr_handle));
		gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);			
		
		// initialize gass handle 
		res = globus_gass_copy_handle_init (&(_sess->gass_handle), &(_sess->gass_handle_attr));
		gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);			
		
        configure_default_stream_attributes();
        apply_default_stream_attribute();
	}
	
	void configure_gridftp_handle_attr(){
		globus_ftp_client_handleattr_set_cache_all(&(_sess->attr_handle), GLOBUS_TRUE);	// enable session re-use
        if(gfal_get_verbose() & GFAL_VERBOSE_TRACE_PLUGIN){
            globus_ftp_client_handleattr_add_plugin(&(_sess->attr_handle), &(_sess->debug_ftp_plugin));
        }
	}

    void configure_default_stream_attributes(){
         _sess->parall.fixed.size = 1;
         _sess->parall.mode = GLOBUS_FTP_CONTROL_PARALLELISM_NONE;
         _sess->mode = GLOBUS_FTP_CONTROL_MODE_NONE;
    }

    void apply_default_stream_attribute(){
        globus_ftp_client_operationattr_set_mode(&(_sess->operation_attr_ftp), _sess->mode);
        globus_ftp_client_operationattr_set_parallelism(&(_sess->operation_attr_ftp),&(_sess->parall));
    }

    void apply_default_tcp_buffer_attributes(){
        globus_ftp_client_operationattr_set_tcp_buffer(&(_sess->operation_attr_ftp), &(_sess->tcp_buffer_size));
    }
	
	void set_gridftpv2(bool v2){
		globus_ftp_client_handleattr_set_gridftp2(&(_sess->attr_handle), v2); // define gridftp 2	
	}

    void set_ipv6(bool enable){
        globus_ftp_client_operationattr_set_allow_ipv6(&(_sess->operation_attr_ftp), (globus_bool_t) enable);
    }

    void set_dcau(const globus_ftp_control_dcau_t & _dcau ){
        _sess->dcau.mode = _dcau.mode;
        globus_ftp_client_operationattr_set_dcau(&(_sess->operation_attr_ftp), &(_sess->dcau));
    }

    void set_nb_stream(const unsigned int nbstream){
        if(nbstream == 0){
            configure_default_stream_attributes();
        }else{
            _sess->parall.fixed.size = nbstream;
            _sess->parall.mode = GLOBUS_FTP_CONTROL_PARALLELISM_FIXED;
            _sess->mode = GLOBUS_FTP_CONTROL_MODE_EXTENDED_BLOCK;
        }
        apply_default_stream_attribute();
    }
	

    virtual void set_tcp_buffer_size(const guint64 tcp_buffer_size){
        if(tcp_buffer_size == 0){
            _sess->tcp_buffer_size.mode = GLOBUS_FTP_CONTROL_TCPBUFFER_DEFAULT;
        }else{
            _sess->tcp_buffer_size.mode = GLOBUS_FTP_CONTROL_TCPBUFFER_FIXED;
            _sess->tcp_buffer_size.fixed.size = tcp_buffer_size;
        }
    }

	GridFTP_session_implem(GridFTPFactory* f, const std::string & hostname){
		this->factory = f;
		this->hostname = hostname;
		init();	
	}
	
	GridFTP_session_implem( GridFTP_session_implem *src){
		this->factory = src->factory;
		this->hostname = src->hostname;
		this->_sess = src->_sess;

	}
	
	virtual ~GridFTP_session_implem(){
        if(_sess != NULL){
            clean();
			factory->gfal_globus_ftp_release_handle_internal(this);
        }
	}
	
	virtual globus_ftp_client_handle_t* get_ftp_handle(){
		globus_result_t res = globus_gass_copy_get_ftp_handle(&(_sess->gass_handle), &(_sess->handle_ftp));
		gfal_globus_check_result("GridFTPFactory::GridFTP_session_implem", res);			
		return &(_sess->handle_ftp);
	}

	virtual globus_gass_copy_handle_t* get_gass_handle(){
		return &(_sess->gass_handle);
	}	
	
	virtual globus_ftp_client_operationattr_t* get_op_attr_ftp(){
		return &(_sess->operation_attr_ftp);
	}	
	
	virtual globus_gass_copy_handleattr_t* get_gass_handle_attr(){
		return &(_sess->gass_handle_attr);
	}

    virtual Gass_attr_handler* generate_gass_copy_attr(){
        Gass_attr_handler_implem* res = new Gass_attr_handler_implem(&(_sess->operation_attr_ftp));
        return res;
    }

    virtual void clean(){
        // clean performance markers
        globus_result_t res = globus_gass_copy_register_performance_cb(&(_sess->gass_handle),
                NULL,NULL);
        gfal_globus_check_result("GridFTPFactory::GridFTP_session_implem", res);
        configure_default_stream_attributes();
    }
	
	virtual void purge(){
        globus_ftp_client_debug_plugin_destroy(&(_sess->debug_ftp_plugin)); // destruct the debug plugin
		globus_gass_copy_handle_destroy(&(_sess->gass_handle));
		globus_ftp_client_operationattr_destroy (&(_sess->operation_attr_ftp));
		globus_gass_copy_handleattr_destroy(&(_sess->gass_handle_attr));	
        globus_ftp_client_handleattr_destroy(&(_sess->attr_handle));
		delete _sess;
		_sess = NULL;		
	}	
	
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
        globus_ftp_control_parallelism_t parall;
        globus_ftp_control_mode_t  	mode;
        globus_ftp_control_tcpbuffer_t tcp_buffer_size;
	};
	
	Session_handler* _sess;
	// internal fields	
	std::string hostname;		
	GridFTPFactory* factory;
};


GridFTPFactory::GridFTPFactory(gfal_handle handle) : _handle(handle)
{
    GError * tmp_err=NULL;
    session_reuse = gfal2_get_opt_boolean(_handle, GRIDFTP_CONFIG_GROUP, gridftp_session_reuse_config, &tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE," define GSIFTP session re-use to %s",(session_reuse)?"TRUE":"FALSE");
    if(tmp_err)
        throw Glib::Error(tmp_err);
	size_cache = 400;
}



GridFTP_Request_state::GridFTP_Request_state(GridFTP_session * s, bool own_session, GridFtp_request_type request_type) : sess(s), end_time(0,0){
	req_status=GRIDFTP_REQUEST_NOT_LAUNCHED;
    this->own_session = own_session;
    this->request_type = request_type;
    canceling = false;
}

GridFTP_Request_state::~GridFTP_Request_state()
{
    Glib::RWLock::WriterLock l(mux_req_state);
	if(!own_session)
		sess.release(); // cancel the automatic memory management
}


static void gfal_gridftp_cancel_slot(gfal2_context_t context, void* userdata){
    GridFTP_Request_state* r = (GridFTP_Request_state*) userdata;
    r->cancel_operation_async(g_quark_to_string(gfal_cancel_quark()), "Operation canceled from gfal2_cancel");
}

GridFTPOperationCanceler::GridFTPOperationCanceler(gfal2_context_t context, GridFTP_Request_state *state) :
    _state(state),
    _cancel_token(gfal2_register_cancel_callback(context, &gfal_gridftp_cancel_slot, state)),
    _context(context)
{
}

GridFTPOperationCanceler::~GridFTPOperationCanceler()
{
    gfal2_remove_cancel_callback(_context, _cancel_token);

}

void GridFTPFactory::clear_cache(){
		gfal_log(GFAL_VERBOSE_TRACE, "gridftp session cache garbage collection ...");	
		std::multimap<std::string, GridFTP_session*>::iterator it;
		for(it = sess_cache.begin(); it != sess_cache.end(); ++it){
			GridFTP_session_implem *sess = static_cast<GridFTP_session_implem *>((*it).second);
			sess->purge();
			delete sess;
		}
		sess_cache.clear();
}

void GridFTPFactory::recycle_session(GridFTP_session* sess){
	
	Glib::Mutex::Lock l(mux_cache);
	
	GridFTP_session_implem * my_sess = static_cast<GridFTP_session_implem *>(sess);
	const char* c_hostname = my_sess->hostname.c_str();
	
	if(sess_cache.size() > size_cache)
		clear_cache();	

	gfal_log(GFAL_VERBOSE_TRACE, "insert gridftp session for %s in cache ...", c_hostname);				
	sess_cache.insert(std::pair<std::string,GridFTP_session*>(c_hostname, new GridFTP_session_implem(my_sess )));
}


// recycle a gridftp session object from cache if exist, return NULL else
GridFTP_session* GridFTPFactory::get_recycled_handle(const std::string & hostname){
	Glib::Mutex::Lock l(mux_cache);	
    GridFTP_session* res= NULL;
    std::multimap<std::string, GridFTP_session*>::iterator it=sess_cache.find(hostname); // try to find a session explicitely associated with this handle
    if(it == sess_cache.end()){ // if no session found, take a generic one
        gfal_log(GFAL_VERBOSE_TRACE, "recycled unamed generic session found .... ");
        it = sess_cache.begin();
    }
    if(it != sess_cache.end()){
        gfal_log(GFAL_VERBOSE_TRACE, "gridftp session for %s found in  cache !", hostname.c_str());
        res = (*it).second;
        sess_cache.erase(it);
    }else{
        gfal_log(GFAL_VERBOSE_TRACE, "no session found in cache for %s!", hostname.c_str());
    }
    return res;
}

GridFTPFactory::~GridFTPFactory()
{
    Glib::Mutex::Lock l(mux_cache);
	clear_cache();
}

gfal_handle GridFTPFactory::get_handle(){
	return _handle;
}


/*
 *  dirty function to convert error code from globus
 *  In the current state, globus provides no way to convert gridftp error code to errno properly....
 * */
static int scan_errstring(const char *p) {

    int ret = ECOMM;
    if (p == NULL) return ret;

    if (strstr(p, "o such file") || strstr(p, "File not found"))
        ret = ENOENT;
    else if (strstr(p, "ermission denied") || strstr(p, "credential"))
        ret = EACCES;
    else if (strstr(p, "exists"))
        ret = EEXIST;
    else if (strstr(p, "ot a direct"))
		ret = ENOTDIR;
    else if (strstr(p, "ation not sup"))
        ret = ENOTSUP;
    else if (strstr(p, "Login incorrect") || strstr(p, "Could not get virtual id"))
        ret = EACCES;
    return ret;
}

int gfal_globus_error_convert(globus_object_t * error, char ** str_error){
	if(error){
		*str_error = globus_error_print_friendly(error);
		char * p = *str_error;
		while(*p != '\0'){ // string normalization of carriage return
			*p = (*p == '\n' || *p == '\r')?' ':*p;
			++p;
		}
		return scan_errstring(*str_error); // try to get errno	
	}
	return 0;
}

void gfal_globus_check_result(const Glib::Quark & scope, gfal_globus_result_t res){
	if(res != GLOBUS_SUCCESS){

		globus_object_t * error=globus_error_get(res); // get error from result code 
		if(error == NULL)
			throw Gfal::CoreException(scope, "Unknow error  unable to map result code to globus error", ENOENT);
		
		gfal_globus_check_error(scope, error);
	}
}

void gfal_globus_check_error(const Glib::Quark & scope,  globus_object_t *	error){
	if(error != GLOBUS_SUCCESS){
		int globus_errno;	
		char errbuff[GFAL_URL_MAX_LEN];
		char * glob_str=NULL;		
		*errbuff='\0';
		
		globus_errno = gfal_globus_error_convert(error, &glob_str);
		if(glob_str){ // security
			g_strlcpy( errbuff, glob_str, GFAL_URL_MAX_LEN);
			g_free(glob_str);
		}
		globus_object_free(error);
		throw Gfal::CoreException(scope, errbuff, globus_errno );		
	}
}


GridFTP_session* GridFTPFactory::get_new_handle(const std::string & hostname){
    GError * tmp_err=NULL;
    globus_ftp_control_dcau_t dcau_param;
    bool gridftp_v2 = gfal2_get_opt_boolean(_handle, GRIDFTP_CONFIG_GROUP, gridftp_version_config, &tmp_err);
    if(tmp_err)
        throw Glib::Error(tmp_err);

    bool ipv6 = gfal2_get_opt_boolean_with_default(_handle, GRIDFTP_CONFIG_GROUP, gridftp_ipv6_config, false);

    dcau_param.mode = (gfal2_get_opt_boolean(_handle, GRIDFTP_CONFIG_GROUP, gridftp_dcau_config, &tmp_err))?GLOBUS_FTP_CONTROL_DCAU_DEFAULT:GLOBUS_FTP_CONTROL_DCAU_NONE;
    if(tmp_err)
        throw Glib::Error(tmp_err);

	std::auto_ptr<GridFTP_session_implem> sess(new GridFTP_session_implem(this, hostname));		

	sess->set_gridftpv2(gridftp_v2);
    sess->set_dcau(dcau_param);
    sess->set_ipv6(ipv6);
	return sess.release();
}

// store the related globus error to the current handle
void gfal_globus_store_error(GridFTP_Request_state * state, globus_object_t *error){
		char * glob_str=NULL;	
		state->set_error_code(gfal_globus_error_convert(error, &glob_str));
		if(glob_str){
            if(state->get_error().empty())
                state->set_error(glob_str);
			g_free(glob_str);
		}else{
			state->set_error("Uknow Globus Error, bad error report");
			state->set_error_code(EFAULT);
		}
}

GridFTP_session* GridFTPFactory::gfal_globus_ftp_take_handle(const std::string & hostname){
	GridFTP_session * res = NULL;
	if( (res = get_recycled_handle(hostname)) == NULL)
		res = get_new_handle(hostname);
	return res;
}

void GridFTPFactory::gfal_globus_ftp_release_handle_internal(GridFTP_session* sess){
    session_reuse = gfal2_get_opt_boolean(_handle, GRIDFTP_CONFIG_GROUP, gridftp_session_reuse_config, NULL);
	if(session_reuse)
		recycle_session(sess);
	else{
		GridFTP_session_implem * s = static_cast<GridFTP_session_implem *>(sess);
		s->purge();
	}
}


void GridFTPFactory::gfal_globus_ftp_release_handle(GridFTP_session* h){
	delete h;
}

static void gfal_globus_prototype_callback(void* user_args, globus_object_t *error){
    GridFTP_Request_state* state = (GridFTP_Request_state*) user_args;
    Glib::RWLock::ReaderLock l (state->mux_req_state);
    Glib::Mutex::Lock l_call(state->mux_callback_lock);


    if(state->get_req_status() == GRIDFTP_REQUEST_FINISHED){
        gfal_log(GFAL_VERBOSE_TRACE,"gridFTP operation already finished ! error !");
    }else{
        if(error != GLOBUS_SUCCESS){
            gfal_globus_store_error(state, error);
        }else if( state->canceling  == FALSE ){
            state->set_error_code(0);
        }
        state->set_req_status(GRIDFTP_REQUEST_FINISHED);
        state->signal_callback_main.broadcast();
    }
}

// gridftp callback generic implementation
void globus_basic_client_callback (void * user_arg, 
				globus_ftp_client_handle_t *		handle,
				globus_object_t *				error){
    gfal_log(GFAL_VERBOSE_TRACE," gridFTP operation done");
    gfal_globus_prototype_callback(user_arg, error);
}

// gass operation callback implementation
void globus_gass_basic_client_callback(
        void * callback_arg,
        globus_gass_copy_handle_t * handle,
        globus_object_t * error){
    gfal_log(GFAL_VERBOSE_TRACE,"gass operation done");
    gfal_globus_prototype_callback( callback_arg, error);
}


void GridFTP_Request_state::poll_callback(const Glib::Quark &scope){
    gfal_log(GFAL_VERBOSE_TRACE," -> go internal polling for request ");
    bool timeout= false;
    Glib::RWLock::ReaderLock l(mux_req_state);
    {

        Glib::Mutex::Lock l(mux_callback_lock);
        // wait for a globus signal or for a timeout
        // if canceling logic -> wait until end
        while(this->req_status != GRIDFTP_REQUEST_FINISHED
                && (timeout == FALSE || this->canceling == TRUE)){
            if(end_time == Glib::TimeVal(0,0) || this->canceling == TRUE){
                signal_callback_main.wait(mux_callback_lock);
           } else{
                timeout = !(signal_callback_main.timed_wait(mux_callback_lock,end_time));
            }
        }
    }

    if(timeout && this->canceling==FALSE){
       gfal_log(GFAL_VERBOSE_TRACE,"gfal gridftp operation timeout occures ! cancel the operation ...");
       cancel_operation(scope, "gfal gridftp internal operation timeout, operation canceled");
       this->set_error_code(ETIMEDOUT);
    }

    gfal_log(GFAL_VERBOSE_TRACE," <- out of gass polling for request ");
}

void GridFTP_Request_state::err_report(const Glib::Quark &scope){
    if(this->errcode != 0)
        throw Gfal::CoreException(scope,  this->error, this->errcode);
}


void GridFTP_Request_state::wait_callback(const Glib::Quark &scope){
    poll_callback(scope);
    err_report(scope);
}

void GridFTP_Request_state::cancel_operation(const Glib::Quark &scope, const std::string &msg){
    cancel_operation_async(scope, msg);
    this->poll_callback(scope);

}

void GridFTP_Request_state::cancel_operation_async(const Glib::Quark &scope, const std::string & msg){
    globus_result_t res;
    Glib::RWLock::ReaderLock l(this->mux_req_state);
    Glib::Mutex::Lock l_call(this->mux_callback_lock);
    this->canceling = TRUE;
    if(this->get_req_status() == GRIDFTP_REQUEST_FINISHED) // already finished before cancelling -> return
        return;

    if(this->request_type == GRIDFTP_REQUEST_GASS){
        gfal_log(GFAL_VERBOSE_TRACE," -> gass operation cancel  ");
        res = globus_gass_copy_cancel(this->sess->get_gass_handle(),
                                                      globus_gass_basic_client_callback,
                                                      this);
        gfal_log(GFAL_VERBOSE_TRACE,"    gass operation cancel <-");
    }else{

        res = globus_ftp_client_abort(this->sess->get_ftp_handle());
    }
    gfal_globus_check_result(scope, res);
    this->set_error_code(ECANCELED);
    this->set_error(msg);
}

void GridFTP_stream_state::poll_callback_stream(const Glib::Quark & scope){
    gfal_log(GFAL_VERBOSE_TRACE," -> go polling for request ");
    while(this->stream_status != GRIDFTP_REQUEST_FINISHED )
            usleep(10);
    gfal_log(GFAL_VERBOSE_TRACE," <- out of polling for request ");
}

void GridFTP_stream_state::wait_callback_stream(const Glib::Quark & scope){
    poll_callback_stream(scope);
    err_report(scope);
}





void gridftp_wait_for_read(const Glib::Quark & scope, GridFTP_stream_state* state, off_t end_read){
    state->wait_callback_stream(scope);
}

void gridftp_wait_for_write(const Glib::Quark & scope, GridFTP_stream_state* state, off_t end_write){
    state->wait_callback_stream(scope);
}

static void gfal_griftp_stream_read_callback(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t *error, globus_byte_t *buffer,
				globus_size_t length, globus_off_t offset, globus_bool_t eof){

	GridFTP_stream_state* state = static_cast<GridFTP_stream_state*>(user_arg);
	
	
	if(error != GLOBUS_SUCCESS){	// check error status
		gfal_globus_store_error(state, error);
	   // gfal_log(GFAL_VERBOSE_TRACE," read error %s , code %d", state->error, state->errcode);					
	}else{
		// verify read 
		//gfal_log(GFAL_VERBOSE_TRACE," read %d bytes , eof %d %d,%d", length, eof, state->offset, offset);			
		if(state->get_offset() != offset){
			state->set_error(" Invalid read callback call from globus, out of order");
			state->set_error_code(EIO);
		}else{
			state->increase_offset(length);
			state->set_eof(eof);
			state->set_error_code(0);
		}		
	}
    state->set_stream_status(GRIDFTP_REQUEST_FINISHED);
}


static void gfal_griftp_stream_write_callback(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t *error, globus_byte_t *buffer,
				globus_size_t length, globus_off_t offset, globus_bool_t eof){

	GridFTP_stream_state* state = static_cast<GridFTP_stream_state*>(user_arg);
	
	
	if(error != GLOBUS_SUCCESS){	// check error status
		gfal_globus_store_error(state, error);
	   // gfal_log(GFAL_VERBOSE_TRACE," read error %s , code %d", state->error, state->errcode);					
	}else{
		// verify write 
		//gfal_log(GFAL_VERBOSE_TRACE," read %d bytes , eof %d %d,%d", length, eof, state->offset, offset);			
		if(state->get_offset() != offset){
			state->set_error(" Invalid write callback call from globus, out of order");
			state->set_error_code(EIO);
		}else{
			state->increase_offset(length);
			state->set_eof(eof);
			state->set_error_code(0);
		}		
	}
    state->set_stream_status(GRIDFTP_REQUEST_FINISHED);
}

ssize_t gridftp_read_stream(const Glib::Quark & scope,
							GridFTP_stream_state* stream,
							void* buffer, size_t s_read
							){
	gfal_log(GFAL_VERBOSE_TRACE,"  -> [gridftp_read_stream]");	
	off_t initial_offset = stream->get_offset();
	
    if(stream->is_eof())
        return 0;
	globus_result_t res = globus_ftp_client_register_read( stream->sess->get_ftp_handle(),
		(globus_byte_t*) buffer,
		s_read,
		gfal_griftp_stream_read_callback,
		stream 
	); 		
	gfal_globus_check_result(scope, res);	
	gridftp_wait_for_read(scope, stream, initial_offset + s_read);	
    stream->set_stream_status(GRIDFTP_REQUEST_NOT_LAUNCHED);
	return stream->get_offset() - initial_offset;							
}


ssize_t gridftp_write_stream(const Glib::Quark & scope,
							GridFTP_stream_state* stream,
							const void* buffer, size_t s_write,
							bool eof
							){
	gfal_log(GFAL_VERBOSE_TRACE,"  -> [gridftp_write_stream]");	
	off_t initial_offset = stream->get_offset();
	
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


std::string gridftp_hostname_from_url(const char * url){
	GError * tmp_err=NULL;
	char buffer[GFAL_URL_MAX_LEN];
	const int res = gfal_hostname_from_uri(url, buffer, GFAL_URL_MAX_LEN, &tmp_err);
	if(res < 0)
		throw Glib::Error(tmp_err);
	return std::string(buffer);
}





