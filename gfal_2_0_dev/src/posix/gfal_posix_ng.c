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
 * @file gfal_posix_ng.c
 * @brief new file for the posix interface
 * @author Devresse Adrien
 * @date 09/05/2011
 * */


#include <common/gfal_constants.h>

#include "gfal_posix_api.h"
#include "gfal_posix_internal.h"






/**
 * \brief test access to the given file
 * \param path path of the file to access, can be in supported protocols (lfn, srm, file, guid,..)
 * \param amode access mode to check (R_OK, W_OK, X_OK or F_OK)
 * \return This routine return 0 if the operation was successful, or -1 if error occured and errno is set, call @ref gfal_posix_check_error() to check it.
 * 
 * 
 * 
 */
int gfal_access (const char *path, int amode){
	return gfal_posix_internal_access(path, amode);	
}

/**
 * @brief change the right for a file or a folder
 * @param path : path of the file or the folder, can be in all supported protocols (lfn, srm, file, guid,..)
 * @param mode : right to configure
 * @return return 0 if success else -1 and errno is set, call @ref gfal_posix_check_error() to check it
 * 		
 */
int gfal_chmod(const char* path, mode_t mode){
	return gfal_posix_internal_chmod(path, mode);
}

/**
 * @brief  change the name or location of a file
 * oldpath and newpath need to be on the same plugin
 * this functions work only with plugins (lfc ) and local files
 * @param oldpath : the old path of the file, can be in supported protocols but need to be in the same adress space than newpath
 * @param newpath : the new path of the file, can be in supported protocols (lfn, srm, file, guid,..)
 * @return : return 0 if success, else -1 and errno / @ref gfal_posix_check_error()
 * 
 *  
*/
int gfal_rename(const char *oldpath, const char *newpath){
	return gfal_posix_internal_rename(oldpath, newpath);
}

/**
 *  @brief  informations about a file 
 * These functions return information about a file.  No permissions are required on the file itself, but — in the case of stat() and lstat() — execute (search) permission is
 *     required on all of the directories in path that lead to the file.
 * @param path : path of the file, can be in supported protocols (lfn, srm, file, guid,..)
 * @param buff : pointer to an allocated struct stat
 * @return return 0 if success else -1 and errno is set, call @ref gfal_posix_check_error() to check it
 * 
 * */
int gfal_stat(const char* path, struct stat* buff){
	return gfal_posix_internal_stat(path, buff);
}

/**
 * @brief gfal_lstat is identical to \ref gfal_stat except for symbolic links. 
 * In this case, the link itself is statted and not
       followed.
*/
int gfal_lstat(const char* path, struct stat* buff){
	return gfal_posix_internal_lstat(path, buff);
}
/**
 * @brief  create a new directory
 * creates a new directory with permission bits taken from mode.
 *  The default behavior of this command is recursive, like "mkdir -p".
 * @param path : url of the directory, can be in supported protocols (lfn, srm, file, guid,..)
 * @param mode : right of the directory ( depend of the implementation )
 * @return return 0 if success else -1 and errno is set call @ref gfal_posix_check_error() to check it
 * 
 */
int gfal_mkdirp( const char* path, mode_t mode){
	return  gfal_posix_internal_mkdir( path, mode);
	
}
/**
 * Wrapper to mkdir for comptibility, same behavior than \ref gfal_mkdirp ( but subject to change in order to follow POSIX mkdir in the futur )
 */
int gfal_mkdir( const char* path, mode_t mode){
	return  gfal_mkdirp( path, mode);
	
}

/**
 * @brief  removes a directory if it is empty
 * remove an existing directory, return error if the dir is not empty
 *  @param path specifies the directory name, can be in supported protocols (lfn, srm, file, guid,..)
 *  @return return 0 is success else -1 and errno is set call @ref gfal_posix_check_error() to check it
 * 
 * */
int gfal_rmdir(const char* path){
	return gfal_posix_internal_rmdir(path);
}

/**
 * @brief  open a directory
 * 
 * opens a directory to be used in subsequent gfal_readdir operations
 * the url supported are : local files, surls, plugin url ( lfc,...)
 * @param name of the directory to open, can be in supported protocols (lfn, srm, file, guid,..)
 * @return file descriptor DIR* if success else NULL if error and errno is set call @ref gfal_posix_check_error() to check it
 * 
 * */
DIR* gfal_opendir(const char* name){
	return gfal_posix_internal_opendir(name);
}

/**
 * @brief  read a directory
 * similar to the POSIX call readdir
 * The readdir() function returns a pointer to a dirent structure representing the next directory entry in the directory stream pointed to by dirp.  It returns NULL on
 *      reaching the end of the directory stream or if an error occurred.
 *            struct dirent {
 *             ino_t          d_ino;       // inode number 
 *             off_t          d_off;       // offset to the next dirent 
 *             unsigned short d_reclen;    // length of this record 
 *             unsigned char  d_type;      // type of file; not supported
 *                                         //  by all file system types 
 *             char           d_name[256]; // filename 
 *         };
 *
 * 
 * @param d file handle ( return by opendir ) to read
 * @return pointer to struct dirent with file information or NULL if end of list or error, errno is set call @ref gfal_posix_check_error() to check it
 * @warning struct dirents are allocated statically, do not use free() on them
 * 
 * 
 * */
struct dirent* gfal_readdir(DIR* d){
	return gfal_posix_internal_readdir(d);	
}

/**
 * @brief  close a file descriptor of a directory
 *  similar to the POSIX call closedir
 *  close the file descriptor of an opendir call
 * 
 * @param d file handle ( return by opendir ) to close
 * @return 0 if success else negativevalue and errno is set (  ( gfal_posix_error_print() )
 * 
 * 
 * 
 * */
int gfal_closedir(DIR* d){
	return gfal_posix_internal_closedir(d);
}

/**
 *  @brief open a file
 *  similar to the POSIX call open
 * 	opens a file according to the value of flags.
 *  @param path : url of the filename to open. can be in supported protocols (lfn, srm, file, guid,..)
 *  @param flag : same flag supported value is built by OR’ing the bits defined in <fcntl.h> but one and only one of the first three flags below must be used
 *            O_RDONLY    open for reading only
 *            O_WRONLY    open for writing only
 *            O_RDWR      open for reading and writing
 *            O_CREAT     If the file exists already and O_EXCL is also set, gfal_open will fail
 *            O_LARGEFILE allows files whose sizes cannot be represented in 31 bits to be opened
 *  @param mode is used only if the file is created.
 *  @return return the file descriptor or -1 if errno is set call @ref gfal_posix_check_error() to check it
 * */
int gfal_open(const char * path, int flag, ...){
	mode_t mode = S_IRWXU | S_IRGRP | S_IROTH;
	va_list va;
	va_start(va, flag);
	mode = va_arg(va, mode_t);
	va_end(va);
	return gfal_posix_internal_open(path, flag, mode);
}



/**
 *  @brief  create a new file or truncate an existing one
 *  similar to the POSIX call creat
 * 	opens a file according to the value of flags.
 *  @param filename : url of the filename to create, can be in supported protocols (lfn, srm, file, guid,..)
 *  @param mode : is used only if the file is created.
 *  @return return the file descriptor or -1 if errno is set call @ref gfal_posix_check_error() to check it
 * */
int gfal_creat (const char *filename, mode_t mode){
    return (gfal_open (filename, O_WRONLY|O_CREAT|O_TRUNC, mode));
}

/**
 *  @brief read a file
 *  similar to the POSIX call read
 * 	gfal_read reads up to size bytes from the file descriptor fd into the buffer pointed by buff
 *  @param fd file descriptor
 *  @param buff buffer of the data to read
 *  @param s_buff size of the data read in bytes
 *  @return number of byte read or -1 if error, errno is set call @ref gfal_posix_check_error() to check it 
 */
int gfal_read(int fd, void* buff, size_t s_buff){
	return gfal_posix_internal_read(fd, buff, s_buff);
}
/**
 *  @brief write a file
 *  similar to the POSIX call write
 *  gfal_write writes size bytes from the buffer pointed by buff to the file descriptor fd.
 *  @param fd file descriptor
 *  @param buff buffer of the data to write
 *  @param s_buff size of the data write in bytes
 *  @return number of byte write or -1 if error, errno is set call @ref gfal_posix_check_error() to check it   
 */
int gfal_write(int fd, const void *buff, size_t s_buff){
	return gfal_posix_internal_write(fd, (void*) buff, s_buff);
}

/**
 *  @brief  close a file
 *  similar to the POSIX call close
 * 	closes the file whose descriptor fd is the one returned by gfal_open.
 *  @param fd : descriptor or the file given by @ref gfal_open
 *  @return This routine returns 0 if the operation was successful or -1 if the operation failed. In the latter case, errno is set appropriately
 * 
 * */
int gfal_close(int fd){
	return gfal_posix_internal_close(fd);
}

/**
 * @brief make a new name for a file
 *  
 *  similar to the POSIX call symlink .
 *  symlink() creates a symbolic link named newpath which contains the string oldpath.
 * @param newpath : path of the link, can be in supported protocols but need to be in the same adress space than newpath
 * @param oldpath : path of the linked file, can be in supported protocols (lfn, srm, file, guid,..)
 * @return 0 if success else -1.  if failure, errno is set, you can call @ref gfal_posix_check_error() for a more complete description. 
*/
int gfal_symlink(const char* oldpath, const char * newpath){
	return gfal_posix_internal_symlink(oldpath, newpath);
}

/**
 * @brief set position in a file 
 * 		
 *  similar to the POSIX call lseek
 *  gfal_lseek  positions/repositions  to  offset  the file associated with the descriptor fd generated by a previous gfal_open.  whence indicates how to interpret the offset
 *  value:
 *
 *            SEEK_SET     The offset is set from beginning of file.
 *
 *            SEEK_CUR     The offset is added to current position.
 *
 *            SEEK_END     The offset is added to current file size.
 * @param fd : file descriptor to lseek
 * @param offset: offset in byte
 * @param whence:  flag
 * @return  This routine returns the actual offset from the beginning of the file if the operation was successful or -1 if the operation failed. In the  latter  case,  errno  is  set
       appropriately, you can call @ref gfal_posix_check_error() for a more complete description. 
*/
off_t gfal_lseek (int fd, off_t offset, int whence){
	return gfal_posix_internal_lseek(fd, offset, whence);
}

/**
 * @brief  retrieve an extended attribute value
 * 
 * similar to the getxattr call of the libattr
 * gfal_getxattr retrieves an extended value for an url in a supported protocol.
 * The extended attributes are use for the advanced file operations ( like set/get replicas, grid status, comments, etc... )
 * @param path : path of the file/dir, can be in supported protocols (lfn, srm, file, guid,..)
 * @param name: name of the attribute to get
 * @param value:  pointer to buffer to get the value
 * @param size : size of the buffer
 * @return  return the size of the data returned, or -1 if error. In this case,  errno  is  set
       and you can call @ref gfal_posix_check_error() for a more complete description. 
*/
ssize_t gfal_getxattr (const char *path, const char *name,
                        void *value, size_t size){
	return gfal_posix_internal_getxattr(path,name, value, size);
}


ssize_t gfal_readlink(const char* path, char* buff, size_t buffsiz){
	return gfal_posix_internal_readlink(path, buff, buffsiz);
}

/**
 *  @brief delete a name and possibly the file it refers to
 *
 *  similar to the POSIX call unlink
 *  gfal_unlink() deletes a name from the file system.  If that name was the last link to a file and no processes have the file open the file is deleted and the space it was using is made avail‐
 *      able for reuse.
 *
 *      If the name was the last link to a file but any processes still have the file open the file will remain in existence until the last file descriptor referring to it is closed.
 *
 *      If the name referred to a symbolic link the link is removed.
 *
 *      If the name referred to a socket, fifo or device the name for it is removed but processes which have the object open may continue to use it.
 * @return On success, zero is returned.  On error, -1 is returned, and errno is set appropriately and you can call @ref gfal_posix_check_error() for a more complete description. 
 * 
 * */
int gfal_unlink(const char* path){
	return gfal_posix_internal_unlink(path);
}

/**
 * @brief  list all extended attributes
 * 
 * similar to listxattr standard call of libattr
 * gfal_listxattr  list all extended atributes associated with a file
 * The extended attributes are use for the advanced file operations ( like set/get replicas, grid status, comments, etc... )
 * 
 * @param path : path of the file/dir, can be in a supported protocol (lfn, srm, file, guid,..)
 * @param list: a list of the attribute in a string format, on after each other, separated by '\\0'. 
 * @param size : size of the buffer
 * @return  return the size of the data in list , or -1 if error. In this case,  errno  is  set
 *     and you can call @ref gfal_posix_check_error() for a more complete description. 
*/
ssize_t gfal_listxattr (const char *path, char *list, size_t size){
	return gfal_posix_internal_listxattr(path, list, size);
}

/**
 * @brief set an extended attribute to a given value
 *  similar to the setxattr standard call of libattr
 * 
 *  the effect of this call can be specific to the plugin used. ( ex : guid are read only)
 *  
 *  @param path : path of the file
 *  @param name : key of the extended atribute to set
 *  @param value : value to set, must be at least of the size size
 *  @param size : size of the attriute to set
 *  @param flags : flags similar to the setxattr call, can be ignored by some plugins/call
 *  @return 0 if success else or -1 if error. In this case,  errno  is  set
 *      and you can call @ref gfal_posix_check_error() for a more complete description. 
 * */
int gfal_setxattr (const char *path, const char *name,
			   const void *value, size_t size, int flags){
	return gfal_posix_internal_setxattr(path, name, value, size, flags);
}

/**
 * @brief removes the extended attribute identified by name and associated with the given path in the filesystem
 *  similar to the removexattr standard call of libattr
 * 
 *  the effect of this call can be specific to the plugin used. ( ex : guid are read only)
 *  
 *  @param path : path of the file
 *  @param name : key of the extended to remove
 *  @return 0 if success or -1 if error. In this case,  errno  is  set
 *      and you can call @ref gfal_posix_check_error() for a more complete description. 
 * */
int gfal_removexattr(const char *path, const char *name){
	return -1;
}


/**
 * print the last string error reported by the gfal error system for the posix API but DO NOT delete it
 * Errors are printed on stderr
 */
void gfal_posix_print_error(){
	gfal_handle handle;
	GError* err=NULL;
	if((handle = gfal_posix_instance()) == NULL){
		g_printerr("[gfal] Initialisation error gfal_posix_instance() failure\n");
	}else if ( (err = *gfal_posix_get_last_error()) != NULL){
		g_printerr("[gfal]%s \n", err->message);
	}else if(errno !=0){
		char* sterr = strerror(errno);
		g_printerr("[gfal] errno reported by external lib : %s", sterr);
	}else{
		g_printerr("[gfal] No gfal error reported\n");
	}
}

/***
 *  commit all operations associated with a given file descriptor.
 *  @warning return always true in the current state.
 * */
int gfal_flush(int fd){
	return 0;
}

/**
 * Display and clear the last string error reported by the gfal error system for the posix API  
 * equivalent to a gfal_posix_print_error() and a gfal_posix_clear_error() call
 */
void gfal_posix_release_error(){
	gfal_posix_print_error();
	gfal_posix_clear_error();
}

/***
 * @brief set a parameter of configuration in gfal
 * 
 *  set a parameter identified by name in the module module with a specific string value
 *  
 * @param namespace : namespace of the parameter ( plugin name, isifce, NULL for core), etc....
 * @param key : key of the parameter to set
 * @param value : value of the parameter
 * @return 0 if success or -1 if error. In this case,  errno  is  set
 *      and you can call @ref gfal_posix_check_error() for a more complete description. 
 * */
int gfal_set_parameter_string(const char* namespace, const char* key, const char* value){
	return gfal_set_parameter_string_internal(namespace, key, value);
}


/***
 * @brief get a parameter of configuration in gfal
 * 
 *  get a string parameter identified by name in the module module 
 *  
 * @param namespace : namespace of the parameter ( plugin name, isifce, NULL for core), etc....
 * @param key : key of the parameter to get
 * @return string  of the parameter if valid ( must be free), or NULL if not valid
 *      and you can call @ref gfal_posix_check_error() for a more complete description. 
 * */
char* gfal_get_parameter_string(const char* namespace, const char* key){
	return gfal_get_parameter_string_internal(namespace, key);	
}



/***
 * @brief set a parameter of configuration in gfal
 * 
 *  set a parameter identified by name in the module module with a specific int value 
 *  
 * @param module : namespace of the parameter ( plugin name, isifce, NULL for core), etc....
 * @param name : key of the parameter to set
 * @param value : int value to set
 * @return 0 if success or -1 if error. In this case,  errno  is  set
 *      and you can call @ref gfal_posix_check_error() for a more complete description. 
 * */
int gfal_set_parameter_int(const char* module, const char* name, int value){
  return -1;
}



/***
 * @brief sget a parameter of configuration in gfal
 * 
 *  get a parameter identified by name in the module module 
 *  
 * @param module : namespace of the parameter ( plugin name, isifce, NULL for core), etc....
 * @param name : key of the parameter to get
 * @return value if success or -1 if error. In this case,  errno  is  set
 *      and you can call @ref gfal_posix_check_error() for a more complete description. 
 * */
int gfal_get_parameter_int(const char* module, const char* name){
  return -1;	 
}

/***
 * @brief set a boolean parameter in gfal
 * 
 *  set a parameter identified by name in the module module with a specific int value 
 *  
 * @param module : namespace of the parameter ( plugin name, isifce, NULL for core), etc....
 * @param name : key of the parameter to set
 * @param value : int value to set
 * @return 0 if success or -1 if error. In this case,  errno  is  set
 *      and you can call @ref gfal_posix_check_error() for a more complete description. 
 * */
int gfal_set_parameter_boolean(const char* namespace, const char* key, int value){
  return gfal_set_parameter_boolean_internal(namespace, key, value);
}

/***
 * @brief set a parameter of configuration in gfal
 * 
 *  set a parameter identified by name in the module module with a specific int value 
 *  store a -1 value in this way is discouraged
 *  
 * @param module : namespace of the parameter ( plugin name, isifce, NULL for core), etc....
 * @param name : key of the parameter to set
 * @return return the value 0 if False, 1 if true or -1 if error occured
 *      and you can call @ref gfal_posix_check_error() for a more complete description. 
 * */
int gfal_get_parameter_boolean(const char* namespace, const char* key){
  return gfal_get_parameter_boolean_internal(namespace, key);
}


/**
 * clear the last error reported by a gfal posix function 
 * 
 */
void gfal_posix_clear_error(){
	g_clear_error( gfal_posix_get_last_error());
	errno =0;	
}

/**
 *  return the last error code ( ERRNO-style )
 *  most of the error code are ERRNO codes.
 *  @return last error code reported or 0 if nothing.
 * 
 * */
int gfal_posix_code_error(){
	GError* err=NULL;
	const int ret = ((err = *gfal_posix_get_last_error()) != NULL)? err->code :0 ;
	return ret;
}

/**
 *  check the  last Error, if no error report return 0 else return 1 and print the error on stderr
 *  @warning this does not clear the error
 * */
int gfal_posix_check_error(){
	GError* err=NULL;
	if((err = *gfal_posix_get_last_error()) != NULL) {
		g_printerr("[gfal]%s\n", err->message);
		return 1;
	}
	return 0;
}

/**
 * Get the last Error in a string format
 * @return return a pointer to the string buffer passed.
 */
char* gfal_posix_strerror_r(char* buff_err, size_t s_err){
	return (char*)gfal_str_GError_r(gfal_posix_get_last_error(), buff_err, s_err);
 }
 
/**
 * pipelined call for read call, support parallels access
 *  similar to system call pread
 * @param fd : file descriptor
 * @param buffer : buffer with the data
 * @param s_buff : maximum size of the buffer
 * @param offset : offset in bytes
 * @return return the number of bytes read, 0 means end of the file, 
 * and you can call @ref gfal_posix_check_error() for a more complete description
 * 
 * */
ssize_t gfal_pread(int fd, void * buffer, size_t count, off_t offset){
	return gfal_posix_internal_pread(fd, buffer, count, offset);
}


/**
 * pipelined write for write call, support parallels access
 *  similar to system call pwrite
 * @param fd : file descriptor
 * @param buffer : buffer with the data
 * @param s_buff : maximum size of the buffer
 * @param offset : offset in bytes
 * @return return the number of bytes write, 0 means end of the file, 
 * and you can call @ref gfal_posix_check_error() for a more complete description
 * 
 * */
ssize_t gfal_pwrite(int fd, const void * buffer, size_t count, off_t offset){
	return gfal_posix_internal_pwrite(fd, buffer, count, offset);
}
 
