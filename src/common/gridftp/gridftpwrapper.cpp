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
#include "gridftpwrapper.h"

const Glib::Quark scope_readder("gfal_griftp_stream_read");
const Glib::Quark scope_request("GridftpModule::request");

struct RwStatus{
	off_t init;
	off_t finish;
	bool ops_state;
};

struct GridFTP_session_implem : public GridFTP_session{
	GridFTP_session_implem(GridFTPFactory* f){
		factory = f;
	}
	
	virtual ~GridFTP_session_implem(){
		factory->gfal_globus_ftp_release_handle_internal(this);
	}
	
	virtual globus_ftp_client_handle_t* get_ftp_handle(){
		globus_result_t res = globus_gass_copy_get_ftp_handle(&gass_handle, &handle_ftp);
		gfal_globus_check_result("GridFTPFactory::GridFTP_session_implem", res);			
		return &handle_ftp;
	}

	virtual globus_gass_copy_handle_t* get_gass_handle(){
		return &gass_handle;
	}	
		
	virtual globus_gass_copy_attr_t* get_gass_attr(){
			return &attr_gass;
	}
	
	virtual globus_gass_copy_handleattr_t* get_gass_handle_attr(){
			return &gass_handle_attr;
	}	
			
	GridFTPFactory* factory;
};


GridFTPFactory::GridFTPFactory(gfal_handle handle) : _handle(handle)
{
	gridftp_v2 = true;
}

void GridFTPFactory::configure_gridftp_handle_attr(globus_ftp_client_handleattr_t * attrs){
	globus_ftp_client_handleattr_set_gridftp2(attrs, (gridftp_v2)?GLOBUS_TRUE:GLOBUS_FALSE); // define gridftp 2
	globus_ftp_client_handleattr_set_cache_all(attrs, GLOBUS_TRUE);	// enable session re-use
}

GridFTP_Request_state::~GridFTP_Request_state()
{
	if(!done){
		gfal_print_verbose(GFAL_VERBOSE_TRACE,"cancel current running gridftp request... ");
		globus_ftp_client_abort(sess->get_ftp_handle());
		gridftp_wait_for_callback(scope_request, this);
	}
		
}


GridFTPFactory::~GridFTPFactory()
{
	
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

    if (strstr(p, "o such file"))
        ret = ENOENT;
    else if (strstr(p, "ermission denied") || strstr(p, "credential"))
        ret = EACCES;
    else if (strstr(p, "exists"))
        ret = EEXIST;
    else if (strstr(p, "ot a direct"))
		ret = ENOTDIR;
    return ret;
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
		char errbuff[GFAL_URL_MAX_LEN];
		char * glob_str;		
		*errbuff='\0';
		
		if((glob_str = globus_error_print_friendly(error)) != NULL) // convert err string
			g_strlcpy( errbuff, glob_str, GFAL_URL_MAX_LEN);
		const int globus_errno = scan_errstring(errbuff); // try to get errno
		globus_object_free(error);
		throw Gfal::CoreException(scope, errbuff, globus_errno );		
	}
}





GridFTP_session* GridFTPFactory::gfal_globus_ftp_take_handle(){
	std::auto_ptr<GridFTP_session> sess(new GridFTP_session_implem(this));
	globus_result_t res;
	// initialize gass copy attr
	res = globus_gass_copy_attr_init(&(sess->attr_gass));
	gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);		
	
	// initialize ftp attributes
	res = globus_ftp_client_handleattr_init(&(sess->attr_handle));
	gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle_attr", res);	
	configure_gridftp_handle_attr(&(sess->attr_handle));
	
	// create gass handle attribute
	res =	globus_gass_copy_handleattr_init(&(sess->gass_handle_attr));
	gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);		
		
	// associate ftp attributes to gass attributes
	res = globus_gass_copy_handleattr_set_ftp_attr (&(sess->gass_handle_attr), &(sess->attr_handle));
	gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);			
	
	// initialize gass handle 
	res = globus_gass_copy_handle_init (&(sess->gass_handle), &(sess->gass_handle_attr));
	gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);	
		
	return sess.release();
}

void GridFTPFactory::gfal_globus_ftp_release_handle_internal(GridFTP_session* sess){
	globus_gass_copy_handle_destroy(&(sess->gass_handle));
	globus_ftp_client_handleattr_destroy(&(sess->attr_handle));
	globus_gass_copy_handleattr_destroy(&(sess->gass_handle_attr));
}


void GridFTPFactory::gfal_globus_ftp_release_handle(GridFTP_session* h){
	delete h;
}


void globus_basic_client_callback (void * user_arg, 
				globus_ftp_client_handle_t *		handle,
				globus_object_t *				error){
	GridFTP_Request_state* state = (GridFTP_Request_state*) user_arg;	
	state->done = true;	
	
	if(error != GLOBUS_SUCCESS){	
		char * glob_str;		
		if((glob_str = globus_error_print_friendly(error)) != NULL){// convert err string
			state->error = std::string(glob_str);
			state->errcode = scan_errstring(glob_str); // try to get errno
			g_free(glob_str);
		}else{
			state->error = "Uknow Globus Error, bad error report";
			state->errcode = EFAULT;
		}
	}else{
		state->errcode = 0;	
	}
	state->status = 0;	
}

void gridftp_poll_callback(const Glib::Quark & scope, GridFTP_Request_state* state){
	gfal_print_verbose(GFAL_VERBOSE_TRACE," -> go polling for request ");
	while(state->status !=0 )
			usleep(10);	
	gfal_print_verbose(GFAL_VERBOSE_TRACE," <- out of polling for request ");			
}

void gridftp_callback_err_report(const Glib::Quark & scope, GridFTP_Request_state* state){
	if(state->errcode != 0)	
		throw Gfal::CoreException(scope,  state->error, state->errcode);	
}

void gridftp_wait_for_callback(const Glib::Quark & scope, GridFTP_Request_state* state){
	gridftp_poll_callback(scope, state);
	gridftp_callback_err_report(scope, state);
}

void gridftp_wait_for_read(const Glib::Quark & scope, GridFTP_stream_state* state, off_t end_read){
	gfal_print_verbose(GFAL_VERBOSE_TRACE," -> go polling for read ");

	while(state->status !=0)
			usleep(10);			
	if(state->errcode != 0)	
		throw Gfal::CoreException(scope,  state->error, state->errcode);
	gfal_print_verbose(GFAL_VERBOSE_TRACE," <- out of polling for read ");			
}

void gridftp_wait_for_write(const Glib::Quark & scope, GridFTP_stream_state* state, off_t end_write){
	gfal_print_verbose(GFAL_VERBOSE_TRACE," -> go polling for write ");

	while(state->status !=0)
			usleep(10);			
	if(state->errcode != 0)	
		throw Gfal::CoreException(scope,  state->error, state->errcode);
	gfal_print_verbose(GFAL_VERBOSE_TRACE," <- out of polling for write ");			
}



static void gfal_griftp_stream_read_callback(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t *error, globus_byte_t *buffer,
				globus_size_t length, globus_off_t offset, globus_bool_t eof){

	GridFTP_stream_state* state = static_cast<GridFTP_stream_state*>(user_arg);
	
	
	if(error != GLOBUS_SUCCESS){	// check error status
		char * glob_str;		
		if((glob_str = globus_error_print_friendly(error)) != NULL){// convert err string
			state->error = std::string(glob_str);
			state->errcode = scan_errstring(glob_str); // try to get errno
			g_free(glob_str);
		}else{
			state->error = "Uknow Globus Error, bad error report";
			state->errcode = EFAULT;
		}
	   // gfal_print_verbose(GFAL_VERBOSE_TRACE," read error %s , code %d", state->error, state->errcode);					
	}else{
		// verify read 
		//gfal_print_verbose(GFAL_VERBOSE_TRACE," read %d bytes , eof %d %d,%d", length, eof, state->offset, offset);			
		if(state->offset != offset){
			state->error = " Invalid read callback call from globus, out of order";
			state->errcode =EIO;			
		}else{
			state->offset += (off_t) length;
			state->eof = (bool) eof;
			state->errcode =0;
		}		
	}
	state->status = 0;		
}


static void gfal_griftp_stream_write_callback(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t *error, globus_byte_t *buffer,
				globus_size_t length, globus_off_t offset, globus_bool_t eof){

	GridFTP_stream_state* state = static_cast<GridFTP_stream_state*>(user_arg);
	
	
	if(error != GLOBUS_SUCCESS){	// check error status
		char * glob_str;		
		if((glob_str = globus_error_print_friendly(error)) != NULL){// convert err string
			state->error = std::string(glob_str);
			state->errcode = scan_errstring(glob_str); // try to get errno
			g_free(glob_str);
		}else{
			state->error = "Uknow Globus Error, bad error report";
			state->errcode = EFAULT;
		}
	   // gfal_print_verbose(GFAL_VERBOSE_TRACE," read error %s , code %d", state->error, state->errcode);					
	}else{
		// verify write 
		//gfal_print_verbose(GFAL_VERBOSE_TRACE," read %d bytes , eof %d %d,%d", length, eof, state->offset, offset);			
		if(state->offset != offset){
			state->error = " Invalid write callback call from globus, out of order";
			state->errcode =EIO;			
		}else{
			state->offset += (off_t) length;
			state->eof = (bool) eof;
			state->errcode =0;
		}		
	}
	state->status = 0;		
}

ssize_t gridftp_read_stream(const Glib::Quark & scope,
							GridFTP_stream_state* stream,
							void* buffer, size_t s_read
							){
	gfal_print_verbose(GFAL_VERBOSE_TRACE,"  -> [gridftp_read_stream]");	
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
	stream->status =1;
	return stream->offset - initial_offset;							
}


ssize_t gridftp_write_stream(const Glib::Quark & scope,
							GridFTP_stream_state* stream,
							const void* buffer, size_t s_write,
							bool eof
							){
	gfal_print_verbose(GFAL_VERBOSE_TRACE,"  -> [gridftp_write_stream]");	
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
	stream->status =1;
	return stream->offset - initial_offset;							
}






