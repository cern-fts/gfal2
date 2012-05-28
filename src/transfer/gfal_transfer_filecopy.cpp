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

const Glib::Quark scope_copy("FileCopy::start_copy");

Gfal::Transfer::FileCopy::FileCopy(PluginFactory* wrap) :  MainDecorator(wrap){

}

Gfal::Transfer::FileCopy::~FileCopy() {

}

void Gfal::Transfer::FileCopy::start_copy(gfalt_params_t p, const std::string & src, const std::string & dst){
	gfal_log(GFAL_VERBOSE_TRACE, " -> Gfal::Transfer::FileCopy ");
	//p->lock(); suppress locker, need read/write lock system	
	void * plug_data;
	GError * tmp_err=NULL;
	plugin_filecopy_call p_copy = find_copy_plugin(src, dst, &plug_data); // get the filecopy call of the plugin
	if(p_copy == NULL)
		throw Gfal::CoreException(scope_copy, "bug detected : \
				no correct filecopy function in a plugin signaled like compatible", ENOSYS);
	const int res = p_copy(plug_data, static_cast<gfal_context_t>(this), p, src.c_str(), dst.c_str(), &tmp_err);
	//p->unlock();
	if(res <0 )
		throw Gfal::CoreException(scope_copy, std::string(tmp_err->message), tmp_err->code);
	gfal_log(GFAL_VERBOSE_TRACE, " <- Gfal::Transfer::FileCopy ");
}

const plugin_filecopy_call Gfal::Transfer::FileCopy::find_copy_plugin(const std::string & src, const std::string & dst, void** plugin_data){
	const VectorPlugin & p = this->get_plugin_links();

	
	VectorPlugin::const_iterator it;
	
	for(it = p.begin(); it != p.end(); it++){
			plugin_url_check2_call check_call = (*it)->get_sym_s<plugin_url_check2_call>("plugin_url_check2");
			if(check_call != NULL){
				gboolean compatible;
				*plugin_data =(*it)->plugin_data(); 				
				if( (compatible = check_call(*plugin_data, src.c_str(), dst.c_str(), GFAL_FILE_COPY) ) == TRUE){
					return (*it)->get_sym_s<plugin_filecopy_call>("plugin_filecopy");					
				}
				
			}
	}
	throw Gfal::CoreException(scope_copy, "no plugin is able to support this transfer", EPROTONOSUPPORT);
}


extern "C" {
	
int gfalt_copy_file(gfal_context_t handle, gfalt_params_t params, 
			const char* src, const char* dst,  GError** err){
	using namespace Gfal::Transfer;
	
	g_return_val_err_if_fail( handle && src && dst, -1, err, "invalid source or/and destination values");
	GError * tmp_err=NULL;
	int ret = -1;
	gfalt_params_t p = NULL;
	
	CPP_GERROR_TRY
	
		if(params == NULL){
			p = gfalt_params_handle_new(NULL);
			reinterpret_cast<FileCopy*>(handle)->start_copy(p, src, dst);
		}
		else{
			reinterpret_cast<FileCopy*>(handle)->start_copy(params, src, dst);
		}
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfalt_params_handle_delete(p, NULL);
	G_RETURN_ERR(ret, tmp_err, err);
}
	
}	



