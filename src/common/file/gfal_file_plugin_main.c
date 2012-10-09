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
 * file gfal_file_plugin_main.c
 * brief file plugin
 * author Devresse Adrien
 */
 
#define _GNU_SOURCE 

#include <regex.h>
#include <time.h> 
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>
#include <sys/stat.h>
#include <attr/xattr.h>

#include <common/gfal_constants.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_errverbose.h>
#include <fdesc/gfal_file_handle.h>

const char* file_prefix="file:";
unsigned int s_prefix = 0;


static unsigned int file_prefix_len(){
    if(!s_prefix)
        g_atomic_int_set(&s_prefix, strlen(file_prefix));
    return s_prefix;
}

/*
 * srm plugin id
 */
const char* gfal_file_plugin_getName(){
	return "file_plugin";
}


int gfal_lfile_path_checker(plugin_handle handle, const char * url){
    const unsigned int s_url = strnlen(url,GFAL_URL_MAX_LEN);
    if( file_prefix_len() <  s_url && s_url < GFAL_URL_MAX_LEN
            && strncmp(url, file_prefix, s_prefix) ==0)
        return 0;
    return -1;
}


/*
 * url checker for the file module
 **/
static gboolean gfal_file_check_url(plugin_handle handle, const char* url, plugin_mode mode, GError** err){
    g_return_val_err_if_fail(url != NULL, EINVAL, err, "[gfal_lfile_path_checker] Invalid url ");
	switch(mode){
        case GFAL_PLUGIN_ACCESS:
        case GFAL_PLUGIN_MKDIR:
		case GFAL_PLUGIN_STAT:
		case GFAL_PLUGIN_LSTAT:
		case GFAL_PLUGIN_RMDIR:
		case GFAL_PLUGIN_OPENDIR:
		case GFAL_PLUGIN_OPEN:
		case GFAL_PLUGIN_CHMOD:
		case GFAL_PLUGIN_UNLINK:
		case GFAL_PLUGIN_GETXATTR:
		case GFAL_PLUGIN_LISTXATTR:
        case GFAL_PLUGIN_SETXATTR:
        case GFAL_PLUGIN_RENAME:
        case GFAL_PLUGIN_SYMLINK:
        //case GFAL_PLUGIN_CHECKSUM:
            return (gfal_lfile_path_checker(handle, url)==0);
		default:
			return FALSE;		
	}
}



void gfal_plugin_file_report_error(const char* funcname, GError** err){
    g_set_error(err,0,errno, "[%s] errno reported by local system call %s", funcname, strerror(errno));
}

 int gfal_plugin_file_access(plugin_handle plugin_data, const char *path, int amode, GError** err){
    const int res = access(path+file_prefix_len(), amode);
    if(res <0){
        gfal_plugin_file_report_error(__func__, err);
    }else
        errno =0;
    return res;
 }

int gfal_plugin_file_chmod(plugin_handle plugin_data, const char* path, mode_t mode,GError** err){
    const int res = chmod(path+file_prefix_len(),mode);
    if(res <0){
        gfal_plugin_file_report_error(__func__, err);
    }else
        errno =0;
    return res;
}

ssize_t gfal_plugin_file_getxattr(plugin_handle plugin_data, const char* path, const char* name, void* buff, size_t s_buff, GError** err){
    const ssize_t res = getxattr(path + file_prefix_len(), name, buff, s_buff);
    if(res <0){
        gfal_plugin_file_report_error(__func__, err);
    }else
        errno =0;
    return res;
}

ssize_t gfal_plugin_file_listxattr(plugin_handle plugin_data, const char* path, char* list, size_t s_list, GError** err){
    const ssize_t res = listxattr(path + file_prefix_len(), list, s_list);
    if(res <0){
        gfal_plugin_file_report_error(__func__, err);
    }else
        errno =0;
    return res;
}

int gfal_plugin_file_setxattr(plugin_handle plugin_data, const char* path, const char* name, const void* value, size_t size, int flags, GError** err){
    const int res = setxattr(path + file_prefix_len(), name, value, size, flags);
    if(res <0){
        gfal_plugin_file_report_error(__func__, err);
    }else
        errno =0;
    return res;
}



int gfal_plugin_file_rename(plugin_handle plugin_data, const char* oldpath, const char* newpath, GError** err){
    const int res = rename(oldpath+file_prefix_len(), newpath + strlen(file_prefix));
    if(res <0){
        gfal_plugin_file_report_error(__func__, err);
    }else
        errno =0;
    return res;
 }

int gfal_plugin_file_stat(plugin_handle plugin_data, const char* path, struct stat* buf, GError ** err){
    const int res = stat(path + file_prefix_len() , buf);
    if(res <0){
        gfal_plugin_file_report_error(__func__, err);
    }else
        errno =0;
    return res;
}

int gfal_plugin_file_lstat(plugin_handle plugin_data, const char* path, struct stat* buf, GError ** err){
    const int res = lstat(path + file_prefix_len(), buf);
    if(res <0){
        gfal_plugin_file_report_error(__func__, err);
    }else
        errno =0;
    return res;
}

int gfal_plugin_file_mkdir(plugin_handle plugin_data, const char* path, mode_t mode, gboolean pflag, GError** err){
    const int res = mkdir(path + file_prefix_len(), mode);
    if(res <0){
        gfal_plugin_file_report_error(__func__, err);
    }
    return res;
}


int gfal_plugin_file_unlink(plugin_handle plugin_data, const char* path, GError** err){
    const int res = unlink(path + file_prefix_len());
    if(res <0){
        gfal_plugin_file_report_error(__func__, err);
    }
    return res;
}


gfal_file_handle gfal_plugin_file_opendir(plugin_handle plugin_data, const char* path, GError** err){
    DIR* ret = opendir(path+file_prefix_len());
    gfal_file_handle resu = NULL;
    if(ret == NULL){
        gfal_plugin_file_report_error(__func__, err);
    }
    if(ret)
        resu = gfal_file_handle_new(gfal_file_plugin_getName(), (gpointer) ret);
    return resu;
}

struct dirent* gfal_plugin_file_readdir(plugin_handle plugin_data, gfal_file_handle fh, GError** err){
    errno=0;
    struct dirent* res = readdir(gfal_file_handle_get_fdesc(fh));
    if(res== NULL && errno){
        gfal_plugin_file_report_error(__func__, err);
    }
    return res;
}

gfal_file_handle gfal_plugin_file_open(plugin_handle plugin_data, const char* path, int flag, mode_t mode, GError** err){
    errno =0;
    const int ret = open(path + file_prefix_len(), flag, mode);
    if(ret <=0){
        gfal_plugin_file_report_error(__func__, err);
        return NULL;
    }else{
        return gfal_file_handle_new(gfal_file_plugin_getName(), GINT_TO_POINTER(ret));
    }
}


/*
 *  map the gfal_read call to the local read call for file://
 * */
ssize_t gfal_plugin_file_read(plugin_handle plugin_data, gfal_file_handle fh, void* buff, size_t s_buff, GError** err){
    errno=0;
    const int fd = GPOINTER_TO_INT(gfal_file_handle_get_fdesc(fh));
    const int ret = read(fd, buff, s_buff);
    if(ret <0)
        gfal_plugin_file_report_error(__func__, err);
    return ret;
}

/*
 *  map the gfal_pread call to the local pread call for file://
 * */
ssize_t gfal_plugin_file_pread(plugin_handle plugin_data, gfal_file_handle fh, void* buff, size_t s_buff, off_t offset, GError** err){
    errno=0;
    const int fd = GPOINTER_TO_INT(gfal_file_handle_get_fdesc(fh));
    const ssize_t ret = pread(fd, buff, s_buff, offset);
    if(ret <0)
        gfal_plugin_file_report_error(__func__, err);
    return ret;
}

off_t gfal_plugin_file_lseek(plugin_handle plugin_data, gfal_file_handle fh, off_t offset, int whence, GError** err){
    errno=0;
    const int ret = lseek(GPOINTER_TO_INT(gfal_file_handle_get_fdesc(fh)), offset, whence);
    if(ret <0)
        gfal_plugin_file_report_error(__func__, err);
    return ret;
}

/*
 *  map to the local write call
 * */
ssize_t gfal_plugin_file_write(plugin_handle plugin_data, gfal_file_handle fh, const void* buff, size_t s_buff, GError** err){
    errno=0;
    const int ret = write(GPOINTER_TO_INT(gfal_file_handle_get_fdesc(fh)), buff, s_buff);
    if(ret <0)
        gfal_plugin_file_report_error(__func__, err);
    return ret;
}

/*
 * map to the local pwrite call
 */
ssize_t gfal_plugin_file_pwrite(plugin_handle plugin_data, gfal_file_handle fh, const void* buff, size_t s_buff, off_t offset, GError** err){
    errno=0;
    const ssize_t ret = pwrite(GPOINTER_TO_INT(gfal_file_handle_get_fdesc(fh)), buff, s_buff, offset);
    if(ret <0)
        gfal_plugin_file_report_error(__func__, err);
    return ret;
}

int gfal_plugin_file_close(plugin_handle plugin_data, gfal_file_handle fh, GError** err){
    errno =0;
    const int ret = close(GPOINTER_TO_INT(gfal_file_handle_get_fdesc(fh)));
    if(ret !=0){
        gfal_plugin_file_report_error(__func__, err);
    }else
        gfal_file_handle_delete(fh);
    return ret;
}

ssize_t gfal_plugin_file_readlink(plugin_handle plugin_data, const char* path, char* buff, size_t buffsiz, GError** err){
    const ssize_t res = readlink(path + file_prefix_len(), buff, buffsiz);
    if(res <0){
        gfal_plugin_file_report_error(__func__, err);
    }else
        errno =0;
    return res;
}

/*
 * local rmdir mapper
 * */
int gfal_plugin_file_rmdir(plugin_handle plugin_data, const char* path, GError** err){
    const int res = rmdir(path+ file_prefix_len());
    if(res<0)
        gfal_plugin_file_report_error(__func__, err);
    return res;
}

/*
 *  local closedir mapper
 *
 * */
int gfal_plugin_file_closedir(plugin_handle plugin_data, gfal_file_handle fh, GError** err){
    const int res = closedir(gfal_file_handle_get_fdesc(fh));
    if(res<0)
        gfal_plugin_file_report_error(__func__, err);
    else
        gfal_file_handle_delete(fh);
    return res;
 }

int gfal_plugin_file_symlink(plugin_handle plugin_data, const char* oldpath, const char* newpath, GError** err){
    const int res = symlink(oldpath + file_prefix_len(), newpath + strlen(file_prefix));
    if(res !=0)
        gfal_plugin_file_report_error(__func__, err);
    return res;
}



/*
 * Init function, called before all
 * */
gfal_plugin_interface gfal_plugin_init(gfal_handle handle, GError** err){
	gfal_plugin_interface file_plugin;
    memset(&file_plugin,0,sizeof(gfal_plugin_interface));	// clear the plugin

    file_plugin.plugin_data = NULL;
    file_plugin.check_plugin_url = &gfal_file_check_url;
    file_plugin.getName= &gfal_file_plugin_getName;
    file_plugin.plugin_delete = NULL;
    file_plugin.accessG = &gfal_plugin_file_access;
    file_plugin.mkdirpG = &gfal_plugin_file_mkdir;
    file_plugin.statG = &gfal_plugin_file_stat;
    file_plugin.lstatG = &gfal_plugin_file_lstat;
    file_plugin.renameG = &gfal_plugin_file_rename;
    file_plugin.symlinkG = &gfal_plugin_file_symlink;
    file_plugin.rmdirG = &gfal_plugin_file_rmdir;
    file_plugin.opendirG = &gfal_plugin_file_opendir;
    file_plugin.readdirG = &gfal_plugin_file_readdir;
    file_plugin.closedirG = &gfal_plugin_file_closedir;


    file_plugin.openG = &gfal_plugin_file_open;
    file_plugin.closeG = &gfal_plugin_file_close;
    file_plugin.readG= &gfal_plugin_file_read;
    file_plugin.preadG = &gfal_plugin_file_pread;
    file_plugin.writeG= &gfal_plugin_file_write;
    file_plugin.pwriteG = &gfal_plugin_file_pwrite;
    file_plugin.chmodG= &gfal_plugin_file_chmod;
    file_plugin.lseekG= &gfal_plugin_file_lseek;
    file_plugin.unlinkG = &gfal_plugin_file_unlink;
    file_plugin.getxattrG = &gfal_plugin_file_getxattr;
    file_plugin.listxattrG = &gfal_plugin_file_listxattr;

 /*    file_plugin. = &gfal_srm_mkdirG;
    file_plugin.statG= &gfal_srm_statG;
    file_plugin.lstatG = &gfal_srm_statG; // no management for symlink in srm protocol/srm-ifce, just map to stat
    file_plugin.rmdirG = &gfal_srm_rmdirG;
    file_plugin.opendirG = &gfal_srm_opendirG;
    file_plugin.readdirG = &gfal_srm_readdirG;
    file_plugin.closedirG = &gfal_srm_closedirG;

    file_plugin.openG = &gfal_srm_openG;
    file_plugin.closeG = &gfal_srm_closeG;
    file_plugin.readG= &gfal_srm_readG;
    file_plugin.preadG = &gfal_srm_preadG;
    file_plugin.writeG= &gfal_srm_writeG;
    file_plugin.chmodG= &gfal_srm_chmodG;
    file_plugin.lseekG= &gfal_srm_lseekG;
    file_plugin.unlinkG = &gfal_srm_unlinkG;
    file_plugin.getxattrG = &gfal_srm_getxattrG;
    file_plugin.listxattrG = &gfal_srm_listxattrG;
    file_plugin.checksum_calcG = &gfal_srm_checksumG;
    file_plugin.copy_file = &file_plugin_filecopy;
    file_plugin.check_plugin_url_transfer =&plugin_url_check2;*/
	return file_plugin;
}



