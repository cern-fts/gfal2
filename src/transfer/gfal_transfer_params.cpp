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

#include <transfer/gfal_transfer_types_internal.h>
#include <transfer/gfal_cpp_wrapper.h>
#include <cerrno>

const Glib::Quark scope_transfer_param("Gfal::Transfer::Params");

Gfal::Transfer::Params::Params(){
	_callback = NULL;
	_lock = false;
	_nbstream = GFALT_DEFAULT_NB_STREAM;
	_timeout = GFALT_DEFAULT_TRANSFERT_TIMEOUT;
	_start_offset = 0;
	uuid_clear(_uuid);
}

Gfal::Transfer::Params::~Params(){
	this->_lock = false;
}


void Gfal::Transfer::Params::_check_params(){
	if(this->_lock)
		throw Gfal::CoreException(scope_transfer_param,"Copy already started, the parameter modifications are locked", EBUSY);
}


void Gfal::Transfer::Params::set_timeout(long timeout){
	_check_params();
	this->_timeout = timeout;
}


void Gfal::Transfer::Params::set_monitor(gfalt_monitor_tfr callback){
	_check_params();
	this->_callback = callback;
}

void Gfal::Transfer::Params::set_nbstream(unsigned long nbstream){
	_check_params();
	this->_nbstream = nbstream;
}

void Gfal::Transfer::Params::set_offset(off_t offset){
	_check_params();
	this->_start_offset = offset;
}

long Gfal::Transfer::Params::get_timeout(){
	return this->_timeout;
}

unsigned long Gfal::Transfer::Params::get_nbstream(){
	return this->_nbstream;
}

void* Gfal::Transfer::Params::get_user_data(){
	return this->_user_data;
}

void Gfal::Transfer::Params::lock(){
	_check_params();
	if(uuid_is_null(_uuid)){ // generate a uuid for the transfer if available
		uuid_generate(_uuid);
	}
	
	_lock = true;	
	
	
}

void Gfal::Transfer::Params::unlock(){
	uuid_clear(_uuid);
	_lock = false;	
}

void Gfal::Transfer::Params::set_user_data(void* user_data){
	this->_user_data = user_data;
}

void Gfal::Transfer::Params::replace_existing(bool replace_existing){
	this->_replace_existing = replace_existing;
}






// external C bindings
extern "C" {
	

gfalt_params_t gfalt_params_handle_new(GError ** err){
	return reinterpret_cast<gfalt_params_t>(new Gfal::Transfer::Params());
}


void gfalt_params_handle_delete(gfalt_params_t params, GError ** err){
	if(params != NULL){
		delete reinterpret_cast<Gfal::Transfer::Params*>(params);
	}
}


int gfalt_set_timeout(gfalt_params_t handle, unsigned long timeout, GError** err){
	using namespace Gfal::Transfer;
	g_return_val_err_if_fail(handle, -1, err, "invalid parameter handle value");
	GError * tmp_err= NULL;
	int ret = -1;

	CPP_GERROR_TRY
		reinterpret_cast<Params*>(handle)->set_timeout(timeout);
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	G_RETURN_ERR(ret, tmp_err, err);
}


long gfalt_get_timeout(gfalt_params_t params, GError** err){
	g_return_val_err_if_fail(params != NULL, -1, err, "invalid params handle");		
	GError * tmp_err= NULL;
	long ret = -1;
		
	CPP_GERROR_TRY		
			ret= (reinterpret_cast<Gfal::Transfer::Params*>(params))->get_timeout();	
	CPP_GERROR_CATCH(&tmp_err);		
	G_RETURN_ERR(ret, tmp_err, err);	
}	


int gfalt_set_nbstreams(gfalt_params_t params, unsigned long nbstreams, GError** err){
	g_return_val_err_if_fail(params != NULL, -1, err, "invalid params handle");		
	GError * tmp_err= NULL;
	int ret= -1;
	
	CPP_GERROR_TRY		
			(reinterpret_cast<Gfal::Transfer::Params*>(params))->set_nbstream(nbstreams);	
			ret =0;
	CPP_GERROR_CATCH(&tmp_err);		
	G_RETURN_ERR(ret, tmp_err, err);	
}

int gfalt_set_replace_existing_file(gfalt_params_t params, gboolean replace, GError** err){
	g_return_val_err_if_fail(params != NULL, -1, err, "invalid params handle");		
	GError * tmp_err= NULL;
	int ret= -1;	
	
	CPP_GERROR_TRY		
			(reinterpret_cast<Gfal::Transfer::Params*>(params))->replace_existing(replace);	
			ret =0;			
	CPP_GERROR_CATCH(&tmp_err);	
	G_RETURN_ERR(ret, tmp_err, err);
}

int gfalt_set_offset_from_source(gfalt_params_t params, off_t offset, GError** err){
	g_return_val_err_if_fail(params != NULL, -1, err, "invalid params handle");	
	GError * tmp_err= NULL;
	int ret= -1;		
	
	CPP_GERROR_TRY		
			(reinterpret_cast<Gfal::Transfer::Params*>(params))->set_offset(offset);
			ret =0;		
	CPP_GERROR_CATCH(&tmp_err);				
	G_RETURN_ERR(ret, tmp_err, err);	
}

int gfalt_set_user_data(gfalt_params_t params, gpointer user_data, GError** err){
	g_return_val_err_if_fail(params != NULL, -1, err, "invalid params handle");
	GError * tmp_err= NULL;
	int ret= -1;		
	
	CPP_GERROR_TRY	
		(reinterpret_cast<Gfal::Transfer::Params*>(params))->set_user_data(user_data);
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);	
	G_RETURN_ERR(ret, tmp_err, err);		
}

long gfalt_get_nbstreams(gfalt_params_t params, GError** err){
	g_return_val_err_if_fail(params != NULL, -1, err, "invalid params handle");
	long ret = -1;
	GError * tmp_err=NULL;
	CPP_GERROR_TRY
		ret= (reinterpret_cast<Gfal::Transfer::Params*>(params))->get_nbstream();
	CPP_GERROR_CATCH(&tmp_err);
	G_RETURN_ERR(ret, tmp_err, err);
}



int gfalt_set_uuid(gfalt_params_t, uuid_t uuid, GError** err);

int gfalt_set_callback_mperiod(gfalt_params_t, unsigned int mtime, GError** err); // time in ms between two callback calls.


void gfalt_set_monitor_tfr(gfalt_params_t params, gfalt_monitor_tfr callback);

}
