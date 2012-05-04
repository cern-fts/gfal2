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
	
	GridFTPFactory* factory;
};


GridFTPFactory::GridFTPFactory(gfal_handle handle) : _handle(handle)
{
	
}




GridFTPFactory::~GridFTPFactory()
{
	
}

gfal_handle GridFTPFactory::get_handle(){
	return _handle;
}

gfal_globus_copy_handle_t GridFTPFactory::take_globus_gass_handle(){
	gfal_globus_copy_handle_t globus_handle;
	globus_result_t res = globus_gass_copy_handle_init 	(&globus_handle, GLOBUS_NULL);
	gfal_globus_check_result("GridFTPFactory::release_globus_handle", res);	
	return globus_handle;
		
}

void GridFTPFactory::release_globus_gass_handle(gfal_globus_copy_handle_t *globus_handle){
	globus_result_t res = globus_gass_copy_handle_destroy(globus_handle);
	gfal_globus_check_result("GridFTPFactory::release_globus_handle", res);	
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


gfal_globus_copy_attr_t* GridFTPFactory::take_globus_gass_attr(){
	gfal_globus_copy_attr_t* attr = (gfal_globus_copy_attr_t*) malloc(sizeof(gfal_globus_copy_attr_t));
	globus_result_t res = globus_gass_copy_attr_init(attr);
	gfal_globus_check_result("GridFTPFactory::take_globus_attr", res);	
	return attr;
}



GridFTP_session* GridFTPFactory::gfal_globus_ftp_take_handle(){
	std::auto_ptr<GridFTP_session> sess(new GridFTP_session_implem(this));
	globus_result_t res;
	res = globus_ftp_client_handleattr_init(&(sess->attr_handle));
	gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle_attr", res);	
	globus_ftp_client_handleattr_set_cache_all(&(sess->attr_handle), GLOBUS_TRUE);
	
	res = globus_ftp_client_handle_init(&(sess->handle), &(sess->attr_handle));
	gfal_globus_check_result("GridFTPFactory::gfal_globus_ftp_take_handle", res);	
	return sess.release();
}

void GridFTPFactory::gfal_globus_ftp_release_handle_internal(GridFTP_session* sess){
	globus_ftp_client_handle_destroy(&(sess->handle));
	globus_ftp_client_handleattr_destroy(&(sess->attr_handle));
}


void GridFTPFactory::gfal_globus_ftp_release_handle(GridFTP_session* h){
	delete h;
}


void GridFTPFactory::release_globus_gass_attr(gfal_globus_copy_attr_t* h){
	free(h);
}

void globus_basic_client_callback (void * user_arg, 
				globus_ftp_client_handle_t *		handle,
				globus_object_t *				error){
	GridFTP_Request_state* state = (GridFTP_Request_state*) user_arg;	
	
	state->status = 0;
	if(error != GLOBUS_SUCCESS){	
		char * glob_str;		
		
		if((glob_str = globus_error_print_friendly(error)) != NULL)// convert err string
			state->error = glob_str;
		state->errcode = scan_errstring(glob_str); // try to get errno
		g_free(glob_str);
	}else{
		state->errcode = 0;	
	}
		
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


static void gfal_griftp_stream_read_callback(void *user_arg, globus_ftp_client_handle_t *handle, globus_object_t *error, globus_byte_t *buffer,
				globus_size_t length, globus_off_t offset, globus_bool_t eof){

	GridFTP_stream_state* state = static_cast<GridFTP_stream_state*>(user_arg);
	
	
	if(error != GLOBUS_SUCCESS){	// check error status
		char * glob_str;		
		if((glob_str = globus_error_print_friendly(error)) != NULL) // convert err string
			state->error = glob_str;				
		state->errcode = scan_errstring(glob_str); // try to get errno
		g_free(glob_str);
	   // gfal_print_verbose(GFAL_VERBOSE_TRACE," read error %s , code %d", state->error, state->errcode);					
	}else{
		// verify read 
		//gfal_print_verbose(GFAL_VERBOSE_TRACE," read %d bytes , eof %d %d,%d", length, eof, state->offset, offset);			
		if(state->offset != offset){
			state->error = " Invalid read callback call from globus, out of order";
			state->errcode =EIO;			
		}else{
			state->offset += length;
			state->eof = eof;
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
	globus_result_t res = globus_ftp_client_register_read( &(stream->sess->handle),
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





