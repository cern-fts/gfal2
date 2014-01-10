#pragma once
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
  * lfc_ifce_ng.h
  * internal header of the lfc plugin module
  * author Adrien Devresse
 */
 
#define GFAL_MAX_LFCHOST_LEN 1024

#include <lfc/lfc_api.h>
#include <lfc/serrno.h>
#include <Cthread_api.h>
#include <Cthread_typedef.h>
#include <regex.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_constants.h>
#include <gsimplecache/gcachemain.h>


#define LFC_ENV_VAR_HOST "LFC_HOST"
#define LFC_GROUP_CONFIG_VAR "LFC PLUGIN"
#define LFC_HOST_CONFIG_VAR LFC_ENV_VAR_HOST
#define LFC_ENV_VAR_CONNTIMEOUT "LFC_CONNTIMEOUT"
#define LFC_ENV_VAR_CONRETRY "LFC_CONRETRY"
#define LFC_ENV_VAR_CONRETRYINT "LFC_CONRETRYINT"

#define LFC_ENV_VAR_GROUP_PLUGIN "LFC PLUGIN"

typedef struct _lfc_checksum{
	char type[255];
	char value[GFAL_URL_MAX_LEN];
} lfc_checksum;


struct lfc_ops {
    char* lfc_endpoint_predefined; // default  lfc env var
    char* lfc_conn_retry;
    char* lfc_conn_try_int;
    char* lfc_conn_timeout;
	regex_t rex; // regular expression compiled 
	gfal_handle handle;
	GSimpleCache* cache_stat;
#if defined(_REENTRANT) || defined(_THREAD_SAFE) || (defined(_WIN32) && (defined(_MT) || defined(_DLL)))
	int*	(*get_serrno)(void);
#else
	int	value_serrno;
#endif
	char	*(*sstrerror)(int);
	int	(*addreplica)(const char *, struct lfc_fileid *, const char *, const char *, const char, const char, const char *, const char *);
	int	(*creatg)(const char *, const char *, mode_t);
	int	(*delreplica)(const char *, struct lfc_fileid *, const char *);
	int	(*aborttrans)();
	int	(*endtrans)();
	int	(*getpath)(char *, u_signed64, char *);
	int (*getlinks)(const char *, const char *, int *, struct lfc_linkinfo **);
	int (*getreplica)(const char *, const char *, const char *, int *, struct lfc_filereplica **);
	int (*setcomment) (const char * path, char * comment );
	int (*getcomment) (const char * path, char * comment);
	int	(*lstat)(const char *, struct lfc_filestat *);
	int (*readlink)(const char *, char *, size_t);
	int	(*mkdirg)(const char *, const char *, mode_t);
	int	(*seterrbuf)(char *, int);
	int	(*setfsizeg)(const char *, u_signed64, const char *, char *);
	int	(*setfsize)(const char *, struct lfc_fileid *, u_signed64);
	int	(*starttrans)(char *, char *);
	int	(*statg)(const char *, const char *, struct lfc_filestatg *);
	int	(*statr)(const char *, struct lfc_filestatg *);
	int	(*symlink)(const char *, const char *);
	int	(*unlink)(const char *);
	int	(*access)(const char *, int);
	int	(*chmod)(const char *, mode_t);
	int (*closedir)(lfc_DIR*);
	int	(*rename)(const char *, const char *);
	lfc_DIR *(*opendirg)(const char *, const char *);
	struct dirent* (*readdir)(lfc_DIR *);
	struct lfc_direnstat* (*readdirx)(lfc_DIR *dirp);
	int	(*rmdir)(const char *);
	int (*startsess) (char *, char *); 
	int (*endsess) ();
	int (*Cthread_init)();
	int (*_Cthread_addcid)(char *, int, char *, int, Cth_pid_t *, unsigned, void *(*)(void *), int);
    int (*set_env)(const char*, const char*, int);
};


int lfc_configure_environment(struct lfc_ops * ops, const char* host, GError** err);


int gfal_lfc_get_errno(struct lfc_ops* ops);

int gfal_lfc_regex_compile(regex_t* rex, GError** err);

char*  gfal_lfc_get_strerror(struct lfc_ops* ops);

char* gfal_convert_guid_to_lfn(plugin_handle handle, char* guid, GError ** err);

int gfal_convert_guid_to_lfn_r(plugin_handle handle, const char* guid, char* buff_lfn, size_t sbuff_lfn, GError ** err);

int gfal_lfc_statg(struct lfc_ops* ops, const char*, struct lfc_filestatg* resu, GError** err);

int gfal_lfc_getComment(struct lfc_ops *ops, const char* lfn, char* buff, size_t s_buff, GError** err);

int gfal_lfc_setComment(struct lfc_ops * ops, const char* lfn, const char* buff, size_t s_buff, GError** err);

int gfal_lfc_getChecksum(struct lfc_ops* ops, const char* lfn, lfc_checksum* checksum, GError** err);

int gfal_lfc_convert_statg(struct stat* output, struct lfc_filestatg* input, GError** err);

int gfal_lfc_ifce_mkdirpG(struct lfc_ops* ops,const char* path, mode_t mode, gboolean pflag, GError**  err);

char ** gfal_lfc_getSURL(struct lfc_ops* ops, const char* path, GError** err);

void gfal_lfc_init_thread(struct lfc_ops* ops);

int gfal_lfc_startSession(struct lfc_ops* ops, GError ** err); 

void gfal_auto_maintain_session(struct lfc_ops* ops, GError ** err); 

ssize_t g_strv_catbuff(char** strv, char* buff, size_t size);

int gfal_lfc_convert_lstat(struct stat* output, struct lfc_filestat* input, GError** err);

void gfal_generate_guidG(char* buf, GError** err);

struct lfc_ops* gfal_load_lfc(const char* name, GError** err);

int gfal_lfc_register_check(plugin_handle plugin_data,  const char* src, const char* dst, gfal_url2_check check);

int gfal_lfc_register(plugin_handle plugin_data, gfal2_context_t context, gfalt_params_t params, const char* src, const char* dst, GError**);
