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
 * file gfal_common.c
 * brief the header file with the main srm funcs of the common API
 * author Devresse Adrien
 */
 
#define _GNU_SOURCE

#include <regex.h>
#include <time.h> 



#include "gfal_common_srm.h"
#include "gfal_common_srm_access.h"
#include "gfal_common_srm_bringonline.h"
#include "gfal_common_srm_internal_layer.h"
#include "gfal_common_srm_access.h"
#include "gfal_common_srm_mkdir.h"
#include "gfal_common_srm_stat.h"
#include "gfal_common_srm_rmdir.h"
#include "gfal_common_srm_opendir.h"
#include "gfal_common_srm_open.h"
#include "gfal_common_srm_readdir.h"
#include "gfal_common_srm_chmod.h"
#include "gfal_common_srm_getxattr.h"
#include "gfal_common_srm_checksum.h"
#include "gfal_common_srm_copy.h"
#include "gfal_common_srm_url_check.h"

#include <cancel/gfal_cancel.h>
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_plugin.h>

#include <file/gfal_file_api.h>

/*
 * 
 * list of the turls supported protocols
 */
static char* srm_turls_sup_protocols_default[] = { "rfio", "gsidcap", "dcap", "kdcap", "gsiftp",  NULL };

/*
 * list of protocols supporting third party transfer
 */
char* srm_turls_thirdparty_protocols_default[] = { "gsiftp", NULL };

char** srm_get_turls_sup_protocol(gfal2_context_t context){
    gsize len;
    return gfal2_get_opt_string_list_with_default(context,
                                                  srm_config_group,
                                                  srm_config_turl_protocols,
                                                  &len,
                                                  srm_turls_sup_protocols_default);
}


char** srm_get_3rdparty_turls_sup_protocol(gfal2_context_t context){
    gsize len;
    return gfal2_get_opt_string_list_with_default(context, srm_config_group,
                                                    srm_config_3rd_party_turl_protocols,
                                                    &len,
                                                    srm_turls_thirdparty_protocols_default);
}



/*
 * 
 * srm plugin id
 */
const char* gfal_srm_getName(){
	return "srm_plugin";
}


int gfal_checker_compile(gfal_srmv2_opt* opts, GError** err){
	int ret = regcomp(&opts->rexurl, "^srm://([:alnum:]|-|/|.|_)+$",REG_ICASE | REG_EXTENDED);
    g_return_val_err_if_fail(ret==0,-1,err,"[gfal_surl_checker_] fail to compile regex for srm checking, report this bug");
    ret = regcomp(&(opts->rex_full), "^srm://([:alnum:]|-|/|.|_)+:[0-9]+/([:alnum:]|-|/|.|_)+?SFN=",REG_ICASE | REG_EXTENDED);
    g_return_val_err_if_fail(ret==0,-1,err,"[gfal_surl_checker_] fail to compile regex for the full SURL srm checking, report this bug");
	return ret;
}

/*
 * parse a surl to check the validity
 */
int gfal_surl_checker(plugin_handle ch, const char* surl, GError** err){
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;
	if(surl == NULL || strnlen(surl, GFAL_URL_MAX_LEN) == GFAL_URL_MAX_LEN){
		g_set_error(err, 0, EINVAL, "[%s] Invalid surl, surl too long or NULL",__func__);
		return -1;
	}	
	return regexec(&opts->rexurl,surl,0,NULL,0);
}

/*
 * 
 * convenience func for a group of surls 
 * */
gboolean gfal_srm_surl_group_checker(gfal_srmv2_opt* opts,char** surls, GError** err){
	GError* tmp_err=NULL;
	if(surls == NULL ){
		g_set_error(err, 0, EINVAL, "[%s] Invalid argument surls ", __func__);
		return FALSE;
	}
	while(*surls != NULL){
		if( gfal_surl_checker(opts, *surls, &tmp_err) != 0){
			g_propagate_prefixed_error(err,tmp_err,"[%s]",__func__);	
			return FALSE;
		}
		surls++;
	}
	return TRUE;
}


/*
 * url checker for the srm module, surl part
 * 
 * */
static gboolean gfal_srm_check_url(plugin_handle handle, const char* url, plugin_mode mode, GError** err){
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
        case GFAL_PLUGIN_CHECKSUM:
        case GFAL_PLUGIN_MKDIR_REC:
        case GFAL_PLUGIN_BRING_ONLINE:
			return (gfal_surl_checker(handle, url,  err)==0);
		default:
			return FALSE;		
	}
}
/*
 * destroyer function, call when the module is unload
 * */
void gfal_srm_destroyG(plugin_handle ch){
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;
	regfree(&opts->rexurl);
	regfree(&opts->rex_full);
	gsimplecache_delete(opts->cache);
	free(opts);
}

static void srm_internal_copy_stat(gpointer origin, gpointer copy){
	memcpy(copy, origin, sizeof(struct stat));
}

/*
 * Init an opts struct with the default parameters
 * */
void gfal_srm_opt_initG(gfal_srmv2_opt* opts, gfal_handle handle){
	memset(opts, 0, sizeof(gfal_srmv2_opt));
	gfal_checker_compile(opts, NULL);
    opts->opt_srmv2_desiredpintime = 0;
	opts->srm_proto_type = PROTO_SRMv2;
	opts->handle = handle;	
    opts->cache = gsimplecache_new(5000, &srm_internal_copy_stat, sizeof(struct stat));
}



/*
 * Init function, called before all
 * */
gfal_plugin_interface gfal_plugin_init(gfal_handle handle, GError** err){
	gfal_plugin_interface srm_plugin;
	memset(&srm_plugin,0,sizeof(gfal_plugin_interface));	// clear the plugin	
	gfal_srmv2_opt* opts = g_new(struct _gfal_srmv2_opt,1);	// define the srmv2 option struct and clear it	
	gfal_srm_opt_initG(opts, handle);
	srm_plugin.plugin_data = (void*) opts;	
	srm_plugin.check_plugin_url = &gfal_srm_check_url;
	srm_plugin.plugin_delete = &gfal_srm_destroyG;
	srm_plugin.accessG = &gfal_srm_accessG;
	srm_plugin.mkdirpG = &gfal_srm_mkdirG;
	srm_plugin.statG= &gfal_srm_statG;
	srm_plugin.lstatG = &gfal_srm_statG; // no management for symlink in srm protocol/srm-ifce, just map to stat
	srm_plugin.rmdirG = &gfal_srm_rmdirG;
	srm_plugin.opendirG = &gfal_srm_opendirG;
	srm_plugin.readdirG = &gfal_srm_readdirG;
	srm_plugin.closedirG = &gfal_srm_closedirG;
	srm_plugin.getName= &gfal_srm_getName;
	srm_plugin.openG = &gfal_srm_openG;
	srm_plugin.closeG = &gfal_srm_closeG;
	srm_plugin.readG= &gfal_srm_readG;
	srm_plugin.preadG = &gfal_srm_preadG;
	srm_plugin.writeG= &gfal_srm_writeG;
	srm_plugin.chmodG= &gfal_srm_chmodG;
	srm_plugin.lseekG= &gfal_srm_lseekG;
	srm_plugin.unlinkG = &gfal_srm_unlinkG;
	srm_plugin.getxattrG = &gfal_srm_getxattrG;
	srm_plugin.listxattrG = &gfal_srm_listxattrG;
    srm_plugin.checksum_calcG = &gfal_srm_checksumG;
    srm_plugin.copy_file = &srm_plugin_filecopy;
    srm_plugin.check_plugin_url_transfer =&plugin_url_check2;
    srm_plugin.bring_online = &gfal_srmv2_bring_onlineG;
    srm_plugin.release_file = &gfal_srmv2_release_fileG;
	return srm_plugin;
}







/*
 * Construct a key for the cache system from a url and a prefix
 * */
inline char* gfal_srm_construct_key(const char* url, const char* prefix, char* buff, const size_t s_buff){
	g_strlcpy(buff, prefix, s_buff);
	g_strlcat(buff, url, s_buff);
	char* p2 = buff + strlen(prefix) + strlen(GFAL_PREFIX_SRM) + 2;
	while(*p2 != '\0'){ //remove the duplicate //
		if(*p2 == '/' && *(p2+1) == '/' ){
			memmove(p2,p2+1,strlen(p2+1)+1);
		}else
			p2++;
	}
	return buff;
}

/*
 *   brief create a full endpath from a surl with full endpath
 * */
char* gfal_get_fullendpoint(const char* surl, GError** err){
	char* p = strstr(surl,"?SFN=");
	const int len_prefix = strlen(GFAL_PREFIX_SRM);						// get the srm prefix length
	const int len_endpoint_prefix = strlen(GFAL_ENDPOINT_DEFAULT_PREFIX); // get the endpoint protocol prefix len 
	g_return_val_err_if_fail(p && len_prefix && (p>(surl+len_prefix)) && len_endpoint_prefix,NULL,err,"[gfal_get_fullendpoint] full surl must contain ?SFN= and a valid prefix, fatal error");	// assertion on params
	char* resu = calloc(p-surl-len_prefix+len_endpoint_prefix, sizeof(char));	
	strncpy(resu, GFAL_ENDPOINT_DEFAULT_PREFIX, len_endpoint_prefix);	// copy prefix
	strncpy(resu + len_endpoint_prefix, surl+len_prefix, p- surl-len_prefix);		// copy endpoint
	return resu;
}

/*
 *  brief get the hostname from a surl
 *   return return NULL if error and set err else return the hostname value
 */
 char*  gfal_get_hostname_from_surl(const char * surl, GError** err){
	 const int srm_prefix_len = strlen(GFAL_PREFIX_SRM);
	 const int surl_len = strnlen(surl,2048);
	 g_return_val_err_if_fail(surl &&  (srm_prefix_len < surl_len)  && (surl_len < 2048),NULL, err, "[gfal_get_hostname_from_surl] invalid value in params");
	 char* p = strchr(surl+srm_prefix_len,'/');
	 char* prep = strstr(surl, GFAL_PREFIX_SRM);
	 if(prep != surl){
		 g_set_error(err,0, EINVAL, "[gfal_get_hostname_from_surl not a valid surl");
		 return NULL;
	 }
	 return strndup(surl+srm_prefix_len, p-surl-srm_prefix_len);	 
 }
 
 /*
  * map a bdii se protocol type to a gfal protocol type
  */
  /*
static enum gfal_srm_proto gfal_get_proto_from_bdii(const char* se_type_bdii){
	enum gfal_srm_proto resu;
	if( strcmp(se_type_bdii,"srm_v1") == 0){
		resu = PROTO_SRM;
	}else if( strcmp(se_type_bdii,"srm_v2") == 0){
		resu = PROTO_SRMv2;
	}else{
		resu = PROTO_ERROR_UNKNOW;
	}
	return resu;
}
*/

/*
 *  brief accessor for the default storage type definition
 * */
void gfal_set_default_storageG(gfal_srmv2_opt* opts, enum gfal_srm_proto proto){
	opts->srm_proto_type = proto;
}




 
int gfal_srm_convert_filestatuses_to_GError(struct srmv2_filestatus* statuses, int n, GError** err){
	g_return_val_err_if_fail(statuses && n, -1, err, "[gfal_srm_convert_filestatuses_to_GError] args invalids");	
	int i;
	int ret =0;
	for(i=0; i< n; ++i){
		if(statuses[i].status != 0){
			g_set_error(err, 0, statuses[i].status, "[%s] Error on the surl %s while putdone : %s", __func__,
				statuses[i].surl, statuses[i].explanation);
			ret = -1;			
		}
	}
	return ret;
}
 
void gfal_srm_report_error(char* errbuff, GError** err){
	int errcode = (errno != ECOMM && errno != 0)?errno:ECOMM;
    g_set_error(err,0, errcode, "srm-ifce err: %s, err: %s", strerror(errcode), errbuff);
}

gboolean gfal_srm_check_cancel(gfal2_context_t context, GError** err){
    if(gfal2_is_canceled(context)){
        g_set_error(err, 0, ECANCELED, "SRM operation canceled");
        return TRUE;
    }
    return FALSE;
}


