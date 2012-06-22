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


/*
 * gfal_dcap_plugin_layer.c
 * file for the external call, abstraction layer for mock purpose
 * author Devresse Adrien
 */


#include <regex.h>
#include <time.h> 
#include <dlfcn.h>
#include <common/gfal_common_internal.h>
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_types.h>
#include "gfal_dcap_plugin_layer.h"



struct dcap_proto_ops * gfal_dcap_internal_loader_base(GError** err){
	struct dcap_proto_ops * pops = NULL;
	GError* tmp_err=NULL;
	
	gfal_log(GFAL_VERBOSE_VERBOSE, "    -> load dcap library ");	
	
	pops = g_new0(struct dcap_proto_ops, 1);
	pops->geterror = &__dc_errno;
	pops->strerror = &dc_strerror;
	pops->access = &dc_access;
	pops->chmod = &dc_chmod;
	pops->close = &dc_close;
	pops->closedir = &dc_closedir;
	pops->lseek = &dc_lseek;
	pops->lseek64 = &dc_lseek64;
	pops->lstat = &dc_lstat;
	pops->lstat64 = &dc_lstat64;
	pops->mkdir = &dc_mkdir;
	pops->open = &dc_open;
	pops->opendir = &dc_opendir;
	pops->read = &dc_read;
	pops->pread = &dc_pread;
	pops->readdir = &dc_readdir;
	pops->readdir64 = &dc_readdir64;
	pops->rename = NULL;
	pops->rmdir = &dc_rmdir;
	pops->stat = &dc_stat;
	pops->stat64 = &dc_stat64;
	pops->unlink = &dc_unlink;
	pops->write = &dc_write;
	pops->pwrite = &dc_pwrite;
	pops->debug_level= &dc_setDebugLevel;
	pops->active_mode = &dc_setClientActive;
	
	//
	pops->active_mode(); // switch to active mode to avoid firewalls problems
	if( (gfal_get_verbose() & GFAL_VERBOSE_TRACE ) )
		pops->debug_level(8 |6 | 32);
		
	gfal_log(GFAL_VERBOSE_VERBOSE, "  end load dcap library <-");	
	G_RETURN_ERR(pops, tmp_err, err);
}


struct dcap_proto_ops * (*gfal_dcap_internal_loader)(GError** err)= &gfal_dcap_internal_loader_base;

