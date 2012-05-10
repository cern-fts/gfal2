#pragma once
#ifndef GRIDFTOMODULE_H
#define GRIDFTOMODULE_H

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

#include <algorithm>
#include <memory>

#include <exceptions/gfalcoreexception.hpp>
#include <common/gfal_types.h>
#include <common/gfal_constants.h>
#include <common/gfal_common_errverbose.h>
#include <transfer/gfal_transfer_plugins.h>
#include <transfer/params_plugin_interface.hpp>
#include <transfer/gfal_transfer.h>


#include "gridftpinterface.h"
#include "gridftpwrapper.h"

#include "gridftp_ifce_filecopy.h"
#include "gridftp_stat_module.h"
#include "gridftp_mkdir_module.h"
#include "gridftp_chmod_module.h"
#include "gridftp_rmdir_module.h"
#include "gridftp_opendir_module.h"
#include "gridftp_unlink_module.h"

typedef globus_gass_copy_glob_stat_t gfal_globus_stat_t;




class GridftpModule 
{
	public:
		GridftpModule(GridFTPFactoryInterface *);
		virtual ~GridftpModule();
		
		// Execute an access call, map on stat due to protocol restrictions
		virtual void access(const char*  path, int mode);

		//Execute a stat call on a gridftp URL
		virtual void stat(const char*  path, struct stat * st);
		
		// remove a file entry
		virtual void unlink(const char* path);

		 // Execute a mkdir query on path
		 virtual void mkdir(const char* path, mode_t mode);
		 
		 // Execute a chmod query on path
		virtual void chmod(const char* path, mode_t mode);
		
		// rmdir query on path
	    virtual void rmdir(const char* path);	
	    
	    // opendir query on path
	    virtual gfal_file_handle opendir(const char* path);
	    
	    // readdir a dir descriptor
	    virtual struct dirent* readdir(gfal_file_handle dh);
	    
	    virtual int closedir(gfal_file_handle fh);
		

		 // Execute a file transfer operation for gridftp URLs
		virtual int filecopy(gfalt_params_handle params, const char* src, const char* dst);
		
				 
		void internal_globus_gass_stat(const char* path,  gfal_globus_stat_t * gl_stat);

	private:
		GridFTPFactoryInterface * _handle_factory;
};

#endif /* GRIDFTOMODULE_H */ 
