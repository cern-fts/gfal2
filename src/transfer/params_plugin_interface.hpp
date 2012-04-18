#pragma once
#ifndef PARAMS_PLUGIN_INTERFACE_HPP
#define PARAMS_PLUGIN_INTERFACE_HPP

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



namespace Gfal{
	
namespace Transfer{

class Params_plugin_interface
{
	public:
		Params_plugin_interface(){};
		virtual ~Params_plugin_interface(){};
		
		virtual long get_timeout()=0;
		virtual unsigned long get_nbstream()=0;
			
};

};

};

#endif /* PARAMS_INTERFACE_H */ 
