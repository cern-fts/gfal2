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
* 
* lfc_ifce_ng.c
* main internal file of the lfc plugin module
* author : Adrien Devresse
 */

#define GFAL_LFN_MAX_LEN	2048

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <pthread.h>
#include <serrno.h>
#include <uuid/uuid.h>

#include <config/gfal_config.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>
#include <logger/gfal_logger.h>

#include <mds/gfal_mds.h>
#include <config/gfal_config.h>

#include "gfal_lfc.h"
#include "lfc_ifce_ng.h"


int _Cthread_addcid(char *, int, char *, int, Cth_pid_t *, unsigned, void *(*)(void *), int); // hack in order to use the internal CThread API ( DPNS limitation )

static __thread int _local_thread_init=FALSE;

static volatile time_t session_timestamp= 0;
static long session_duration = 20;
static pthread_mutex_t m_session = PTHREAD_MUTEX_INITIALIZER;

int gfal_lfc_regex_compile(regex_t* rex, GError** err){
    int ret = regcomp(rex, "^(lfn:/|lfc://)([:alnum:]|-|/|.|_)+", REG_ICASE | REG_EXTENDED);
	g_return_val_err_if_fail(ret ==0,-1,err,"[gfal_lfc_check_lfn_url] fail to compile regex, report this bug");
	return ret;
}

static void lfc_plugin_set_lfc_env(struct lfc_ops* ops, const char* var_name,
        const char* var_value)
{
    if (ops->set_env) { // if new lfc library with set_env call, set at runtime
        ops->set_env(var_name, var_value, TRUE);
    }
    else { // else set it as env var !! NOT THREAD SAFE
        g_setenv(var_name, var_value, TRUE);
    }
}


int lfc_configure_environment(struct lfc_ops * ops, const char* host, GError** err)
{
    GError * tmp_err=NULL;
    const char * tab_envar[] = { ops->lfc_endpoint_predefined, ops->lfc_conn_timeout,
                           ops->lfc_conn_retry, ops->lfc_conn_try_int};
    const char * tab_envar_name[] = { LFC_ENV_VAR_HOST , LFC_ENV_VAR_CONNTIMEOUT,
                           LFC_ENV_VAR_CONRETRY, LFC_ENV_VAR_CONRETRYINT};
    const int tab_type[] = { 0, 1,
                           1, 1};
    const char* tab_override[] = { host, NULL, NULL, NULL, NULL };
    const int n_var =4;
    const char * plugin_group = LFC_ENV_VAR_GROUP_PLUGIN;
    int i,ret;

    ret = 0;

    for(i = 0; i < n_var;++i){
        if(tab_envar[i] == NULL){
            switch(tab_type[i]){
                case 0:{
                    char* v1 = NULL;
                    char* value = NULL;
                    if(tab_override[i] != NULL){
                        value = (char*) tab_override[i];
                    }else{
                        value= v1 =gfal2_get_opt_string(ops->handle, plugin_group, tab_envar_name[i], &tmp_err);
                    }
                    if(!tmp_err){
                        gfal_log(GFAL_VERBOSE_TRACE, "lfc plugin : setup env var value %s to %s", tab_envar_name[i],value);
                        lfc_plugin_set_lfc_env(ops, tab_envar_name[i],value);
                        g_free(v1);
                    }else{
                        ret = -1;
                    }
                    break;
                }
                case 1:{
                    int v2;
                    v2 =gfal2_get_opt_integer(ops->handle, plugin_group, tab_envar_name[i], &tmp_err);
                    if(!tmp_err){
                        char v_str[20];
                        snprintf(v_str,20, "%d",v2);
                        gfal_log(GFAL_VERBOSE_TRACE, "lfc plugin : setup env var value %s to %d",tab_envar_name[i],v2);
                        lfc_plugin_set_lfc_env(ops, tab_envar_name[i],v_str);
                    }else{
                        ret = -1;
                    }
                    break;
                 }
                default:
                    ret =-1;
                    gfal2_set_error(&tmp_err, gfal2_get_plugin_lfc_quark(), EINVAL, __func__,
                            "Invalid value %s in configuration file ", tab_envar_name[i]);
            }
        }
        if(tmp_err)
            break;
    }

    // Set credentials
    gchar* ucert = gfal2_get_opt_string(ops->handle, "X509", "CERT", NULL);
    gchar* ukey = gfal2_get_opt_string(ops->handle, "X509", "KEY", NULL);
    if (ucert && ukey) {
        gfal_log(GFAL_VERBOSE_TRACE, "lfc plugin : using certificate %s", ucert);
        gfal_log(GFAL_VERBOSE_TRACE, "lfc plugin : using private key %s", ukey);
        lfc_plugin_set_lfc_env(ops, "X509_USER_CERT", ucert);
        lfc_plugin_set_lfc_env(ops, "X509_USER_KEY", ukey);
    }
    else if (ucert) {
        gfal_log(GFAL_VERBOSE_TRACE, "lfc plugin : using proxy %s", ucert);
        lfc_plugin_set_lfc_env(ops, "X509_USER_PROXY", ucert);
    }
    g_free(ucert);
    g_free(ukey);

    G_RETURN_ERR(ret, tmp_err, err);
}


// Routine for internal lfc hack, need to be call for the thread safety 
void gfal_lfc_init_thread(struct lfc_ops* ops){
	if(_local_thread_init==FALSE){
		Cth_pid_t th = pthread_self();
		ops->_Cthread_addcid (NULL, 0, NULL, 0, &th, 0, NULL, 0); 
		_local_thread_init= TRUE;
	}
}



// WARNING : ALL SESSIONS and transactions for LFC disabled
// REASONS : unstable, can just fail forever in case of connexion drop
int gfal_lfc_startSession(struct lfc_ops* ops, GError ** err){ 
    /*if (ops->startsess (ops->lfc_endpoint, "gfal 2.0 auto-session") < 0){
		int sav_errno = gfal_lfc_get_errno(ops);
        gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
            "Error while start session with lfc, lfc_endpoint: %s, Error : %s ",
             ops->lfc_endpoint, gfal_lfc_get_strerror(ops));
		return -1;
    }*/
    return 0;
}
/* --> session reused disabled -> bugged in liblfc
static int gfal_lfc_endSession(struct lfc_ops* ops, GError ** err){ 
	if (ops->endsess() < 0){
		int sav_errno = gfal_lfc_get_errno(ops);
        gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
            "Error while start transaction with lfc, Error : %s ", gfal_lfc_get_strerror(ops));
		return -1;
	}
	return 0;
}*/

// session re-use
void gfal_auto_maintain_session(struct lfc_ops* ops, GError ** err){
	/*time_t current = time(NULL); --> disabled, thread safety problem in liblfc inthe current state
	if(session_timestamp < current){
		pthread_mutex_lock(&m_session);
		if(session_timestamp < current){
			gfal_lfc_endSession(ops, NULL);
			gfal_lfc_startSession(ops, err);
			current = time(NULL);
			session_timestamp = current + session_duration;
		}
		pthread_mutex_unlock(&m_session);	
	}*/
}

void lfc_set_session_timeout(int timeout){
	pthread_mutex_lock(&m_session);	
	session_duration = timeout;
	pthread_mutex_unlock(&m_session);
}


static int gfal_lfc_startTransaction(struct lfc_ops* ops, GError ** err){ 
    /*if (ops->starttrans(ops->lfc_endpoint, "gfal 2.0 auto-trans")){
		int sav_errno = gfal_lfc_get_errno(ops);
        gfal2_set_error(err, gfal2_get_plugin_lfc_quark() ,sav_errno, __func__,
            "Error while start transaction with lfc, lfc_endpoint: %s, Error : %s ",
			ops->lfc_endpoint, gfal_lfc_get_strerror(ops));
		return -1;
    }*/
	return 0;
}

static int gfal_lfc_endTransaction(struct lfc_ops* ops, GError ** err){ 
	if (ops->endtrans() < 0){
		int sav_errno = gfal_lfc_get_errno(ops);
        gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), sav_errno,
                "Error while start transaction with lfc, Error : %s ", gfal_lfc_get_strerror(ops));
		return -1;
	}
	return 0;
}




static int gfal_lfc_abortTransaction(struct lfc_ops* ops, GError ** err){ 
	if (ops->aborttrans() < 0){
		int sav_errno = gfal_lfc_get_errno(ops);
        gfal2_set_error(err,gfal2_get_plugin_lfc_quark(),sav_errno,
                "Error while abort transaction with lfc,  Error : %s ", gfal_lfc_get_strerror(ops));
		return -1;
	}
	return 0;
}

 
 

int gfal_convert_guid_to_lfn_r(plugin_handle handle, const char* guid, char* buff_lfn, size_t sbuff_lfn, GError ** err){
	int ret;
	int size = 0;
	struct lfc_ops* ops = (struct lfc_ops*) handle;	
	gfal_lfc_init_thread(ops);
	struct lfc_linkinfo* links = NULL;
	if(ops->getlinks(NULL, guid, &size, &links) <0){
		int sav_errno = gfal_lfc_get_errno(ops);
        gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
                "Error while getlinks() with lfclib,  guid : %s, Error : %s ", guid, gfal_lfc_get_strerror(ops));
		ret = -1;
	}else{
		if(!links || strnlen(links[0].path, GFAL_LFN_MAX_LEN) >= GFAL_LFN_MAX_LEN){
            gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), EINVAL, __func__,
                    "Error no links associated with this guid or corrupted one : %s", guid);
			ret = -1;
		}else{
			g_strlcpy(buff_lfn, links[0].path, sbuff_lfn);
			ret =0;
		}
	}
	free(links);
	return ret;
 }
 

/*
 * load the shared library and link the symbol for the LFC usage
 * @param name : name of the library
 * @param err:  error report
*/
struct lfc_ops* gfal_load_lfc(const char* name, GError** err){
	struct lfc_ops* lfc_sym=NULL;
	GError * tmp_err=NULL;
	
	lfc_sym = calloc(1, sizeof(struct lfc_ops));
	// static resolution
	lfc_sym->addreplica = &lfc_addreplica;
	lfc_sym->get_serrno = &C__serrno;
	lfc_sym->sstrerror = &sstrerror;
	lfc_sym->creatg= &lfc_creatg;
	lfc_sym->delreplica = &lfc_delreplica;
	lfc_sym->aborttrans = &lfc_aborttrans;
	lfc_sym->endtrans = &lfc_endtrans;
	lfc_sym->getpath = &lfc_getpath;
	lfc_sym->getlinks = &lfc_getlinks;
	lfc_sym->getreplica = &lfc_getreplica;
	lfc_sym->lstat = &lfc_lstat;
	lfc_sym->mkdirg =  &lfc_mkdirg;
	lfc_sym->seterrbuf = &lfc_seterrbuf; 
	lfc_sym->setfsizeg = &lfc_setfsizeg;
	lfc_sym->setfsize = &lfc_setfsize;
	lfc_sym->starttrans = &lfc_starttrans;
	lfc_sym->statg = &lfc_statg;
	lfc_sym->statr = &lfc_statr;
	lfc_sym->symlink = &lfc_symlink;
	lfc_sym->unlink = &lfc_unlink;
	lfc_sym->access = &lfc_access;
	lfc_sym->chmod = &lfc_chmod;
	lfc_sym->rename = &lfc_rename;
	lfc_sym->opendirg = &lfc_opendirg;
	lfc_sym->rmdir = &lfc_rmdir;
	lfc_sym->startsess = &lfc_startsess;
	lfc_sym->endsess = &lfc_endsess;
	lfc_sym->closedir = &lfc_closedir;
	lfc_sym->readdir= &lfc_readdir;
	lfc_sym->Cthread_init = &Cthread_init;
	lfc_sym->readlink = &lfc_readlink;
	lfc_sym->readdirx= &lfc_readdirx;
	lfc_sym->getcomment = &lfc_getcomment;
	lfc_sym->setcomment = &lfc_setcomment;
	lfc_sym->_Cthread_addcid =&_Cthread_addcid;


    // dyn resolution
    void* lib_handle = dlopen("liblfc.so.1", RTLD_LAZY);

    if(lib_handle) {
        lfc_sym->set_env = dlsym(lib_handle, "lfc_setenv");
        dlclose(lib_handle);
    }
    else {
        lfc_sym->set_env = NULL;
    }
    errno = 0;
	G_RETURN_ERR(lfc_sym, tmp_err, err);
}

/*
 *  convert a internal lfc statg to a POSIX lfc stat
 *  struct must be already allocated
 */
int gfal_lfc_convert_statg(struct stat* output, struct lfc_filestatg* input, GError** err){
	g_return_val_err_if_fail(output && input, -1, err, "[gfal_lfc_convert_statg] Invalid args statg/stat");
	output->st_mode = input->filemode;
	output->st_nlink = input->nlink;
	output->st_uid = input->uid;
	output->st_gid = input->gid;
	output->st_size = input->filesize;
	output->st_atime = input->atime;
	output->st_ctime = input->ctime;
	output->st_mtime = input->mtime;
	return 0;
}

/*
 *  convert a internal lfc lstat to a POSIX lfc stat
 *  struct must be already allocated
 */
int gfal_lfc_convert_lstat(struct stat* output, struct lfc_filestat* input, GError** err){
	g_return_val_err_if_fail(output && input, -1, err, "[gfal_lfc_convert_lstat] Invalid args statg/stat");
	output->st_mode = input->filemode;
	output->st_nlink = input->nlink;
	output->st_uid = input->uid;
	output->st_gid = input->gid;
	output->st_size = input->filesize;
	output->st_atime = input->atime;
	output->st_ctime = input->ctime;
	output->st_mtime = input->mtime;
	return 0;
}



/*
 * basic wrapper mkdir to the lfc api
 */
static int gfal_lfc_mkdir(struct lfc_ops* ops, const char* path, mode_t mode, GError** err){
	
	char struid[37];
	gfal_generate_guidG(struid,NULL);
	
	if(ops->mkdirg (path, struid, mode)){ 
		int sav_errno = gfal_lfc_get_errno(ops);
        gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), sav_errno,
                "Error while mkdir call in the lfc %s", strerror(sav_errno));
		return -1;
	}				
	return 0;	
}


/*
 * Begin a recursive call on mkdir to create a full tree path
 * @warning not safe, please ensure that string begin by "/"
 * 
 */
int gfal_lfc_mkdir_rec(struct lfc_ops* ops, char* browser_path, const char* full_path, mode_t mode, GError** err){
	int ret=-1;
	char* next_sep= strchr(browser_path, G_DIR_SEPARATOR); 
	if(  next_sep == NULL || *(next_sep+1) == '\0'){ // last folder
		return gfal_lfc_mkdir(ops, full_path, mode, err);
	}else{
		const int path_size =next_sep - full_path; 
		GError* tmp_err = NULL;
		char path[ path_size+1];
		
		*((char*) mempcpy(path, full_path, path_size )) = '\0';
		ret = gfal_lfc_mkdir(ops, path, ( 0700 | mode) , &tmp_err);
		if( ret== 0 || tmp_err->code == EEXIST || tmp_err->code == EACCES){
			g_clear_error(&tmp_err);
			return gfal_lfc_mkdir_rec(ops, next_sep+1, full_path, mode, err);
		}
		g_propagate_error(err, tmp_err);
		return ret;
	}
	
} 

/*
 *  Implementation of mkdir -p call on the lfc
 * 
 * */
int gfal_lfc_ifce_mkdirpG(struct lfc_ops* ops,const char* path, mode_t mode, gboolean pflag, GError**  err){
	g_return_val_err_if_fail( ops && path, -1, err, "[gfal_lfc_ifce_mkdirpG] Invalid args in ops or/and path");
	int ret;
	GError* tmp_err = NULL;
	
	ret = gfal_lfc_startTransaction(ops, &tmp_err);
	if(ret >=0 ){
		ret = gfal_lfc_mkdir(ops, path, mode, &tmp_err); // try to create the directory, suppose the non-recursive case
		if( tmp_err && tmp_err->code == ENOENT
					&& pflag){ // failure on the simple call, try to do a recursive create
			errno = 0;
			g_clear_error(&tmp_err);
			ret = gfal_lfc_mkdir_rec(ops, (char*)path+1, path,  mode, &tmp_err);
		}
		if( ret == 0)
			ret =gfal_lfc_endTransaction(ops,&tmp_err);
		else
			gfal_lfc_abortTransaction(ops,NULL);
	}
	if(tmp_err)
		gfal2_propagate_prefixed_error(err, tmp_err, __func__);
	else
		errno = 0;
	return ret;
}
/*
 * return a list of surls from a getreplica request
 * 
 */
char ** gfal_lfc_getSURL(struct lfc_ops* ops, const char* path, GError** err){
	struct lfc_filereplica* list = NULL;
	char **replicas = NULL;
	int size=0,i;
	
	if (ops->getreplica (path, NULL, NULL, &size, &list) < 0) {
		int myerrno = gfal_lfc_get_errno(ops);
        gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), myerrno,
                "error reported from lfc : %s", gfal_lfc_get_strerror(ops));
		return NULL;
	}
	replicas = malloc( sizeof(char*)* (size+1));
	replicas[size]= NULL;
	for(i=0; i< size; ++i){
		replicas[i] = strndup(list[i].sfn, GFAL_URL_MAX_LEN);
	}
	free(list);
	return replicas;
	
}

/*
 * return the comment associated with this path
 *  follow the xattr behavior, if buff==NULL, return only the appropriate buffer size for the call
 * @return the size of the comment or -1 if error
 */
int gfal_lfc_getComment(struct lfc_ops *ops, const char* lfn, char* buff, size_t s_buff, GError** err){
	g_return_val_err_if_fail(lfn, -1, err, "bad path");
	const size_t req_size = CA_MAXCOMMENTLEN+1;
	char local_buff[CA_MAXCOMMENTLEN+1];
	int ret, resu_len;
	
	if(buff == NULL || s_buff == 0)
		return req_size;
	else{
		ret = ops->getcomment(lfn, local_buff);
		if(ret < 0){
			const int sav_errno = gfal_lfc_get_errno(ops);
			if(sav_errno == ENOENT) { // no comments is define or not file exist, can be ambigous
				resu_len = 0;
				*buff = '\0';
				ret = 0;
			}
			else {
                gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
                        "Error report from LFC : %s", gfal_lfc_get_strerror(ops) );
			}
		}else{
			resu_len = strnlen(local_buff, MIN(s_buff, req_size));
			*((char*)mempcpy(buff, local_buff,resu_len )) = '\0';		
		}
		return (ret==0)?(resu_len):-1;
	}
}

int gfal_lfc_setComment(struct lfc_ops * ops, const char* lfn, const char* buff, size_t s_buff, GError** err){
	g_return_val_err_if_fail(lfn, -1, err, "bad path");	
	int res = -1;
	char internal_buff[GFAL_URL_MAX_LEN];
	GError* tmp_err=NULL;
	
	if(s_buff > 0 && buff != NULL){
		*((char*)mempcpy(internal_buff, buff, MIN(s_buff, GFAL_URL_MAX_LEN-1))) = '\0';
		if( (res = ops->setcomment(lfn, internal_buff)) !=0 ){
			const int sav_errno = gfal_lfc_get_errno(ops);
            gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), sav_errno,
                    "Error report from LFC : %s", gfal_lfc_get_strerror(ops) );
		}
	}else{
        gfal2_set_error(&tmp_err, gfal2_get_plugin_lfc_quark(), EINVAL, __func__, "sizeof the buffer incorrect");
	}
	return res;
}

/*
 * provide the checksum for a given file
 *  @return the size of the checksum string or -1 if error
 */
int gfal_lfc_getChecksum(struct lfc_ops* ops, const char* lfn, lfc_checksum* checksum, GError** err){
	g_return_val_err_if_fail(ops && checksum, -1, err, " inval args")
	GError * tmp_err=NULL;
	struct lfc_filestatg statbuf;
    memset(&statbuf, 0, sizeof(struct lfc_filestatg));
	int ret;
	if( (ret = gfal_lfc_statg(ops, lfn, &statbuf, &tmp_err)) == 0){
		*((char*) mempcpy(checksum->type, statbuf.csumtype, sizeof(char)*3)) = '\0';
		*((char*) mempcpy(checksum->value, statbuf.csumvalue, sizeof(char)*33)) = '\0';		
	}
	if(tmp_err)
		gfal2_propagate_prefixed_error(err, tmp_err, __func__);
	return ret;
}

int gfal_lfc_statg(struct lfc_ops* ops, const char* lfn, struct lfc_filestatg* statbuf, GError** err){
	int ret = ops->statg(lfn, NULL, statbuf);
	if(ret != 0){
		int sav_errno = gfal_lfc_get_errno(ops);
        gfal2_set_error(err, gfal2_get_plugin_lfc_quark(), sav_errno, __func__,
                "Error report from LFC : %s", gfal_lfc_get_strerror(ops) );
	}	
	return ret;
}

ssize_t g_strv_catbuff(char** strv, char* buff, size_t size){
	if(strv == NULL)
		return -1;
	const size_t sbuff = g_strv_length(strv);
	ssize_t resu=0;
	size_t i;
	char* p = buff;
	for(i=0; i < sbuff; ++i){
		const size_t s_str= strnlen(strv[i], GFAL_URL_MAX_LEN);
		resu += s_str+1;
		if(buff && size){
			*(p = (char*) mempcpy(p,strv[i], MIN(size, s_str) ))= '\0';
			++p;
		}
		size = (size >= s_str+1)?(size-s_str-1):0;
	}
	return resu;
}


int gfal_lfc_get_errno(struct lfc_ops* ops){
	int lfc_error;
#if defined(_REENTRANT) || defined(_THREAD_SAFE) || (defined(_WIN32) && (defined(_MT) || defined(_DLL)))
	lfc_error = *(ops->get_serrno());
#else
	lfc_error = *ops->get_serrno ;
#endif
	switch(lfc_error){
		case ESEC_BAD_CREDENTIALS:
			lfc_error = EPERM;
		break;
		default:
			lfc_error = (lfc_error < 1000)?lfc_error:ECOMM;
	}
	return lfc_error;
}

void gfal_lfc_reset_errno(struct lfc_ops* ops){
    int *lfc_error;
#if defined(_REENTRANT) || defined(_THREAD_SAFE) || (defined(_WIN32) && (defined(_MT) || defined(_DLL)))
    lfc_error = ops->get_serrno();
#else
    lfc_error = ops->get_serrno;
#endif
    *lfc_error = 0;
}

char*  gfal_lfc_get_strerror(struct lfc_ops* ops){
#if defined(_REENTRANT) || defined(_THREAD_SAFE) || (defined(_WIN32) && (defined(_MT) || defined(_DLL)))
	return ops->sstrerror (*(ops->get_serrno()));
#else
	return ops->sstrerror (*ops->get_serrno);
#endif
}



/*
 * @brief generate an uiid string
 * Generate a uuid string and copy it in the buf,
 * @warning buff must be > uuid size ( 37 bytes )
 * 
 * */
void gfal_generate_guidG(char* buf, GError** err){
    uuid_t myuid;
    
    uuid_generate_random(myuid);
    uuid_unparse (myuid, buf);
    uuid_clear(myuid);
}
