#pragma once
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


#ifndef PARAMS_INTERFACE_HPP
#define PARAMS_INTERFACE_HPP

namespace Gfal{
	
namespace Transfer{

class Params_interface
{
	public:
		Params_interface(){};
		virtual ~Params_interface(){};
		
		virtual void lock()=0;
		virtual void set_timeout(long timeout)=0;
		virtual void replace_existing(bool replace)=0;
		virtual void set_offset(off_t offset)=0;
		virtual void set_nbstream(unsigned long nbstream)=0;
		virtual void set_user_data(void* user_data)=0;
		virtual void set_monitor(gfalt_monitor_tfr callback)=0;
			
};

};

};

#endif /* PARAMS_INTERFACE_H */ 
