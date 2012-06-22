#pragma once
#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H
/* 
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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

#include <string>
#include <vector>
#include <iostream>
#include <common/gfal_prototypes.h>


namespace Gfal{


/**
 * Interface for direct interaction with a plugin
 * */
class PluginLink {
	public:
		PluginLink(){};
		virtual ~PluginLink(){};
		
		/**
		 * Resolve a function name in the plugin converted to the T function pointer
		 * return NULL in case of no function regirested with this name
		 **/
		template<typename T> 
		T get_sym_s(const std::string & str) const;
		/**
		 * Resolve a function name in the plugin to a generic function pointer
		 * return NULL in case of no function regirested with this name
		 **/
		virtual void* get_sym(const std::string & str) const =0;
		
		/**
		 * Get the plugin data handler
		 **/
		virtual void* plugin_data()= 0;
	
};

// overloded member of get_sym, allow type safety
template<typename T> 
T PluginLink::get_sym_s(const std::string & str) const {
	return reinterpret_cast<T>(this->get_sym(str));
}

// 
typedef std::vector<PluginLink* > VectorPlugin;

class PluginFactory{
	public:
		PluginFactory(){};
		virtual ~PluginFactory(){};	
		
		/**
		 * provide virtual interface to the set of plugins and their symbol resolution
		 */
		virtual const VectorPlugin& get_plugin_links() =0;

        virtual gfal_context_t get_context()=0;
};

}
#endif /* PluginFactory_H */ 
