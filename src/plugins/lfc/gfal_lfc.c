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
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <attr/xattr.h>

#include "gfal_lfc.h"
#include "gfal_lfc_open.h"
#include "lfc_ifce_ng.h"


static gboolean init_thread = FALSE;
pthread_mutex_t m_lfcinit=PTHREAD_MUTEX_INITIALIZER;

typedef struct _lfc_opendir_handle {
    char url[GFAL_URL_MAX_LEN];
    struct dirent current_dir;
}*lfc_opendir_handle;

static char* file_xattr[] = {
    GFAL_XATTR_GUID, GFAL_XATTR_REPLICA, GFAL_XATTR_COMMENT,
    NULL
};

/*
 * just return the name of the layer
 */
const char* lfc_getName(){
	return GFAL2_PLUGIN_VERSIONED("lfc", VERSION);
}


// LFC plugin GQuark
GQuark gfal2_get_plugin_lfc_quark(){
    return g_quark_from_static_string(GFAL2_QUARK_PLUGINS "::LFC");
}

/*
 * convert the lfn url for internal usage
 * result must be free
 */
static inline char* lfc_urlconverter(const char * lfn_url, const char* prefix)
{
    const int pref_len = strlen(prefix);
    const int strsize = strnlen(lfn_url, GFAL_URL_MAX_LEN - 1);
    const int res_len = strsize - pref_len;
    char* p, *pdest, *porg;
    p = pdest = g_malloc(sizeof(char) * (res_len + 1));
    porg = (char*) lfn_url + pref_len;
    while ((pdest - p) < res_len && (porg - lfn_url) < strsize) { // remove double sep, remove end sep
        if ((*porg == '/' && *(porg + 1) == '/') == FALSE
                && (*porg == '/' && *(porg + 1) == '\0') == FALSE) {
            *pdest = *porg;
            ++pdest;
        }
        ++porg;
    }
    *(pdest) = '\0';
    return p;
}

/*
 * convert the lfc type url "lfc://" to an url
 * result must be free
 */
static inline int lfc_full_urlconverter(const char * lfn_url, char** host,
    char** path, GError** err)
{
    GError* tmp_err = NULL;

    int res = -1;
    const int pref_len = strlen(GFAL_LFC_PREFIX2);
    const int strsize = strnlen(lfn_url, GFAL_URL_MAX_LEN - 1);
    const int res_len = strsize - pref_len;
    char *p_org, *p_end, *p;
    p = (char*) lfn_url + pref_len;
    p_end = (char*) lfn_url + strsize;
    if (res_len > 0) {
        while (p < p_end && *p == '/')
            ++p;
        p_org = p;
        while (p < p_end && *p != '/')
            ++p;
        if (p_org < p && p < p_end) {
            if (host)
                *host = g_strndup(p_org, p - p_org);
            if (path)
                *path = g_strndup(p, p_end - p);
            res = 0;
        }

    }
    if (res != 0) {
        gfal2_set_error(&tmp_err, gfal2_get_plugin_lfc_quark(), EINVAL, __func__,
                "Invalid lfc:// url");
    }
    return res;
}

/// manage convertion for all lfc url type : lfc://, lfn://, guid:
/// return 0 if success, or -1 if bad url
int url_converter(plugin_handle handle, const char * url, char** host,
    char** path, GError** err)
{
    GError* tmp_err = NULL;
    int res = -1;
    if (strnlen(url, 5) != 5) { // bad string size, return empty string
        gfal2_log(G_LOG_LEVEL_WARNING, "lfc url converter -> bad url size");
        return res;
    }
    if (strncmp(url, "lfn", 3) == 0) {
        if (path)
            *path = lfc_urlconverter(url, GFAL_LFC_PREFIX);
        if (host)
            *host = g_strdup(lfc_plugin_get_lfc_env((struct lfc_ops*)handle, "LFC_HOST"));
        res = 0;
    }
    else if (strncmp(url, "lfc", 3) == 0) {
        res = lfc_full_urlconverter(url, host, path, &tmp_err);
    }
    else {
        char buff_lfn[GFAL_URL_MAX_LEN];
        res = gfal_convert_guid_to_lfn_r(handle, url + GFAL_LFC_GUID_PREFIX_LEN,
                buff_lfn, GFAL_URL_MAX_LEN, &tmp_err);
        if (path)
            *path = g_strdup(buff_lfn);
    }
    G_RETURN_ERR(res, tmp_err, err);
}



/*
 *  Deleter to unload the lfc part
 * */
static void lfc_destroyG(plugin_handle handle){
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	if(ops){
		gsimplecache_delete(ops->cache_stat);
		regfree(&(ops->rex));
		free(ops);
	}
}

/*
 * Implementation of the chmod function with the LFC plugin
 * return 0 or the errno if error, or set GError if serious error
 */
int lfc_chmodG(plugin_handle handle, const char* path, mode_t mode, GError** err){
	g_return_val_err_if_fail(handle && path, -1, err, "[lfc_chmodG] Invalid valid value in handle/path ");
	GError* tmp_err=NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;
    int  ret=-1;
    char *url_path=NULL, *url_host=NULL;
    if( (ret = url_converter(handle, path, &url_host, &url_path, &tmp_err)) ==0){
        ret= lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            gfal_lfc_init_thread(ops);
            gfal_auto_maintain_session(ops, &tmp_err);
            ret = ops->chmod(url_path, mode);
            if(ret < 0){
                const int myerrno = gfal_lfc_get_errno(ops);
                gfal2_set_error(&tmp_err, gfal2_get_plugin_lfc_quark(), myerrno, __func__,
                        "Errno reported from lfc : %s ", gfal_lfc_get_strerror(ops));
            }else{
                errno =0;
                gsimplecache_remove_kstr(ops->cache_stat, url_path);
            }
        }
    }
    g_free(url_path);
    g_free(url_host);
    G_RETURN_ERR(ret, tmp_err, err);
}

/*
 *
 * implementation of the access call with the lfc plugin
 *  return 0 or -1 if error and report GError** with error code and message
 */
int lfc_accessG(plugin_handle handle, const char* lfn, int mode, GError** err){
	g_return_val_err_if_fail(handle && lfn, -1, err, "[lfc_accessG] Invalid value in arguments handle  or/and path");
	GError* tmp_err=NULL;
    int ret =-1;
	struct lfc_ops* ops = (struct lfc_ops*) handle;
    char *url_path=NULL, *url_host=NULL;

    if( (ret = url_converter(handle, lfn, &url_host, &url_path, &tmp_err)) ==0){
        ret= lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            gfal_lfc_init_thread(ops);
            gfal_auto_maintain_session(ops, &tmp_err);
            ret = ops->access(url_path, mode);
            if(ret <0){
                int sav_errno = gfal_lfc_get_errno(ops);
                gfal2_set_error(&tmp_err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
                        "lfc access error, file : %s, error : %s", lfn, gfal_lfc_get_strerror(ops) );
            }else
                errno=0;
        }
    }
    g_free(url_path);
    g_free(url_host);
    G_RETURN_ERR(ret, tmp_err, err);
}



/*
 * Implementation of the rename call for the lfc plugin
 * return 0 if success else -1 if error and set GError
 *
 * */
int lfc_renameG(plugin_handle handle, const char* oldpath, const char* newpath, GError** err){
	g_return_val_err_if_fail(handle && oldpath && newpath, -1, err, "[lfc_renameG] Invalid value in args handle/oldpath/newpath");
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	GError* tmp_err=NULL;
    int ret =-1;

    char *source_url_path=NULL, *source_url_host=NULL;
    char *dest_url_path=NULL, *dest_url_host=NULL;

    if( (ret = url_converter(handle, oldpath, &source_url_host, &source_url_path, &tmp_err)) ==0
        && (ret = url_converter(handle, newpath, &dest_url_host, &dest_url_path, &tmp_err)) ==0){
        ret= lfc_configure_environment(ops, source_url_host, &tmp_err);
        if(!tmp_err){
            gfal_lfc_init_thread(ops);
            gfal_auto_maintain_session(ops, &tmp_err);
            ret  = ops->rename(source_url_path, dest_url_path);
            if(ret <0){
                int sav_errno = gfal_lfc_get_errno(ops);
                gfal2_set_error(&tmp_err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
                        "Error report from LFC : %s",  gfal_lfc_get_strerror(ops) );
            }else{
                gsimplecache_remove_kstr(ops->cache_stat, source_url_path);
            }
        }
    }
    g_free(source_url_path);
    g_free(source_url_host);
    g_free(dest_url_path);
    g_free(dest_url_host);
    G_RETURN_ERR(ret, tmp_err, err);
}


/*
 * Implementation of the symlinkG call for the lfc plugin
 * return 0 if success else -1 if error and set GError
 *
 * */
int lfc_symlinkG(plugin_handle handle, const char* oldpath, const char* newpath, GError** err){
	g_return_val_err_if_fail(handle && oldpath && newpath, -1, err, "[lfc_symlinkG] Invalid value in args handle/oldpath/newpath");
    struct lfc_ops* ops = (struct lfc_ops*) handle;
    GError* tmp_err=NULL;
    int ret =-1;

    char *url_path=NULL, *url_host=NULL;
    char *link_url_path=NULL, *link_url_host=NULL;

    if( (ret = url_converter(handle, oldpath, &url_host, &url_path, &tmp_err)) ==0
        && (ret = url_converter(handle, newpath, &link_url_host, &link_url_path, &tmp_err)) ==0 ){
        ret= lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            gfal_lfc_init_thread(ops);
            gfal_auto_maintain_session(ops, &tmp_err);
            ret  = ops->symlink(url_path, link_url_path);
            if(ret <0){
                int sav_errno = gfal_lfc_get_errno(ops);
                gfal2_set_error(&tmp_err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
                        "Error report from LFC : %s",  gfal_lfc_get_strerror(ops) );
            }
        }
    }
    g_free(url_path);
    g_free(url_host);
    G_RETURN_ERR(ret, tmp_err, err);
}

/*
 * execute a posix stat request on the lfc
 * return 0 and set struct if correct answer, else return negative value and set GError
 *
 */
int lfc_statG(plugin_handle handle, const char* path, struct stat* st, GError** err){
	g_return_val_err_if_fail(handle && path && st, -1, err, "[lfc_statG] Invalid value in args handle/path/stat");
	GError* tmp_err=NULL;
	int ret=-1;
	struct lfc_ops* ops = (struct lfc_ops*) handle;
    char *url_path=NULL, *url_host=NULL;

    if( (ret = url_converter(handle, path, &url_host, &url_path, &tmp_err)) ==0){
        ret= lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            gfal_lfc_init_thread(ops);
            gfal_auto_maintain_session(ops, &tmp_err);
            struct lfc_filestatg statbuf;
            ret = gfal_lfc_statg(ops, url_path, &statbuf, &tmp_err);
            if(ret == 0){
                ret= gfal_lfc_convert_statg(st, &statbuf, err);
                errno=0;
            }
        }
    }
    g_free(url_path);
    g_free(url_host);
    G_RETURN_ERR(ret, tmp_err, err);
}
/*
 * execute a posix lstat request on the lfc ( stat request with link information)
 *  return 0 if success and set the struct buf else return negative value and set GError
 */
static int lfc_lstatG(plugin_handle handle, const char* path, struct stat* st, GError** err){
	g_return_val_err_if_fail(handle && path && st, -1, err, "[lfc_lstatG] Invalid value in args handle/path/stat");
	GError* tmp_err=NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	int ret=-1;
    char *url_path=NULL, *url_host=NULL;

    if( (ret = url_converter(handle, path, &url_host, &url_path, &tmp_err)) ==0){
        ret= lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            struct lfc_filestat statbuf;

            if( ( ret= gsimplecache_take_one_kstr(ops->cache_stat, url_path, st)) == 0){ // take the version of the buffer
                gfal2_log(G_LOG_LEVEL_DEBUG, " lfc_lstatG -> value taken from cache");
            }else{
                gfal2_log(G_LOG_LEVEL_DEBUG, " lfc_lstatG -> value not in cache, do normal call");
                gfal_lfc_init_thread(ops);
                gfal_auto_maintain_session(ops, &tmp_err);
                if(!tmp_err){
                    ret = ops->lstat(url_path, &statbuf);
                    if(ret != 0){
                        int sav_errno = gfal_lfc_get_errno(ops);
                        gfal2_set_error(&tmp_err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
                                "Error report from LFC : %s", gfal_lfc_get_strerror(ops));
                    }else{
                        ret= gfal_lfc_convert_lstat(st, &statbuf, err);
                        errno=0;
                    }
                }
            }
        }
    }
    g_free(url_path);
    g_free(url_host);
    G_RETURN_ERR(ret, tmp_err, err);
}

/*
 *  Execute a posix mkdir on the lfc
 *  return 0 on success else -1 and err is set with the correct value
 * */
 static int lfc_mkdirpG(plugin_handle handle, const char* path, mode_t mode, gboolean pflag, GError** err){
	g_return_val_err_if_fail(handle && path , -1, err, "[lfc_mkdirpG] Invalid value in args handle/path");
	GError* tmp_err = NULL;
	int ret= -1;
    struct lfc_ops* ops = (struct lfc_ops*) handle;
    char *url_path=NULL, *url_host=NULL;

    if( (ret = url_converter(handle, path, &url_host, &url_path, &tmp_err)) ==0){
        ret= lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            gfal_lfc_init_thread(ops);
            gfal_auto_maintain_session(ops, &tmp_err);
            ret =gfal_lfc_ifce_mkdirpG(ops, url_path, mode, pflag, &tmp_err);
        }
    }
    g_free(url_path);
    g_free(url_host);
    G_RETURN_ERR(ret, tmp_err, err);
 }

/*
 * Execute a rmdir on the lfc
 *  return 0 on success else -1 and err is set with the correct value
 * */
 static int lfc_rmdirG(plugin_handle handle, const char* path, GError** err){
	g_return_val_err_if_fail( handle && path , -1, err, "[lfc_rmdirG] Invalid value in args handle/path");
	GError* tmp_err=NULL;
	int ret = -1;
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	gfal_lfc_init_thread(ops);
    char *url_path=NULL, *url_host=NULL;

    if( (ret = url_converter(handle, path, &url_host, &url_path, &tmp_err)) ==0){
        ret= lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            ret = ops->rmdir(url_path);
            if( ret < 0){
                int sav_errno = gfal_lfc_get_errno(ops);
                sav_errno = (sav_errno==EEXIST)?ENOTEMPTY:sav_errno;		// convert wrong reponse code
                gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
                        "Error report from LFC %s", gfal_lfc_get_strerror(ops) );
            }
        }
    }
    g_free(url_path);
    g_free(url_host);
    G_RETURN_ERR(ret, tmp_err, err);
 }

/*
 * execute an opendir func to the lfc
 * */
static gfal_file_handle lfc_opendirG(plugin_handle handle, const char* path, GError** err){
    g_return_val_err_if_fail( handle && path , NULL, err, "[lfc_rmdirG] Invalid value in args handle/path");
	GError* tmp_err=NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	lfc_opendir_handle oh=NULL;
    DIR* d = NULL;
    char *url_path=NULL, *url_host=NULL;

    if( url_converter(handle, path, &url_host, &url_path, &tmp_err) ==0 ){
        lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            gfal_lfc_init_thread(ops);
            gfal_auto_maintain_session(ops, &tmp_err);

            d  = (DIR*) ops->opendirg(url_path,NULL);
            if(d==NULL){
                int sav_errno = gfal_lfc_get_errno(ops);
                gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
                        "Error report from LFC %s", gfal_lfc_get_strerror(ops) );
            }else{
                oh = g_new0(struct _lfc_opendir_handle,1);
                g_strlcpy(oh->url, url_path, GFAL_URL_MAX_LEN );
            }
        }
    }
    g_free(url_path);
    g_free(url_host);
    G_RETURN_ERR(((d)?(gfal_file_handle_new2(lfc_getName(), (gpointer) d, (gpointer) oh, path)):NULL), tmp_err, err);
}

static struct dirent* lfc_convert_dirent_struct(struct lfc_ops *ops , struct dirent* dir , struct stat* st,
                                                struct Cns_direnstat* filestat, const char* url){
    struct stat st2;
    if(st == NULL)
        st = &st2;
	if(filestat == NULL)
		return NULL;
	GSimpleCache* cache = ops->cache_stat;
	char fullurl[GFAL_URL_MAX_LEN];
	g_strlcpy(fullurl, url, GFAL_URL_MAX_LEN);
	g_strlcat(fullurl, "/", GFAL_URL_MAX_LEN);
	g_strlcat(fullurl, filestat->d_name, GFAL_URL_MAX_LEN);

    memset(st, 0, sizeof(struct stat));
    st->st_mode = (mode_t) filestat->filemode;
    st->st_nlink = (nlink_t) filestat->nlink;
    st->st_uid = (uid_t) filestat->uid;
    st->st_gid = (gid_t) filestat->gid;
    st->st_size = (off_t) filestat->filesize;
    st->st_atime = (time_t) filestat->atime;
    st->st_ctime = (time_t)filestat->ctime;
    st->st_mtime = (time_t) filestat->mtime;

    gsimplecache_add_item_kstr(cache, fullurl, (void*) st);
#if defined(SOLARIS) || defined(__linux__)
	dir->d_off +=1;
#endif
	g_strlcpy(dir->d_name, filestat->d_name, NAME_MAX);
	return dir;
}


/*
 * Execute a readdirpp func on the lfc
 * */
static struct dirent* lfc_readdirppG(plugin_handle handle, gfal_file_handle fh,
        struct stat* st, GError** err)
{
    g_return_val_err_if_fail(handle && fh, NULL, err, "[lfc_rmdirG] Invalid value in args handle/path");
    GError* tmp_err = NULL;
    int sav_errno = 0;
    struct lfc_ops *ops = (struct lfc_ops*) handle;

    gfal_lfc_init_thread(ops);
    gfal_auto_maintain_session(ops, &tmp_err);
    gfal_lfc_reset_errno(ops);

    lfc_opendir_handle oh = (lfc_opendir_handle) gfal_file_handle_get_user_data(fh);
    struct dirent* ret;
    lfc_DIR* lfc_dir = (lfc_DIR*) gfal_file_handle_get_fdesc(fh);
    ret = lfc_convert_dirent_struct(ops, ((struct dirent*) &oh->current_dir), st, ops->readdirx(lfc_dir), oh->url);
    if (ret == NULL && (sav_errno = gfal_lfc_get_errno(ops))) {
        gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
                "Error report from LFC %s", gfal_lfc_get_strerror(ops));
    }
    return ret;
}

/*
 * Execute a readdir func on the lfc
 * */
static struct dirent* lfc_readdirG(plugin_handle handle, gfal_file_handle fh, GError** err){
    return lfc_readdirppG(handle, fh, NULL, err);
}

/*
 * execute an closedir func on the lfc
 * */
static int lfc_closedirG(plugin_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail( handle && fh , -1, err, "[lfc_rmdirG] Invalid value in args handle/path");
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	gfal_lfc_init_thread(ops);
	int ret = ops->closedir(gfal_file_handle_get_fdesc(fh));
	if(ret != 0){
		int sav_errno = gfal_lfc_get_errno(ops);
        gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
                "Error report from LFC %s", gfal_lfc_get_strerror(ops));
	}else{
		g_free(gfal_file_handle_get_user_data(fh));
		gfal_file_handle_delete(fh);
	}
	return ret;
}

/*
 * resolve the lfc link to the surls
 */
char ** lfc_getSURLG(plugin_handle handle, const char * path, GError** err){
	g_return_val_err_if_fail( handle && path , NULL, err, "[lfc_getSURLG] Invalid value in args handle/path");
	GError* tmp_err=NULL;
	char** resu = NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	gfal_lfc_init_thread(ops);
    char *url_path=NULL, *url_host=NULL;

    if( (url_converter(handle, path, &url_host, &url_path, &tmp_err)) ==0){
        (void) lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            resu = gfal_lfc_getSURL(ops, url_path, &tmp_err);
        }
    }
    g_free(url_path);
    g_free(url_host);
    G_RETURN_ERR(resu, tmp_err, err);
}

/*
 * lfc getxattr for the path -> surls resolution
 * */
ssize_t lfc_getxattr_getsurl(plugin_handle handle, const char* path, void* buff, size_t size, GError** err){
	GError* tmp_err=NULL;
	ssize_t res = -1;

	char** tmp_ret = lfc_getSURLG(handle, path, &tmp_err);
	if(tmp_ret != NULL){
		res = g_strv_catbuff(tmp_ret, buff, size);
		g_strfreev(tmp_ret);
	}
    G_RETURN_ERR(res, tmp_err, err);
}


/*
 * lfc getxattr for the path -> guid resolution
 * */
ssize_t lfc_getxattr_getguid(plugin_handle handle, const char* path, void* buff, size_t size, GError** err){
	GError* tmp_err=NULL;
	ssize_t res = -1;
    struct lfc_ops* ops = (struct lfc_ops*) handle;
    char *url_path=NULL, *url_host=NULL;

    if( ( res = url_converter(handle, path, &url_host, &url_path, &tmp_err)) ==0){
        res= lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            if(size == 0 || buff ==NULL){ // just return the size of a guid
                res = sizeof(char) * 36; // strng uuid are 36 bytes long
            }else{
                struct lfc_filestatg statbuf;
                int tmp_ret = gfal_lfc_statg(ops, url_path, &statbuf, &tmp_err);
                if(tmp_ret == 0){
                    res = strnlen(statbuf.guid, GFAL_URL_MAX_LEN);
                    g_strlcpy(buff,statbuf.guid, size);
                    errno=0;
                }
            }
        }
    }
    g_free(url_path);
    g_free(url_host);
    G_RETURN_ERR(res, tmp_err, err);
}

/*
 * lfc getxattr for path -> comment resolution
 *
 * */
 ssize_t lfc_getxattr_comment(plugin_handle handle, const char* path, void* buff, size_t size, GError** err){
	GError* tmp_err=NULL;
	ssize_t res = -1;
	struct lfc_ops* ops = (struct lfc_ops*) handle;
    char *url_path=NULL, *url_host=NULL;

    if( ( res = url_converter(handle, path, &url_host, &url_path, &tmp_err)) ==0){
        res= lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            res = gfal_lfc_getComment(ops, url_path, buff, size, &tmp_err);
        }
    }
    g_free(url_path);
    g_free(url_host);
    G_RETURN_ERR(res, tmp_err, err);
 }

/*
 * lfc getxattr implem
 * */
ssize_t lfc_getxattrG(plugin_handle handle, const char* path, const char* name, void* buff, size_t size, GError** err){
	GError* tmp_err=NULL;
	ssize_t res = -1;
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	gfal_lfc_init_thread(ops);
	gfal_auto_maintain_session(ops, &tmp_err);
	if( strncmp(name, GFAL_XATTR_GUID, LFC_MAX_XATTR_LEN) == 0){
		res = lfc_getxattr_getguid(handle, path, buff, size, &tmp_err );
	}else if(strncmp(name, GFAL_XATTR_REPLICA, LFC_MAX_XATTR_LEN) == 0){
		res = lfc_getxattr_getsurl(handle, path, buff, size, &tmp_err);
	}else if(strncmp(name, GFAL_XATTR_COMMENT, LFC_MAX_XATTR_LEN) == 0){
		res= lfc_getxattr_comment(handle, path, buff, size, &tmp_err);
	}else{
        gfal2_set_error(&tmp_err, gfal2_get_plugin_lfc_quark(), ENOATTR, __func__, "axttr not found");
		res = -1;
	}
    G_RETURN_ERR(res, tmp_err, err);
}

/*
 * lfc getxattr implem
 * */
ssize_t lfc_listxattrG(plugin_handle handle, const char* path, char* list, size_t size, GError** err){
    ssize_t res = 0;
    char** p = file_xattr;
    char* plist = list;
    GError* tmp_err = NULL;

    struct stat st;
    if (lfc_lstatG(handle, path, &st, &tmp_err) < 0) {
        res = -1;
    }
    else {
        if (!S_ISDIR(st.st_mode)) {
            while (*p != NULL) {
                const int size_str = strlen(*p) + 1;
                if (size > res && size - res >= size_str)
                    plist = mempcpy(plist, *p, size_str * sizeof(char));
                res += size_str;
                p++;
            }
        }
        else {
            mempcpy(list, GFAL_XATTR_COMMENT, size);
            res = 1;
        }
    }
    G_RETURN_ERR(res, tmp_err, err);
}

/*
 * setxattr function special for comments
 * */
int lfc_setxattr_comment(plugin_handle handle, const char* path, const char* name,
							const void* value, size_t size, int flags, GError** err){
	GError* tmp_err=NULL;
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	int res = -1;
    char *url_path=NULL, *url_host=NULL;

    if( ( res = url_converter(handle, path, &url_host, &url_path, &tmp_err)) ==0){
        res= lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            res = gfal_lfc_setComment(ops, url_path, value, size, &tmp_err);
        }
    }
    g_free(url_path);
    g_free(url_host);
	return res;
}

/*
 * setxattr for replicas
 */
int lfc_setxattr_replica(plugin_handle handle, const char* path, const char* name,
                            const void* value, size_t size, int flags, GError** err){
    const char* sfn = (const char*)value;
    if (size == 0) {
        gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), EINVAL, __func__,
                "Missing value");
        return -1;
    }

    struct lfc_ops* ops = (struct lfc_ops*)handle;

    if (sfn[0] == '+') {
        int ret = -1;
        gfalt_params_t params = gfalt_params_handle_new(err);
        if (!*err) {
            ret = gfal_lfc_register(handle, ops->handle, params, sfn + 1, path, err);
            gfalt_params_handle_delete(params, err);
            if (*err) ret = -1;
        }
        return ret;
    }
    else if (sfn[0] == '-') {
        return gfal_lfc_unregister(handle, path, sfn + 1, err);
    }
    else {
        gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), EINVAL, __func__,
                "user.replica only accepts additions (+) or deletions (-)");
        return -1;
    }
}

/*
 * lfc setxattr implem
 * */
int lfc_setxattrG(plugin_handle handle, const char *path, const char *name,
                       const void *value, size_t size, int flags, GError** err){
    g_return_val_err_if_fail(path && name, -1, err, "invalid name/path");
    int res = -1;
    GError* tmp_err = NULL;

    if (strcmp(name, GFAL_XATTR_COMMENT) == 0) {
        res = lfc_setxattr_comment(handle, path, name, value, size, flags, err);
    }
    else if (strcmp(name, GFAL_XATTR_REPLICA) == 0) {
        res = lfc_setxattr_replica(handle, path, name, value, size, flags, err);
    }
    else {
        gfal2_set_error(&tmp_err, gfal2_get_plugin_lfc_quark(), ENOATTR, __func__,
                "unable to set this attribute on this file");
    }
    G_RETURN_ERR(res, tmp_err, err);
}

/*
 * Convert a guid to a plugin url if possible
 *  return the link in a plugin's url string or err and NULL if not found
 */
char* lfc_resolve_guid(plugin_handle handle, const char* guid, GError** err){
	g_return_val_err_if_fail( handle && guid, NULL, err, "[lfc_resolve_guid] Invalid args in handle and/or guid ");
    GError * tmp_err=NULL;
    char *url_path=NULL, *url_host=NULL, *res = NULL;
    struct lfc_ops* ops = (struct lfc_ops*) handle;

    if( url_converter(handle, guid, &url_host, &url_path, &tmp_err) ==0){
        lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            res = url_path;
        }
    }
    g_free(url_host);
    G_RETURN_ERR(res, tmp_err, err);
}

static int lfc_unlinkG(plugin_handle handle, const char* path, GError** err)
{
    g_return_val_err_if_fail(path, -1, err,
            "[lfc_unlink] Invalid value in args handle/path/stat");
    GError* tmp_err = NULL;
    struct lfc_ops* ops = (struct lfc_ops*) handle;
    char *url_path = NULL, *url_host = NULL;
    int ret = -1;

    if ((ret = url_converter(handle, path, &url_host, &url_path, &tmp_err)) == 0) {
        ret = lfc_configure_environment(ops, url_host, &tmp_err);
        if (!tmp_err) {
            int nreplies = 0;
            int *replies = NULL;
            ret = ops->delfilesbyname(1, (const char**)(&url_path), 1, &nreplies, &replies);
            if (ret != 0 || (nreplies && replies[0] != 0)) {
                int sav_errno = gfal_lfc_get_errno(ops);
                if (sav_errno != 0) {
                    gfal2_set_error(&tmp_err, gfal2_get_plugin_lfc_quark(),
                            sav_errno, __func__, "Error report from LFC : %s",
                            gfal_lfc_get_strerror(ops));
                }
                else {
                    gfal2_set_error(&tmp_err, gfal2_get_plugin_lfc_quark(),
                            replies[0], __func__, "Error report from LFC : %s",
                            ops->sstrerror(replies[0]));
                    ret = -1;
                }
            }
            else {
                gsimplecache_remove_kstr(ops->cache_stat, url_path); // remove the key associated in the buffer
                errno = 0;
            }
            free(replies);
        }

    }
    g_free(url_path);
    g_free(url_host);
    G_RETURN_ERR(ret, tmp_err, err);
}

/*
 * execute a posix readlink request on the lfc
 *  return size of the buffer if success and set the struct buf else return negative value and set GError
 */
static ssize_t lfc_readlinkG(plugin_handle handle, const char* path, char* buff, size_t buffsiz, GError** err){
	g_return_val_err_if_fail(handle && path && buff, -1, err, "[lfc_readlinkG] Invalid value in args handle/path/stat");
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	GError* tmp_err=NULL;
    ssize_t ret=-1;
	char res_buff[LFC_BUFF_SIZE];
	gfal_lfc_init_thread(ops);
	gfal_auto_maintain_session(ops, &tmp_err);

    char *url_path=NULL, *url_host=NULL;

    if( ( ret = url_converter(handle, path, &url_host, &url_path, &tmp_err)) ==0){
        ret= lfc_configure_environment(ops, url_host, &tmp_err);
        if(!tmp_err){
            ret = ops->readlink(url_path, res_buff, LFC_BUFF_SIZE );
            if(ret == -1){
                int sav_errno = gfal_lfc_get_errno(ops);
                gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
                        "Error report from LFC : %s", gfal_lfc_get_strerror(ops) );
            }else{
                errno=0;
                if(buffsiz > 0)
                    memcpy(buff, GFAL_LFC_PREFIX, MIN(buffsiz,GFAL_LFC_PREFIX_LEN) );
                if(buffsiz - GFAL_LFC_PREFIX_LEN > 0)
                    memcpy(buff+ GFAL_LFC_PREFIX_LEN, res_buff, MIN(ret,buffsiz-GFAL_LFC_PREFIX_LEN) );
                ret += GFAL_LFC_PREFIX_LEN;
            }
        }

    }
    g_free(url_path);
    g_free(url_host);
    G_RETURN_ERR(ret, tmp_err, err);
}


static void internal_stat_copy(gpointer original, gpointer copy){
	memcpy(copy, original, sizeof(struct stat));
}

/*
 * Map function for the lfc interface
 * this function provide the generic PLUGIN interface for the LFC plugin.
 * lfc_initG do : liblfc shared library load, sym resolve, endpoint check, and plugin function map.
 *
 * */
gfal_plugin_interface gfal_plugin_init(gfal2_context_t handle, GError** err){
	pthread_mutex_lock(&m_lfcinit);
	gfal_plugin_interface lfc_plugin;
	GError* tmp_err=NULL;
	memset(&lfc_plugin,0,sizeof(gfal_plugin_interface));	// clear the plugin

	struct lfc_ops* ops = gfal_load_lfc(GFAL_LFC_LIBRARY_NAME, &tmp_err); // load library
	if(ops ==NULL){
		gfal2_propagate_prefixed_error(err, tmp_err, __func__);
		pthread_mutex_unlock(&m_lfcinit);
		return lfc_plugin;
	}
    ops->lfc_endpoint_predefined = (char*) g_getenv(LFC_ENV_VAR_HOST);
    ops->lfc_conn_retry = (char*) g_getenv(LFC_ENV_VAR_CONRETRY) ;
    ops->lfc_conn_try_int = (char*) g_getenv(LFC_ENV_VAR_CONRETRYINT);
    ops->lfc_conn_timeout  =(char*)  g_getenv(LFC_ENV_VAR_CONNTIMEOUT);
	ops->handle = handle;

    lfc_configure_environment(ops, NULL, err);

    ops->cache_stat = gsimplecache_new(5000,&internal_stat_copy, sizeof(struct stat) );
	gfal_lfc_regex_compile(&(ops->rex), err);
	lfc_plugin.plugin_data = (void*) ops;
    lfc_plugin.priority = GFAL_PLUGIN_PRIORITY_CATALOG;
	lfc_plugin.check_plugin_url= &gfal_lfc_check_lfn_url;
	lfc_plugin.plugin_delete = &lfc_destroyG;
	lfc_plugin.accessG = &lfc_accessG;
	lfc_plugin.chmodG = &lfc_chmodG;
	lfc_plugin.renameG = &lfc_renameG;
	lfc_plugin.statG = &lfc_statG;
	lfc_plugin.lstatG = &lfc_lstatG;
	lfc_plugin.mkdirpG = &lfc_mkdirpG;
	lfc_plugin.rmdirG = &lfc_rmdirG;
	lfc_plugin.opendirG = &lfc_opendirG;
	lfc_plugin.closedirG = &lfc_closedirG;
	lfc_plugin.readdirG = &lfc_readdirG;
	lfc_plugin.getName = &lfc_getName;
	lfc_plugin.openG = &lfc_openG;
	lfc_plugin.symlinkG= &lfc_symlinkG;
	lfc_plugin.getxattrG= &lfc_getxattrG;
	lfc_plugin.setxattrG= &lfc_setxattrG;
	lfc_plugin.listxattrG = &lfc_listxattrG;
	lfc_plugin.readlinkG = &lfc_readlinkG;
	lfc_plugin.unlinkG = &lfc_unlinkG;
    lfc_plugin.readdirppG= &lfc_readdirppG;

    // Copy (as register)
    lfc_plugin.check_plugin_url_transfer = gfal_lfc_register_check;
    lfc_plugin.copy_file = gfal_lfc_register;


	if(init_thread== FALSE){ // initiate Cthread system
		ops->Cthread_init();	// must be called one time for DPM thread safety
		init_thread = TRUE;
	}
	gfal_lfc_init_thread(ops);
	pthread_mutex_unlock(&m_lfcinit);
	return lfc_plugin;
}


 /*
 * parse a guid to check the validity
 */
gboolean gfal_checker_guid(const char* guid, GError** err){
	g_return_val_err_if_fail(guid != NULL,FALSE,err,"[gfal_checker_guid] check URL failed : guid is empty");
	const size_t sguid = strnlen(guid, GFAL_URL_MAX_LEN);
	return ( sguid < GFAL_URL_MAX_LEN && sguid > 5 && strncmp(guid, "guid:",5)== 0);
}

/*
 * Check if the passed url and operation is compatible with lfc
 *
 * */
 gboolean gfal_lfc_check_lfn_url(plugin_handle handle, const char* url, plugin_mode mode, GError** err){
	struct lfc_ops* ops = (struct lfc_ops*) handle;
	int ret;
	switch(mode){
		case GFAL_PLUGIN_RESOLVE_GUID:
			return TRUE;

		case GFAL_PLUGIN_ACCESS:
		case GFAL_PLUGIN_CHMOD:
		case GFAL_PLUGIN_STAT:
		case GFAL_PLUGIN_LSTAT:
		case GFAL_PLUGIN_OPEN:
		case GFAL_PLUGIN_GETXATTR:
		case GFAL_PLUGIN_LISTXATTR:
		case GFAL_PLUGIN_SETXATTR:
		case GFAL_PLUGIN_UNLINK:
			ret= regexec(&(ops->rex), url, 0, NULL, 0);
			return (!ret || gfal_checker_guid(url, err ))?TRUE:FALSE;

		case GFAL_PLUGIN_RENAME:
		case GFAL_PLUGIN_MKDIR:
		case GFAL_PLUGIN_RMDIR:
		case GFAL_PLUGIN_OPENDIR:

		case GFAL_PLUGIN_SYMLINK:
		case GFAL_PLUGIN_READLINK:
			ret= regexec(&(ops->rex), url, 0, NULL, 0);
			return (!ret)?TRUE:FALSE;
		default:
			return FALSE;
	}
}


