/*
 * Copyright (c) CERN 2013-2015
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
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

#include <regex.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef __APPLE__
#include <sys/xattr.h>
#else
#include <attr/xattr.h>
#endif
#include <zlib.h>

#include <gfal_plugins_api.h>
#include <checksums/checksums.h>

const char* file_prefix="file:";
unsigned int s_prefix = 0;


typedef struct _chksum_interface{
    // init checksum handle
    void*  (*init)(void);
    // compute checksum chunk
    ssize_t (*update)(void* chk_handler, const char* buffer, size_t s_size);
    // return checksum result : > 0 -> success, -1 : buffer to short
    int (*getResult)(void* chk_handler, char* buffer, size_t s_b);
} Chksum_interface;


// LFC plugin GQuark
GQuark gfal2_get_plugin_file_quark(){
    return g_quark_from_static_string(GFAL2_QUARK_PLUGINS "::FILE");
}


static unsigned int file_prefix_len(){
    if(!s_prefix)
        g_atomic_int_set(&s_prefix, strlen(file_prefix));
    return s_prefix;
}

/*
 * srm plugin id
 */
const char* gfal_file_plugin_getName(){
	return GFAL2_PLUGIN_VERSIONED("file", VERSION);
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
        case GFAL_PLUGIN_CHECKSUM:
        case GFAL_PLUGIN_READLINK:
            return (gfal_lfile_path_checker(handle, url)==0);
		default:
			return FALSE;
	}
}



void gfal_plugin_file_report_error(const char* funcname, GError** err){
    gfal2_set_error(err, gfal2_get_plugin_file_quark(), errno,
            funcname, "errno reported by local system call %s", strerror(errno));
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
#ifdef __APPLE__
    const ssize_t res = getxattr(path + file_prefix_len(), name, buff, s_buff, 0, XATTR_NOFOLLOW);
#else
    const ssize_t res = getxattr(path + file_prefix_len(), name, buff, s_buff);
#endif
    if(res <0){
        gfal_plugin_file_report_error(__func__, err);
    }else
        errno =0;
    return res;
}

ssize_t gfal_plugin_file_listxattr(plugin_handle plugin_data, const char* path, char* list, size_t s_list, GError** err){
#ifdef __APPLE__
    const ssize_t res = listxattr(path + file_prefix_len(), list, s_list, XATTR_NOFOLLOW);
#else
    const ssize_t res = listxattr(path + file_prefix_len(), list, s_list);
#endif
    if(res <0){
        gfal_plugin_file_report_error(__func__, err);
    }else
        errno =0;
    return res;
}

int gfal_plugin_file_setxattr(plugin_handle plugin_data, const char* path, const char* name, const void* value, size_t size, int flags, GError** err){
#ifdef __APPLE__
    const int res = setxattr(path + file_prefix_len(), name, value, size, 0, flags);
#else
    const int res = setxattr(path + file_prefix_len(), name, value, size, flags);
#endif
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
        resu = gfal_file_handle_new2(gfal_file_plugin_getName(), (gpointer) ret, NULL, path);
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
    if(ret < 0){
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


// checksum wrapper

static void* adler_init(){
    unsigned long* lp =  malloc(sizeof(unsigned long));
    *lp = adler32(0L, Z_NULL, 0);
    return (void*) lp;
}

static ssize_t adler32_update(void* chk_handler, const char* buffer, size_t s){
    unsigned long* lp = (unsigned long*) chk_handler;
    *lp = adler32(*lp, (const Bytef *) buffer, (uInt) s);
    return (ssize_t)  s;
}

static int adler32_getResult(void* chk_handler, char* resu, size_t s_b){
    unsigned long* lp = (unsigned long*) chk_handler;
    snprintf(resu, s_b, "%lx", *lp);
    free(lp);
    return 0;
}

static void* crc32_init(){
    unsigned long* lp =  malloc(sizeof(unsigned long));
    *lp = crc32(0L, Z_NULL, 0);
    return (void*) lp;
}

static ssize_t crc32_update(void* chk_handler, const char* buffer, size_t s){
    unsigned long* lp = (unsigned long*) chk_handler;
    *lp = crc32(*lp, (const Bytef *)  buffer, s);
    return (ssize_t) s;
}

static int crc32_getResult(void* chk_handler, char* resu, size_t s_b){
    unsigned long* lp = (unsigned long*) chk_handler;
    snprintf(resu, s_b, "%ld", *lp);
    free(lp);
    return 0;
}


static void* md5_init(){
    GFAL_MD5_CTX* ctx  =  (GFAL_MD5_CTX*) malloc(sizeof(GFAL_MD5_CTX));
    gfal2_md5_init(ctx);
    return (void*) ctx;
}

static ssize_t md5_update(void* chk_handler, const char* buffer, size_t s){
    GFAL_MD5_CTX* ctx  =  (GFAL_MD5_CTX*) chk_handler;
    gfal2_md5_update(ctx, buffer, (unsigned long)s);
    return (ssize_t) s;
}

static int md5_getResult(void* chk_handler, char* resu, size_t s_b){
    GFAL_MD5_CTX* ctx  =  (GFAL_MD5_CTX*) chk_handler;
    unsigned char buffer[16];
    if( s_b < 33) // buffer to short
        return -1;
    gfal2_md5_final(buffer, ctx);
    gfal2_md5_to_hex_string((char*) buffer, resu);
    free(ctx);
    return 0;
}


// checksum implem

static int gfal_plugin_file_chk_compute(plugin_handle data, const char* url, const char* check_type,
                                         char * checksum_buffer, size_t buffer_length,
                                         off_t start_offset, size_t data_length,
                                         Chksum_interface* i_chk,
                                         GError ** err){
    GError* tmp_err=NULL;
    const ssize_t chunk_size = 2 << 20;
    gfal2_context_t handle = (gfal2_context_t) data;
    int fd;
    ssize_t ret = 0, remain_bytes= ((data_length>0)?(data_length):(chunk_size));

    if( (fd = gfal2_open(handle, url, O_RDONLY,  &tmp_err)) <0 ){
        g_prefix_error(err, "Error during checksum calculation, open ");
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }

    if( gfal2_lseek(handle, fd, start_offset, SEEK_SET, &tmp_err) < 0){
        g_prefix_error(err, "Error during checksum calculation, lseek ");
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }

    void* c_handle= i_chk->init();
    char *buffer = malloc(chunk_size);
    do{
        ret = gfal2_read(handle, fd, buffer, MIN(chunk_size, remain_bytes),  &tmp_err);
        if (data_length > 0 ){
            remain_bytes -= ret;
        }
        if( ret > 0){
            i_chk->update(c_handle, buffer, ret);
        }
    }while(ret > 0 && remain_bytes > 0);
    free(buffer);
    gfal2_close(handle, fd, NULL);

    if( i_chk->getResult(c_handle, checksum_buffer, buffer_length) < 0){
        gfal2_set_error(err, gfal2_get_plugin_file_quark(), ENOBUFS, __func__, "buffer for checksum too short");
        return -1;
    }


    if( ret < 0){
        gfal2_set_error(err, gfal2_get_plugin_file_quark(), tmp_err->code, __func__,
                "Error during checksum calculation, read: %s", tmp_err->message);
        g_error_free(tmp_err);
        return -1;
    }
    return 0;
}



int gfal_plugin_filechecksum_calc(plugin_handle data, const char* url, const char* check_type,
                       char * checksum_buffer, size_t buffer_length,
                       off_t start_offset, size_t data_length,
                       GError ** err){
    if(strcasecmp(check_type, "adler32") ==0){
        Chksum_interface ie= { .init = &adler_init,
                               .update = &adler32_update,
                               .getResult = &adler32_getResult};

        return gfal_plugin_file_chk_compute(data, url, check_type, checksum_buffer,
                                            buffer_length, start_offset, data_length,
                                            &ie,
                                            err);
    } else if( strcasecmp(check_type, "crc32") == 0){
        Chksum_interface ie= { .init = &crc32_init,
                               .update = &crc32_update,
                               .getResult = &crc32_getResult};
        return gfal_plugin_file_chk_compute(data, url, check_type, checksum_buffer,
                                            buffer_length, start_offset, data_length,
                                            &ie,
                                            err);
    }else if( strcasecmp(check_type, "md5") == 0){
        Chksum_interface ie= { .init = &md5_init,
                               .update = &md5_update,
                               .getResult = &md5_getResult};
        return gfal_plugin_file_chk_compute(data, url, check_type, checksum_buffer,
                                            buffer_length, start_offset, data_length,
                                            &ie,
                                            err);
    }
    gfal2_set_error(err, gfal2_get_plugin_file_quark(), ENOSYS, __func__,
            "Checksum type %s not supported for local files", check_type);
    return -1;
}


/*
 * Init function, called before all
 * */
gfal_plugin_interface gfal_plugin_init(gfal2_context_t handle, GError** err){
	gfal_plugin_interface file_plugin;
    memset(&file_plugin,0,sizeof(gfal_plugin_interface));	// clear the plugin

    file_plugin.plugin_data = handle;
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
    file_plugin.readlinkG = &gfal_plugin_file_readlink;


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
    file_plugin.setxattrG  = &gfal_plugin_file_setxattr;
    file_plugin.checksum_calcG = &gfal_plugin_filechecksum_calc;

	return file_plugin;
}
