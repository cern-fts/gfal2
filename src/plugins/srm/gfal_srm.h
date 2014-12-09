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
 * gfal_srm.h
 * the header file with the main srm funcs of the common API
 * author Devresse Adrien
 */

#include <string.h>
#include <regex.h>

#include <gfal_plugins_api.h>
#include <gsimplecache/gcachemain.h>

#define GFAL_PREFIX_SRM "srm://"
#define GFAL_PREFIX_SRM_LEN 6
#define GFAL_ENDPOINT_DEFAULT_PREFIX "httpg://"
#define GFAL_DEFAULT_SERVICE_ENDPOINT_SUFFIX "/srm/managerv2"

#define SRM_XATTR_GETURL "user.replicas"

#define GFAL_SRM_LSTAT_PREFIX "lstat_"

//typedef struct srm_spacemd gfal_spacemd;
enum status_type {DEFAULT_STATUS = 0, MD_STATUS, PIN_STATUS};

enum se_type {TYPE_NONE = 0, TYPE_SRM, TYPE_SRMv2, TYPE_SE};
enum gfal_srm_proto {PROTO_SRM=0, PROTO_SRMv2, PROTO_ERROR_UNKNOW};


// SRM plugin GQuark
GQuark gfal2_get_plugin_srm_quark();


/*
 * the state of the last request -> depreciated
 *  needed to get the response
 */
typedef struct _gfal_request_state{
    char *						srmv2_token;
    struct srmv2_filestatus *	srmv2_statuses;
    struct srmv2_pinfilestatus *srmv2_pinstatuses;
    enum gfal_srm_proto current_request_proto;
    char * 						request_endpoint;
    gboolean					finished;			// finished or not
    int							number;				// number of files in request
 } gfal_request_state;

/*
 * @struct structure for the srmv2
 */
typedef struct srm_context* srm_context_t;
typedef struct _gfal_srmv2_opt{
	enum gfal_srm_proto srm_proto_type;		// default protocol version
	regex_t rexurl;
	regex_t rex_full;
	gfal2_context_t handle;
	GSimpleCache* cache;

	char srm_ifce_error_buffer[GFAL_ERRMSG_LEN];

	GStaticRecMutex srm_context_mutex; // Avoid same context being used from more than one thread at the time
	srm_context_t srm_context;

	// Used to know if the srm context must be cleaned
	char x509_ucert[GFAL_URL_MAX_LEN], x509_ukey[GFAL_URL_MAX_LEN];
	char endpoint[GFAL_URL_MAX_LEN];
} gfal_srmv2_opt;


typedef struct _gfal_srm_result{
	char turl[GFAL_URL_MAX_LEN+1]; // turl associated with the request ( main result )
	char *reqtoken; // token of the request ( common to all result of a request )
	int err_code;		// errcode, !=0 if error
	char err_str[GFAL_ERRMSG_LEN+1];	// explanation about the error
} gfal_srm_result;

typedef struct _gfal_srm_params{
	char** protocols;				// optional protocols list for manual set
	enum gfal_srm_proto proto_version;		// default protocol version
	char * spacetokendesc;		// optional spacetokens desc for srmv2
	//int desiredpintime;			//	optional desired default endpoint
    size_t file_size;
}* gfal_srm_params_t;

typedef void* srm_request_handle;

// default set of protocols for TURL in case of remote IO
char** srm_get_turls_sup_protocol(gfal2_context_t context);

// default set of protocols for TURL in case of third party transfer
char** srm_get_3rdparty_turls_sup_protocol(gfal2_context_t context);

const char* gfal_srm_getName();

gfal_plugin_interface gfal_srm_initG(gfal2_context_t handle, GError** err);

void gfal_srm_destroyG(plugin_handle ch);

void gfal_srm_opt_initG(gfal_srmv2_opt* opts, gfal2_context_t handle);


inline char* gfal_srm_construct_key(const char* url, const char* prefix, char* buff, const size_t s_buff);

void gfal_set_default_storageG(gfal_srmv2_opt* opts, enum gfal_srm_proto proto);


int gfal_srm_convert_filestatuses_to_GError(struct srmv2_filestatus* statuses, int n, GError** err);

gboolean gfal_srm_check_cancel(gfal2_context_t context, GError** err);

int gfal_surl_checker(plugin_handle ch, const char* surl, GError** err);



