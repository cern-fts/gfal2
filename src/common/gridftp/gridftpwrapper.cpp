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
#include "gridftpwrapper.h"

const char* gridftp_version_config = "GRIDFTP_V2";
const char* gridftp_session_reuse_config= "SESSION_REUSE";
const char* gridftp_dcau_config= "DCAU";

const Glib::Quark scope_readder("gfal_griftp_stream_read");
const Glib::Quark scope_request("GridftpModule::request");

struct RwStatus{
	off_t init;
	off_t finish;
	bool ops_state;
};

struct GridFTP_session_implem : public GridFTP_session{
	
	void init(){
		_sess = new Session_handler();
		globus_result_t res;
		
		// init operation attr
		res= globus_ftp_client_operationattr_init(&(_sess->operation_attr_ftp)); 	
		gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_ops_attr", res);	
			
		// initialize gass copy attr
		res = globus_gass_copy_attr_init(&(_sess->attr_gass));
        gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);

        // associate ftp operation attr to gass
        res = globus_gass_copy_attr_set_ftp (&(_sess->attr_gass), &(_sess->operation_attr_ftp));
        gfal_globus_check_result("GridFTPFactory::globus_gass_copy_handleattr_set_ftp_attr", res);
		
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
		
	}
	
	void configure_gridftp_handle_attr(){
		globus_ftp_client_handleattr_set_cache_all(&(_sess->attr_handle), GLOBUS_TRUE);	// enable session re-use
	}

	
	void set_gridftpv2(bool v2){
		globus_ftp_client_handleattr_set_gridftp2(&(_sess->attr_handle), v2); // define gridftp 2	
	}

    void set_dcau(const globus_ftp_control_dcau_t & dcau ){
        globus_ftp_client_operationattr_set_dcau(&(_sess->operation_attr_ftp), &dcau);
        globus_result_t res = globus_gass_copy_attr_set_ftp (&(_sess->attr_gass), &(_sess->operation_attr_ftp));
        gfal_globus_check_result("GridFTPFactory::globus_gass_copy_handleattr_set_ftp_attr", res);

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
		if(_sess != NULL)
			factory->gfal_globus_ftp_release_handle_internal(this);
	}
	
	virtual globus_ftp_client_handle_t* get_ftp_handle(){
		globus_result_t res = globus_gass_copy_get_ftp_handle(&(_sess->gass_handle), &(_sess->handle_ftp));
		gfal_globus_check_result("GridFTPFactory::GridFTP_session_implem", res);			
		return &(_sess->handle_ftp);
	}

	virtual globus_gass_copy_handle_t* get_gass_handle(){
		return &(_sess->gass_handle);
	}	
		
	virtual globus_gass_copy_attr_t* get_gass_attr(){
			return &(_sess->attr_gass);
	}
	
	virtual globus_ftp_client_operationattr_t* get_op_attr_ftp(){
		return &(_sess->operation_attr_ftp);
	}	
	
	virtual globus_gass_copy_handleattr_t* get_gass_handle_attr(){
		return &(_sess->gass_handle_attr);
	}
	
	virtual void purge(){
		globus_gass_copy_handle_destroy(&(_sess->gass_handle));
		globus_ftp_client_handleattr_destroy(&(_sess->attr_handle));
		globus_ftp_client_operationattr_destroy (&(_sess->operation_attr_ftp));
		globus_gass_copy_handleattr_destroy(&(_sess->gass_handle_attr));	
		globus_ftp_client_operationattr_destroy(&(_sess->operation_attr_ftp));
		delete _sess;
		_sess = NULL;		
	}	
	
	// handle ftp
	
	struct Session_handler{
		globus_ftp_client_handle_t handle_ftp;
		globus_ftp_client_handleattr_t attr_handle;
		globus_ftp_client_operationattr_t operation_attr_ftp;     
		globus_gass_copy_attr_t attr_gass;
		globus_gass_copy_handle_t gass_handle;
		globus_gass_copy_handleattr_t gass_handle_attr;
	};
	
	Session_handler* _sess;
	// internal fields	
	std::string hostname;		
	GridFTPFactory* factory;
    globus_ftp_control_dcau_t dcau;
};


GridFTPFactory::GridFTPFactory(gfal_handle handle) : _handle(handle)
{
    GError * tmp_err=NULL;
    gridftp_v2 = gfal2_get_opt_boolean(_handle, GRIDFTP_CONFIG_GROUP, gridftp_version_config, &tmp_err);
    if(tmp_err)
        throw Glib::Error(tmp_err);

    session_reuse = gfal2_get_opt_boolean(_handle, GRIDFTP_CONFIG_GROUP, gridftp_session_reuse_config, &tmp_err);
    if(tmp_err)
        throw Glib::Error(tmp_err);

    dcau_param.mode = (gfal2_get_opt_boolean(_handle, GRIDFTP_CONFIG_GROUP, gridftp_dcau_config, &tmp_err))?GLOBUS_FTP_CONTROL_DCAU_DEFAULT:GLOBUS_FTP_CONTROL_DCAU_NONE;
    if(tmp_err)
        throw Glib::Error(tmp_err);
	size_cache = 400;
}


GridFTP_Request_state::GridFTP_Request_state(GridFTP_session * s, bool own_session) : sess(s){
	req_status=GRIDFTP_REQUEST_NOT_LAUNCHED;
	this-> own_session = own_session;
}

GridFTP_Request_state::~GridFTP_Request_state()
{
	if(!own_session)
		sess.release(); // cancel the automatic memory management
	
	if(req_status == GRIDFTP_REQUEST_RUNNING){
		gfal_log(GFAL_VERBOSE_TRACE,"cancel current running gridftp request... ");
		globus_ftp_client_abort(sess->get_ftp_handle());
		gridftp_wait_for_callback(scope_request, this);
	}
		
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
	

GridFTP_session* GridFTPFactory::get_recycled_handle(const std::string & hostname){
	Glib::Mutex::Lock l(mux_cache);	
	GridFTP_session* res= NULL;
	std::multimap<std::string, GridFTP_session*>::iterator it=sess_cache.find(hostname);
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
 *  In the current state, globus provides no way to convert gridftp error code to errno properly :'(
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
		if(glob_str) // security
			g_strlcpy( errbuff, glob_str, GFAL_URL_MAX_LEN);
		globus_object_free(error);
		throw Gfal::CoreException(scope, errbuff, globus_errno );		
	}
}


GridFTP_session* GridFTPFactory::get_new_handle(const std::string & hostname){
	std::auto_ptr<GridFTP_session_implem> sess(new GridFTP_session_implem(this, hostname));		
	sess->set_gridftpv2(gridftp_v2);
    sess->set_dcau(dcau_param);
	return sess.release();
}

// store the related globus error to the current handle
void gfal_globus_store_error(GridFTP_Request_state * state, globus_object_t *error){
		char * glob_str=NULL;	
		state->errcode = gfal_globus_error_convert(error, &glob_str);
		if(glob_str){
			state->error = std::string(glob_str);
			g_free(glob_str);
		}else{
			state->error = "Uknow Globus Error, bad error report";
			state->errcode = EFAULT;
		}
}

GridFTP_session* GridFTPFactory::gfal_globus_ftp_take_handle(const std::string & hostname){
	GridFTP_session * res = NULL;
	if( (res = get_recycled_handle(hostname)) == NULL)
		res = get_new_handle(hostname);
	return res;
}

void GridFTPFactory::gfal_globus_ftp_release_handle_internal(GridFTP_session* sess){
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


void globus_basic_client_callback (void * user_arg, 
				globus_ftp_client_handle_t *		handle,
				globus_object_t *				error){
	GridFTP_Request_state* state = (GridFTP_Request_state*) user_arg;	
	
	if(error != GLOBUS_SUCCESS){	
		gfal_globus_store_error(state, error);
	}else{
		state->errcode = 0;	
	}
	state->req_status = GRIDFTP_REQUEST_FINISHED;	
}



void gridftp_poll_callback(const Glib::Quark & scope, GridFTP_Request_state* state){
	gfal_log(GFAL_VERBOSE_TRACE," -> go polling for request ");
	while(state->req_status != GRIDFTP_REQUEST_FINISHED )
			usleep(10);	
	gfal_log(GFAL_VERBOSE_TRACE," <- out of polling for request ");			
}

void gridftp_poll_callback_stream(const Glib::Quark & scope, GridFTP_stream_state* state){
    gfal_log(GFAL_VERBOSE_TRACE," -> go polling for request ");
    while(state->stream_status != GRIDFTP_REQUEST_FINISHED )
            usleep(10);
    gfal_log(GFAL_VERBOSE_TRACE," <- out of polling for request ");
}

void gridftp_callback_err_report(const Glib::Quark & scope, GridFTP_Request_state* state){
	if(state->errcode != 0)	
		throw Gfal::CoreException(scope,  state->error, state->errcode);	
}

void gridftp_wait_for_callback(const Glib::Quark & scope, GridFTP_Request_state* state){
	gridftp_poll_callback(scope, state);
	gridftp_callback_err_report(scope, state);
}

void gridftp_wait_for_callback_stream(const Glib::Quark & scope, GridFTP_stream_state* state){
    gridftp_poll_callback_stream(scope, state);
    gridftp_callback_err_report(scope, state);
}

void gridftp_wait_for_read(const Glib::Quark & scope, GridFTP_stream_state* state, off_t end_read){
    gridftp_wait_for_callback_stream(scope, state);
}

void gridftp_wait_for_write(const Glib::Quark & scope, GridFTP_stream_state* state, off_t end_write){
    gridftp_wait_for_callback_stream(scope, state);
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
		if(state->offset != offset){
			state->error = " Invalid read callback call from globus, out of order";
			state->errcode =EIO;			
		}else{
			state->offset += (off_t) length;
			state->eof = (bool) eof;
			state->errcode =0;
		}		
	}
    state->stream_status = GRIDFTP_REQUEST_FINISHED;
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
		if(state->offset != offset){
			state->error = " Invalid write callback call from globus, out of order";
			state->errcode =EIO;			
		}else{
			state->offset += (off_t) length;
			state->eof = (bool) eof;
			state->errcode =0;
		}		
	}
    state->stream_status = GRIDFTP_REQUEST_FINISHED;
}

ssize_t gridftp_read_stream(const Glib::Quark & scope,
							GridFTP_stream_state* stream,
							void* buffer, size_t s_read
							){
	gfal_log(GFAL_VERBOSE_TRACE,"  -> [gridftp_read_stream]");	
	off_t initial_offset = stream->offset;
	
    if(stream->eof)
        return 0;
	globus_result_t res = globus_ftp_client_register_read( stream->sess->get_ftp_handle(),
		(globus_byte_t*) buffer,
		s_read,
		gfal_griftp_stream_read_callback,
		stream 
	); 		
	gfal_globus_check_result(scope, res);	
	gridftp_wait_for_read(scope, stream, initial_offset + s_read);	
    stream->stream_status =GRIDFTP_REQUEST_NOT_LAUNCHED;
	return stream->offset - initial_offset;							
}


ssize_t gridftp_write_stream(const Glib::Quark & scope,
							GridFTP_stream_state* stream,
							const void* buffer, size_t s_write,
							bool eof
							){
	gfal_log(GFAL_VERBOSE_TRACE,"  -> [gridftp_write_stream]");	
	off_t initial_offset = stream->offset;
	
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
    stream->stream_status =GRIDFTP_REQUEST_NOT_LAUNCHED;
	return stream->offset - initial_offset;							
}


std::string gridftp_hostname_from_url(const char * url){
	GError * tmp_err=NULL;
	char buffer[GFAL_URL_MAX_LEN];
	const int res = gfal_hostname_from_uri(url, buffer, GFAL_URL_MAX_LEN, &tmp_err);
	if(res < 0)
		throw Glib::Error(tmp_err);
	return std::string(buffer);
}





