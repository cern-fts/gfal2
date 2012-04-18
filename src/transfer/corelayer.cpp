/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
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



#include <common/gfal_types.h>
#include <common/gfal_common_internal.h>
#include <exceptions/gfalcoreexception.hpp>

#include "corelayer.h"

#include <dlfcn.h>


using namespace Utilpp;
 

void* Gfal::PluginItem::get_sym(const std::string & str) const{
	return dlsym(_dl, str.c_str());
}

void* Gfal::PluginItem::plugin_data(){
	return _data;
}


Gfal::CoreLayer::CoreLayer()
	: PluginFactory()
{
	_internal_handle= true;
	GError * tmp_err=NULL;
	gfal_handle mhandle = gfal_initG(&tmp_err);
	if(mhandle == NULL)
		throw Gfal::CoreException("CoreLayer::CoreLayer()", tmp_err->message, tmp_err->code);	
	_init(mhandle);
}


void Gfal::CoreLayer::_init(gfal_handle handle)
{
	GError * tmp_err=NULL;
	_handle = handle;
	plugin_pointer_handle pdls, dls;
	dls = pdls = gfal_plugins_list_handler(_handle, &tmp_err);
	gerror_to_cpp(&tmp_err);
	
	while(pdls->dlhandle != NULL){
		gfal_print_verbose(GFAL_VERBOSE_TRACE, " get plugin links for %s plugin ", pdls->plugin_name);
		_plugins.push_back(new PluginItem(pdls->dlhandle, pdls->plugin_data));		
		++pdls;
	}
	g_free(dls);
}


Gfal::CoreLayer::CoreLayer(gfal_handle handle)
	: PluginFactory()
{
	_internal_handle= false;
	_init(handle);
}


Gfal::CoreLayer::~CoreLayer()
{
	for(unsigned int i=0; i < _plugins.size(); ++i){
		delete _plugins[i];
	}
	
	if(_internal_handle)
		gfal_handle_freeG(_handle);
}


const Gfal::VectorPlugin & Gfal::CoreLayer::get_plugin_links(){
	return _plugins;
}


