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


/**
 * @file gfal_dcap_plugin_main.c
 * @brief file for bindings for dcap funcs
 * @author Devresse Adrien
 * @date 20/07/2011
 * 
 **/


#include <regex.h>
#include <time.h> 
#include <glib.h>
#include <stdlib.h>
#include <fcntl.h>
#include "../gfal_common_internal.h"
#include "../gfal_common_errverbose.h"
#include "../gfal_common_plugin.h"
#include "../gfal_common_filedescriptor.h"
#include "../gfal_types.h"
#include "gfal_dcap_plugin_layer.h"
#include "gfal_dcap_plugin_bindings.h"
#include "gfal_dcap_plugin_main.h"

static void dcap_report_error(gfal_plugin_dcap_handle h,  const char * func_name, GError** err){
	char buff_error[2048];
	const int status = *(h->ops->geterror());
	g_strlcpy(buff_error, h->ops->strerror(status), 2048);
	g_set_error(err, 0, status, "[%s] Error reported by the external library dcap : %s, number : %d ", func_name, buff_error, status);
}

gfal_file_handle gfal_dcap_openG(plugin_handle handle , const char* path, int flag, mode_t mode, GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	gfal_file_handle ret = NULL;
	const int internal_flag = flag & (~ 0100000) ; // filter non supported flags
	int fd= h->ops->open(path, internal_flag, mode);
	if(fd == -1)
		dcap_report_error(h, __func__, err);
	else 
		ret = gfal_file_handle_new(gfal_dcap_getName(), GINT_TO_POINTER(fd));
	return ret;
}

/**
 * map to the libdcap read call
 * 
 * */
int gfal_dcap_readG(plugin_handle handle , gfal_file_handle fd, void* buff, size_t s_buff, GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	int ret = h->ops->read(GPOINTER_TO_INT(fd->fdesc), buff, s_buff);
	if(ret <0)
		dcap_report_error(h, __func__, err);
	else
		errno =0;
	return ret;
}

/**
 * map to the libdcap pread call
 * 
 * */
ssize_t gfal_dcap_preadG(plugin_handle handle , gfal_file_handle fd, void* buff, size_t s_buff, off_t offset,  GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	ssize_t ret = h->ops->pread(GPOINTER_TO_INT(fd->fdesc), buff, s_buff, offset);
	if(ret <0)
		dcap_report_error(h, __func__, err);
	else
		errno =0;
	return ret;
}

/**
 * map to the libdcap pwrite call
 * 
 * */
ssize_t gfal_dcap_pwriteG(plugin_handle handle , gfal_file_handle fd, const void* buff, size_t s_buff, off_t offset,  GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	ssize_t ret = h->ops->pwrite(GPOINTER_TO_INT(fd->fdesc), buff, s_buff, offset);
	if(ret <0)
		dcap_report_error(h, __func__, err);
	else
		errno =0;
	return ret;
}

int gfal_dcap_lseekG(plugin_handle handle , gfal_file_handle fd, off_t offset, int whence, GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	off_t ret = h->ops->lseek(GPOINTER_TO_INT(fd->fdesc), offset, (int) whence);
	if(ret == ((off_t)0)-1)
		dcap_report_error(h, __func__, err);
	else
		errno =0;
	return ret;
}

int gfal_dcap_writeG(plugin_handle handle , gfal_file_handle fd, void* buff, size_t s_buff, GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	int ret = h->ops->write(GPOINTER_TO_INT(fd->fdesc), buff, s_buff);
	if(ret <0)
		dcap_report_error(h, __func__, err);
	else
		errno =0;
	return ret;
}




int gfal_dcap_closeG(plugin_handle handle, gfal_file_handle fd, GError ** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	int ret= h->ops->close(GPOINTER_TO_INT(fd->fdesc));
	if(ret != 0){
		dcap_report_error(h, __func__, err);
	}else
		free(fd);
	return ret;	
}


int gfal_dcap_statG(plugin_handle handle, const char* name, struct stat* buff, GError ** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	int ret= h->ops->stat(name, buff);
	if(ret != 0){
		dcap_report_error(h, __func__, err);
	}
	return ret;	
}

int gfal_dcap_lstatG(plugin_handle handle, const char* name, struct stat* buff, GError ** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	int ret= h->ops->lstat(name, buff);
	if(ret != 0){
		dcap_report_error(h, __func__, err);
	}
	return ret;	
}

int gfal_dcap_mkdirG(plugin_handle handle, const char* name, mode_t mode, gboolean pflag, GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	int ret = h->ops->mkdir(name, mode);	
	if(ret !=0){
		dcap_report_error(h, __func__, err);		
	}
	return ret;
}


const char* gfal_dcap_getName(){
	return "dcap";
}

