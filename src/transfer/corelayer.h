#pragma once
#ifndef CORELAYER_H
#define CORELAYER_H
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

#include <transfer/gfal_transfer_internal.h> 
#include <transfer/plugininterface.h>


namespace Gfal{

/**
 *  Main Implementation of pluginLink
 *  represent the main layer for the direct interaction with a plugin
 */
class PluginItem : public PluginLink{
	
	public:
		PluginItem(void* dlhandle, void*  data) : _dl(dlhandle), _data(data) {};
		virtual ~PluginItem(){};
		
		virtual void* get_sym(const std::string & str) const;	
		
		virtual void* plugin_data();
	private:
		void* _dl;
		void* _data;
};

class CoreLayer: public PluginFactory
{
	public:
		CoreLayer(gfal_handle handle);
		CoreLayer();
		virtual ~CoreLayer();

		virtual const VectorPlugin & get_plugin_links();
	private:
		void _init(gfal_handle handle);
	
		bool _internal_handle;
		gfal_handle _handle;
		VectorPlugin _plugins;
		/* add your private declarations */
};


}

#endif /* CORELAYER_H */ 
