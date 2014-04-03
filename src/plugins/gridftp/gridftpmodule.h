#pragma once
#ifndef GRIDFTOMODULE_H
#define GRIDFTOMODULE_H

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

#include <algorithm>
#include <memory>

#include <exceptions/gfalcoreexception.hpp>
#include <common/gfal_types.h>
#include <common/gfal_constants.h>
#include <common/gfal_common_err_helpers.h>
#include <transfer/gfal_transfer_plugins.h>
#include <transfer/gfal_transfer.h>


#include "gridftpinterface.h"
#include "gridftpwrapper.h"

#include "gridftp_filecopy.h"
#include "gridftp_namespace.h"

typedef globus_gass_copy_glob_stat_t gfal_globus_stat_t;



class GridftpModule 
{
	public:
		GridftpModule(GridFTPFactoryInterface *);
		virtual ~GridftpModule();
		
		// Execute an access call, map on stat due to protocol restrictions
		virtual void access(const char*  path, int mode);
		
		 // Execute a chmod query on path
		virtual void chmod(const char* path, mode_t mode);
		
		 // Execute a open query on path
		virtual gfal_file_handle open(const char* url, int flag, mode_t mode);
		
		// Execute a read/pread query
		virtual ssize_t read(gfal_file_handle handle, void* buffer, size_t count);			
		virtual ssize_t pread(gfal_file_handle handle, void* buffer, size_t count, off_t offset);
		
		// Execute a read/pread query
		virtual ssize_t write(gfal_file_handle handle, const void* buffer, size_t count);			
		virtual ssize_t pwrite(gfal_file_handle handle, const void* buffer, size_t count, off_t offset);		
		
		// seek a file
		virtual off_t lseek(gfal_file_handle handle, off_t offset, int whence);	
		
		// close a file 
		virtual int close(gfal_file_handle handle);
		
		//Execute a stat call on a gridftp URL
		virtual void stat(const char*  path, struct stat * st);
		
		// remove a file entry
		virtual void unlink(const char* path);

		 // Execute a mkdir query on path
		 virtual void mkdir(const char* path, mode_t mode);
		 
        virtual void checksum(const char* url, const char* check_type,
                               char * checksum_buffer, size_t buffer_length,
                               off_t start_offset, size_t data_length);

        // Rename
        virtual void rename(const char* src, const char* dst);
		
		// rmdir query on path
	    virtual void rmdir(const char* path);	

        void autoCleanFileCopy(gfalt_params_t params, GError* checked_error, const char* dst);

		 // Execute a file transfer operation for gridftp URLs
		virtual void filecopy(gfalt_params_t params, const char* src, const char* dst);
		

				 
		void internal_globus_gass_stat(const char* path,  gfal_globus_stat_t * gl_stat);

		GridFTPFactoryInterface* get_session_factory() {
		    return _handle_factory;
		}

	private:
		GridFTPFactoryInterface * _handle_factory;

};


void core_init();

#endif /* GRIDFTOMODULE_H */ 
