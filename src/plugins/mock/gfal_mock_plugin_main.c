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
 * file gfal_mock_plugin_main.c
 * brief file plugin
 * author Michal Simon
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
#include <attr/xattr.h>

#include <common/gfal_constants.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_errverbose.h>
#include <fdesc/gfal_file_handle.h>
#include <config/gfal_config.h>


const char* mock_prefix="mock:";
const char* mock_config_group= "MOCK PLUGIN";


const char* MAX_TRANSFER_TIME = "MAX_TRANSFER_TIME";
const char* MIN_TRANSFER_TIME = "MIN_TRANSFER_TIME";

const char* FILE_SIZE = "size";
const char* FILE_SIZE_PRE = "size_pre";
const char* FILE_SIZE_POST = "size_post";

typedef enum _stat_order {

	STAT_SOURCE = 0,
	STAT_DESTINATION_PRE,
	STAT_DESTINATION_POST

} stat_order;

unsigned int s_prefix = 0;

// LFC plugin GQuark
GQuark gfal2_get_plugin_mock_quark(){
    return g_quark_from_static_string(GFAL2_QUARK_PLUGINS "::FILE");
}

static unsigned int mock_prefix_len(){
    if(!s_prefix)
        g_atomic_int_set(&s_prefix, strlen(mock_prefix));
    return s_prefix;
}

static gboolean is_mock_uri(const char* src){
    return (strncmp(src, mock_prefix, s_prefix) == 0);
}

/*
 * srm plugin id
 */
const char* gfal_mock_plugin_getName(){
	return "mock_plugin";
}


int gfal_lmock_path_checker(plugin_handle handle, const char * url){
    const unsigned int s_url = strnlen(url,GFAL_URL_MAX_LEN);
    if( mock_prefix_len() <  s_url && s_url < GFAL_URL_MAX_LEN
            && strncmp(url, mock_prefix, s_prefix) ==0)
        return 0;
    return -1;
}

/*
 * url checker for the file module
 **/
static gboolean gfal_mock_check_url(plugin_handle handle, const char* url, plugin_mode mode, GError** err){
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
            return (gfal_lmock_path_checker(handle, url)==0);
		default:
			return FALSE;		
	}
}

void gfal_plugin_mock_report_error(const char* funcname, GError** err){
    g_set_error(err, gfal2_get_plugin_mock_quark(),errno, "[%s] errno reported by local system call %s", funcname, strerror(errno));
}


int gfal_plugin_mock_get_value(const char* url, const char* key)
{
	// look for the place where parameter list starts
	char* str = strchr(url, '?');
	// if there is no parameter list ...
	if (str == NULL) return -1;
	// find the parameter name
	str = strstr(str, key);
	// if the parameter is not on the list
	if (str == NULL) return -1;
	// find the assignment
	str = strchr(str, '=');
	// if no value was assigned ...
	if (str == NULL) return -1;
	// get the value
	return atoi(str + 1);
}

int gfal_plugin_mock_stat(plugin_handle plugin_data, const char* path, struct stat* buf, GError ** err){

	// it is assumed that the stat operations will be done in following order:
	//   - stat source
	//   - stat destination before transfer
	//   - stat destination after transfer
	static int order = 0;

	int size = 0;

	switch (order)
	{

	case STAT_SOURCE:
		size = gfal_plugin_mock_get_value(path, FILE_SIZE);
		break;

	case STAT_DESTINATION_PRE:
		size = gfal_plugin_mock_get_value(path, FILE_SIZE_PRE);
		break;

	case STAT_DESTINATION_POST:
		size = gfal_plugin_mock_get_value(path, FILE_SIZE_POST);
		break;
	}
	
	if (size >= 0) buf->st_size = size;

	// let's prepare for the next stat operation
	order++;

	// since it's a mock transfer anyway let's say the stat was successful
	// unless we would like to emulate the file is not there
	return 0;
}

gboolean gfal_plugin_mock_check_url_transfer(plugin_handle handle, const char* src, const char* dst, gfal_url2_check type){
    gboolean res = FALSE;

    if( src != NULL && dst != NULL){
        if (type == GFAL_FILE_COPY && is_mock_uri(src) && is_mock_uri(dst))
            res = TRUE;
    }
    return res;
}

int gfal_plugin_mock_filecopy(plugin_handle handle, gfal2_context_t context, gfalt_params_t params, const char* src, const char* dst, GError ** err){
    // here we are mocking the copying
	int max = gfal2_get_opt_integer_with_default(context, mock_config_group, MAX_TRANSFER_TIME, 100);
	int min = gfal2_get_opt_integer_with_default(context, mock_config_group, MIN_TRANSFER_TIME, 10);

	int seconds = rand() % (max - min) + min;
	sleep(seconds);
	return 0;
}

/*
 * Init function, called before all
 * */
gfal_plugin_interface gfal_plugin_init(gfal_handle handle, GError** err){

	gfal_plugin_interface mock_plugin;
    memset(&mock_plugin,0,sizeof(gfal_plugin_interface));	// clear the plugin

    mock_plugin.plugin_data = NULL;
    mock_plugin.check_plugin_url = &gfal_mock_check_url;
    mock_plugin.getName= &gfal_mock_plugin_getName;

    mock_plugin.statG = &gfal_plugin_mock_stat;


    mock_plugin.check_plugin_url_transfer = &gfal_plugin_mock_check_url_transfer;
    mock_plugin.copy_file = &gfal_plugin_mock_filecopy;

	return mock_plugin;
}



