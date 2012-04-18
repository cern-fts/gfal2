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
 * @file gfal_posix_local_file.c
 * @brief file for the local access file map for the gfal_posix call
 * @author Devresse Adrien
 * @version 2.0
 * @date 06/05/2011
 * */
 
#define _GNU_SOURCE
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>
#include <sys/stat.h>
#include <regex.h>
#include <attr/xattr.h>

#include "../common/gfal_common_errverbose.h"
#include "../common/gfal_common_filedescriptor.h"
#include "gfal_posix_local_file.h"

#define GFAL_LOCAL_PREFIX "file:"
#define GFAL_LOCAL_PREFIX_LEN 5

static regex_t rex;

int gfal_local_initG(GError** err){
	int ret = regcomp(&rex, "^file:([:print:]|/)+",REG_ICASE | REG_EXTENDED);
	g_return_val_err_if_fail(ret==0,-1,err,"[gfal_check_local_url] fail to compile regex, report this bug");
	return ret;
}

void gfal_local_report_error(const char* funcname, GError** err){
	g_set_error(err,0,errno, "[%s] errno reported by local system call %s", funcname, strerror(errno));	
}

 int gfal_local_access(const char *path, int amode, GError** err){
	const int res = access(path+strlen(GFAL_LOCAL_PREFIX), amode);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}else
		errno =0;
	return res;
 }
 
int gfal_local_chmod(const char* path, mode_t mode,GError** err){
	const int res = chmod(path+strlen(GFAL_LOCAL_PREFIX),mode);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}else
		errno =0;
	return res;
}

ssize_t gfal_local_getxattr(const char* path, const char* name, void* buff, size_t s_buff, GError** err){
	const ssize_t res = getxattr(path + strlen(GFAL_LOCAL_PREFIX), name, buff, s_buff);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}else
		errno =0;
	return res;
}

ssize_t gfal_local_listxattr(const char* path, char* list, size_t s_list, GError** err){
	const ssize_t res = listxattr(path + strlen(GFAL_LOCAL_PREFIX), list, s_list);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}else
		errno =0;
	return res;	
}

int gfal_local_setxattr(const char* path, const char* name, const void* value, size_t size, int flags, GError** err){
	const int res = setxattr(path + strlen(GFAL_LOCAL_PREFIX), name, value, size, flags);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}else
		errno =0;
	return res;	
}
	


int gfal_local_rename(const char* oldpath, const char* newpath, GError** err){
	const int res = rename(oldpath+strlen(GFAL_LOCAL_PREFIX), newpath + strlen(GFAL_LOCAL_PREFIX));
	if(res <0){
		gfal_local_report_error(__func__, err);
	}else
		errno =0;
	return res;
 }
 
int gfal_local_stat(const char* path, struct stat* buf, GError ** err){
	const int res = stat(path + strlen(GFAL_LOCAL_PREFIX) , buf);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}else
		errno =0;
	return res;
} 

int gfal_local_lstat(const char* path, struct stat* buf, GError ** err){
	const int res = lstat(path + strlen(GFAL_LOCAL_PREFIX), buf);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}else
		errno =0;
	return res;
}

int gfal_local_mkdir_rec(const char* full_path, mode_t mode){
	char *p;
	int res = -1, i;
	
	if( (res = mkdir(full_path, mode)) ==0 ||  errno != ENOENT )// try to create without recursive mode
		return res;	

	errno =0;
	size_t len = strnlen(full_path, GFAL_URL_MAX_LEN);
	char buff[len+1];
	
	i=0;
	p = (char*) full_path;
	while((p-full_path) < len){ // remove '/{2,+}' and last char if =='/'
		if( ( *p == '/' && ( *(p+1) == '/' || *(p+1) == '\0')) == FALSE)
			buff[i++] = *p;
		++p;
	}
	buff[i] = '\0';
				
	for(p = buff+1 ; *p != '\0'; p++){ // begin the recursive mode
		if(*p == '/' && *(p+1) != '/') { // check the '/' but skip the '//////' sequencies'
			*p = '\0';
			if( ((res =mkdir(buff, ( 0700 | mode) )) !=0) && errno != EEXIST && errno != EACCES)
				return res;
			*p = '/';
			errno =0;
		}
	}
	return mkdir(buff, mode);
}

int gfal_local_mkdir(const char* path, mode_t mode, GError** err){
	const int res = gfal_local_mkdir_rec(path + strlen(GFAL_LOCAL_PREFIX), mode);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}
	return res;	
}


int gfal_local_unlink(const char* path, GError** err){
	const int res = unlink(path + strlen(GFAL_LOCAL_PREFIX));
	if(res <0){
		gfal_local_report_error(__func__, err);
	}
	return res;		
}


gfal_file_handle gfal_local_opendir(const char* path, GError** err){
	DIR* ret = opendir(path+strlen(GFAL_LOCAL_PREFIX));
	gfal_file_handle resu = NULL;
	if(ret == NULL){
		gfal_local_report_error(__func__, err);
	}
	if(ret)
		resu = gfal_file_handle_new(GFAL_MODULEID_LOCAL, (gpointer) ret);
	return resu;
}

struct dirent* gfal_local_readdir(gfal_file_handle fh, GError** err){
	errno=0;
	struct dirent* res = readdir(fh->fdesc);
	if(res== NULL && errno){
		gfal_local_report_error(__func__, err);
	}
	return res;	 	
}

gfal_file_handle gfal_local_open(const char* path, int flag, mode_t mode, GError** err){
	errno =0;
	const int ret = open(path + strlen(GFAL_LOCAL_PREFIX), flag, mode);
	if(ret <=0){
		gfal_local_report_error(__func__, err);	
		return NULL;	
	}else{
		return gfal_file_handle_new(GFAL_MODULEID_LOCAL, GINT_TO_POINTER(ret));
	}
}

gboolean gfal_is_local_call(const char * module_name){
	return (strncmp(module_name, GFAL_MODULEID_LOCAL, GFAL_MODULE_NAME_SIZE) == 0);
}

/**
 *  map the gfal_read call to the local read call for file://
 * */
int gfal_local_read(gfal_file_handle fh, void* buff, size_t s_buff, GError** err){
	errno=0;
	const int fd = GPOINTER_TO_INT(fh->fdesc);
	const int ret = read(fd, buff, s_buff);
	if(ret <0)
		gfal_local_report_error(__func__, err);
	return ret;
}

/**
 *  map the gfal_pread call to the local pread call for file://
 * */
ssize_t gfal_local_pread(gfal_file_handle fh, void* buff, size_t s_buff, off_t offset, GError** err){
	errno=0;
	const int fd = GPOINTER_TO_INT(fh->fdesc);
	const ssize_t ret = pread(fd, buff, s_buff, offset);
	if(ret <0)
		gfal_local_report_error(__func__, err);
	return ret;
}

int gfal_local_lseek(gfal_file_handle fh, off_t offset, int whence, GError** err){
	errno=0;
	const int ret = lseek(GPOINTER_TO_INT(fh->fdesc), offset, whence);
	if(ret <0)
		gfal_local_report_error(__func__, err);
	return ret;
}

/**
 *  map to the local write call
 * */
int gfal_local_write(gfal_file_handle fh, void* buff, size_t s_buff, GError** err){
	errno=0;
	const int ret = write(GPOINTER_TO_INT(fh->fdesc), buff, s_buff);
	if(ret <0)
		gfal_local_report_error(__func__, err);
	return ret;
}

/**
 * map to the local pwrite call
 */
ssize_t gfal_local_pwrite(gfal_file_handle fh, void* buff, size_t s_buff, off_t offset, GError** err){
	errno=0;
	const ssize_t ret = pwrite(GPOINTER_TO_INT(fh->fdesc), buff, s_buff, offset);
	if(ret <0)
		gfal_local_report_error(__func__, err);
	return ret;
}

int gfal_local_close(gfal_file_handle fh, GError** err){
	errno =0;
	const int ret = close(GPOINTER_TO_INT(fh->fdesc));
	if(ret !=0){
		gfal_local_report_error(__func__, err);		
	}else
		free(fh);
	return ret;
}

ssize_t gfal_local_readlink(const char* path, char* buff, size_t buffsiz, GError** err){
	const ssize_t res = readlink(path + strlen(GFAL_LOCAL_PREFIX), buff, buffsiz);
	if(res <0){
		gfal_local_report_error(__func__, err);
	}else
		errno =0;
	return res;
}

/**
 * local rmdir mapper
 * */
int gfal_local_rmdir(const char* path, GError** err){
	const int res = rmdir(path+ strlen(GFAL_LOCAL_PREFIX));
	if(res<0)
		gfal_local_report_error(__func__, err);	
	return res;
}

/**
 *  local closedir mapper
 * 
 * */
int gfal_local_closedir(gfal_file_handle fh, GError** err){
	const int res = closedir(fh->fdesc);
	if(res<0)
		gfal_local_report_error(__func__, err);	
	else
		free(fh);
	return res;	 
 }
 
int gfal_local_symlink(const char* oldpath, const char* newpath, GError** err){
	const int res = symlink(oldpath + strlen(GFAL_LOCAL_PREFIX), newpath + strlen(GFAL_LOCAL_PREFIX));
	if(res !=0)
		gfal_local_report_error(__func__, err);
	return res;		
}
 
/**
 * check the validity of a classique file url
 * */ 
gboolean gfal_check_local_url(const char* path, GError** err){
	int ret=  regexec(&rex, path,0,NULL,0);
	return (!ret)?TRUE:FALSE;		
}

