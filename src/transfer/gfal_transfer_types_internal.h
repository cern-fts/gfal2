#pragma once
#ifndef _GFAL2_TRANSFER_TYPES_INTERNAL_
#define _GFAL2_TRANSFER_TYPES_INTERNAL_

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

#include <iostream>
#include <string>


#include <exceptions/gfalcoreexception.hpp>

#include <common/gfal_constants.h>
#include <common/gfal_types.h>
#include <common/gfal_common_errverbose.h>

#include <global/gfal_global.h>

#include <transfer/gfal_transfer.h>
#include <transfer/gfal_transfer_plugins.h>
#include <transfer/gfalmaindecorator.h>
#include <transfer/params_interface.hpp>
#include <transfer/params_plugin_interface.hpp>


namespace Gfal{
	
namespace Transfer{

class Params : public Params_interface, public Params_plugin_interface{
public:
		Params();
		virtual ~Params();
		
		// setters
		virtual void lock();
		virtual void unlock();
		virtual void set_timeout(long timeout);
		virtual void replace_existing(bool replace);
		virtual void set_offset(off_t offset);
		virtual void set_nbstream(unsigned long nbstream);
		virtual void set_user_data(void* user_data);
		virtual void set_monitor(gfalt_monitor_tfr callback);
		
		virtual long get_timeout();
		virtual unsigned long get_nbstream();
		virtual void* get_user_data();
		
				
private:
	bool _lock; // lock enabled after the start of the transfer
	uuid_t _uuid; // unique id of the transfer
	long _timeout; 
	gboolean _replace_existing;
	off_t _start_offset;
	unsigned long _nbstream;
	gpointer _user_data;
	gfalt_monitor_tfr _callback;
	size_t buffer_size; // internal buffer size per flow for non-third party transfer
	
	void _check_params();
}; 


class FileCopy : public MainDecorator {
public:
	FileCopy(PluginFactory* wrap);
	virtual ~FileCopy();
	
	void start_copy(Params* p, const std::string & src, const std::string & dst);
	
protected:
	const plugin_filecopy_call find_copy_plugin(const std::string & src, const std::string & dst, void** plugin_data);
	
}; 

} // end Transfer

} //end Gfal



#endif //_GFAL2_TRANSFER_TYPES_INTERNAL_

