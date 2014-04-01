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
 * gfal_dcap_plugin_main.c
 * file for bindings for dcap funcs
 * author Devresse Adrien
 */


#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>


#include <regex.h>
#include <time.h> 
#include <glib.h>
#include <stdlib.h>
#include <fcntl.h>
#include <common/gfal_common_internal.h>
#include <common/gfal_common_err_helpers.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_filedescriptor.h>
#include <common/gfal_types.h>
#include "gfal_dcap_plugin_layer.h"
#include "gfal_dcap_plugin_bindings.h"
#include "gfal_dcap_plugin_main.h"


// errno conversion for dcap non-complete error messages
static int dcap_errno_conversion( const char * msg, int old_err){
	int ret;
	ret = old_err;	
	switch(old_err){
		case EIO:
			if(strstr(msg, "o such"))
				ret = ENOENT;
			break;
		case EACCES:
			if(strstr(msg, "ectory not empty"))
				ret = ENOTEMPTY;
			break;
		default:
			ret = old_err;
	}
	return ret;
}

static void dcap_report_error(gfal_plugin_dcap_handle h,  const char * func_name, GError** err){
	char buff_error[2048];
	const int status = *(h->ops->geterror());
	g_strlcpy(buff_error, h->ops->strerror(status), 2048);
	// errno conversion
	errno = dcap_errno_conversion(buff_error, errno);
    g_set_error(err, gfal2_get_plugin_dcap_quark(), errno, "[%s] Error reported by the external library dcap : %s, number : %d ", func_name, buff_error, status);
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

/*
 * map to the libdcap read call
 * 
 */
ssize_t gfal_dcap_readG(plugin_handle handle , gfal_file_handle fd, void* buff, size_t s_buff, GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	ssize_t ret = h->ops->read(GPOINTER_TO_INT(fd->fdesc), buff, s_buff);
	if(ret <0)
		dcap_report_error(h, __func__, err);
	else
		errno =0;
	return ret;
}

/*
 * map to the libdcap pread call
 */
ssize_t gfal_dcap_preadG(plugin_handle handle , gfal_file_handle fd, void* buff, size_t s_buff, off_t offset,  GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	ssize_t ret = h->ops->pread(GPOINTER_TO_INT(fd->fdesc), buff, s_buff, offset);
	if(ret <0)
		dcap_report_error(h, __func__, err);
	else
		errno =0;
	return ret;
}

/*
 * map to the libdcap pwrite call
 */
ssize_t gfal_dcap_pwriteG(plugin_handle handle , gfal_file_handle fd, const void* buff, size_t s_buff, off_t offset,  GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	ssize_t ret = h->ops->pwrite(GPOINTER_TO_INT(fd->fdesc), buff, s_buff, offset);
	if(ret <0)
		dcap_report_error(h, __func__, err);
	else
		errno =0;
	return ret;
}

off_t gfal_dcap_lseekG(plugin_handle handle , gfal_file_handle fd, off_t offset, int whence, GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	off_t ret = h->ops->lseek(GPOINTER_TO_INT(fd->fdesc), offset, (int) whence);
	if(ret == ((off_t)0)-1)
		dcap_report_error(h, __func__, err);
	else
		errno =0;
	return ret;
}

ssize_t gfal_dcap_writeG(plugin_handle handle , gfal_file_handle fd, const void* buff, size_t s_buff, GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	ssize_t ret = h->ops->write(GPOINTER_TO_INT(fd->fdesc), buff, s_buff);
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
	}
    gfal_file_handle_delete(fd);
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

int gfal_dcap_accessG(plugin_handle plugin_data, const char* url, int mode, GError** err){
gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) plugin_data;
	ssize_t ret = h->ops->access(url, mode);
	if(ret <0)
		dcap_report_error(h, __func__, err);
	else
		errno =0;
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

int gfal_dcap_chmodG(plugin_handle handle, const char* name, mode_t mode,  GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	int ret = h->ops->chmod(name, mode);	
	if(ret !=0){
		dcap_report_error(h, __func__, err);		
	}
	return ret;
}


int gfal_dcap_rmdirG(plugin_handle handle, const char* name, GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	int ret = h->ops->rmdir(name);	
	if(ret !=0){
		dcap_report_error(h, __func__, err);		
	}
	return ret;
}

gfal_file_handle gfal_dcap_opendirG(plugin_handle handle, const char* path, GError ** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	DIR * d = h->ops->opendir(path);	
	gfal_file_handle ret = NULL;
	if(d == NULL){
		dcap_report_error(h, __func__, err);		
	}else{
        ret = gfal_file_handle_new2(gfal_dcap_getName(), (gpointer) d, NULL, path);
	}
	return ret;	
}

int gfal_dcap_closedirG(plugin_handle handle, gfal_file_handle fh, GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	int ret = h->ops->closedir(fh->fdesc);	
	if(ret !=0){
		dcap_report_error(h, __func__, err);		
	}else{
		gfal_file_handle_delete(fh);
	}
	return ret;	
}

struct dirent* gfal_dcap_readdirG(plugin_handle handle, gfal_file_handle fh, GError** err){
	gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
	struct dirent* ret = h->ops->readdir(fh->fdesc);	
	if(ret == NULL && *(h->ops->geterror()) != 0){
		dcap_report_error(h, __func__, err);		
	}
	return ret;	
	
}

int gfal_dcap_unlinkG(plugin_handle handle, const char* url, GError** err)
{
    gfal_plugin_dcap_handle h = (gfal_plugin_dcap_handle) handle;
    int ret = h->ops->unlink(url);
    if(ret != 0){
        dcap_report_error(h, __func__, err);
    }
    return ret;
}

const char* gfal_dcap_getName(){
    return "dcap_plugin";
}

