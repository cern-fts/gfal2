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
 * plugin mock
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
#include <common/gfal_common_err_helpers.h>
#include <fdesc/gfal_file_handle.h>
#include <config/gfal_config.h>
#include <transfer/gfal_transfer.h>
#include <checksums/checksums.h>


const char* mock_prefix = "mock:";
const char* mock_config_group = "MOCK PLUGIN";
const char* mock_skip_transfer_config = "SKIP_SOURCE_CHECKSUM";

const char* MAX_TRANSFER_TIME = "MAX_TRANSFER_TIME";
const char* MIN_TRANSFER_TIME = "MIN_TRANSFER_TIME";

const char* FILE_SIZE = "size";
const char* FILE_SIZE_PRE = "size_pre";
const char* FILE_SIZE_POST = "size_post";
const char* CHECKSUM = "checksum";
const char* TIME = "time";

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
    return (strncmp(src, mock_prefix, mock_prefix_len()) == 0);
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

void gfal_plugin_mock_report_error(const char* msg, GError** err)
{
    g_set_error(err, gfal2_get_plugin_mock_quark(), errno, msg);
}


void gfal_plugin_mock_get_value(const char* url, const char* key, char* value)
{
	// make sure it's an empty C-string
	value[0] = '\0';
	// look for the place where parameter list starts
	char* str = strchr(url, '?');
	// if there is no parameter list ...
	if (str == NULL) return;
	// find the parameter name
	str = strstr(str, key);
	// if the parameter is not on the list
	if (str == NULL) return;
	// find the assignment
	str = strchr(str, '=');
	// if no value was assigned ...
	if (str == NULL) return;

	str = str + 1;
	char* end = strchr(str, '&');

	if (end)
	{
		// if it is not the last parameter ...
		int size = end - str;
		g_strlcpy(value, str, size);
	}
	else
	{
		// if it is the last parameter just copy the string until it ends
	    g_stpcpy(value, str);
	}
}

int gfal_plugin_mock_get_size(const char* buff)
{
	// if the string is empty return -1
	if (buff == 0 || buff[0] == '\0') return -1;
	// get the value
	return atoi(buff);
}

void gfal_plugin_mock_get_checksum(char* buff, char* checksum)
{
	// make sure that checksum is empty
	checksum[0] = '\0';
	// if buff is empty there's nothing to do
	if (buff == 0 || buff[0] == '\0') return;

	// look for the delimiter
	char* str = strchr(buff, ':');
	// if it's not a proper checksum return
	if (!str) return;

	// copy checksum
	strcpy(checksum, str + 1);
}

int gfal_plugin_mock_stat(plugin_handle plugin_data, const char* path, struct stat* buf, GError ** err){

	// it is assumed that the stat operations will be done in following order:
	//   - stat source
	//   - stat destination before transfer
	//   - stat destination after transfer
	
	/*USE HARDCODED FOR NOW SINCE RUCIO WOULD LIKE TO USE IT*/
	buf->st_size = 10;
	
        time_t epoch_time;
    	struct tm *tm_p;
    	epoch_time = time( NULL );
    	tm_p = localtime( &epoch_time );
        int secs =  tm_p->tm_sec;	
	
	if(secs == 5 || secs == 15 || secs == 25 || secs == 45 || secs == 60)
	{
		gfal_plugin_mock_report_error("Mock failure", err);	
		return -1;
        }		
	
	return 0;
	
	
	
	static int order = 0;

	int size = 0;
	char buff[GFAL_URL_MAX_LEN] = { 0 };

	switch (order)
	{

	case STAT_SOURCE:
		gfal_plugin_mock_get_value(path, FILE_SIZE, buff);
		size = gfal_plugin_mock_get_size(buff);
		break;

	case STAT_DESTINATION_PRE:
		gfal_plugin_mock_get_value(path, FILE_SIZE_PRE, buff);
		size = gfal_plugin_mock_get_size(buff);
		break;

	case STAT_DESTINATION_POST:
		gfal_plugin_mock_get_value(path, FILE_SIZE_POST, buff);
		size = gfal_plugin_mock_get_size(buff);
		break;
	}

	if (size >= 0) buf->st_size = size;

	// let's prepare for the next stat operation
	order++;

	// since it's a mock transfer anyway let's say the stat was successful
	// unless we would like to emulate the file is not there
	return 0;
}

gboolean gfal_plugin_mock_check_url_transfer(plugin_handle handle, gfal2_context_t ctx, const char* src, const char* dst, gfal_url2_check type) {
    gboolean res = FALSE;

    if( src != NULL && dst != NULL){
        if (type == GFAL_FILE_COPY && is_mock_uri(src) && is_mock_uri(dst))
            res = TRUE;
    }
    return res;
}

gboolean gfal_plugin_mock_checksum_verify(const char* src_chk, const char* dst_chk, const char* user_defined_chk, GError** err)
{
	// if no checksum was defined return
	if (*user_defined_chk == '\0' && *src_chk == '\0' && *dst_chk == '\0') return TRUE;
	// if user checksum was not defined compare source and destination
    if (*user_defined_chk == '\0')
    {
    	if (strcmp(src_chk, dst_chk) != 0)
    	{
			gfal_plugin_mock_report_error("SRC and DST checksum are different.", err);
			return FALSE;
    	}
    	// source and destination checksums match
    	return TRUE;
    }
    // if user and source were defined ...
	if (*src_chk != '\0' && strcmp(src_chk, user_defined_chk) != 0)
	{
		gfal_plugin_mock_report_error("USER_DEFINE and SRC checksums are different.", err);
		return FALSE;
	}
	// compare user and destination
	if (strcmp(dst_chk, user_defined_chk) != 0)
	{
		gfal_plugin_mock_report_error("USER_DEFINE and DST checksums are different.", err);
		return FALSE;
	}

	return TRUE;
}

int gfal_plugin_mock_filecopy(plugin_handle handle, gfal2_context_t context, gfalt_params_t params, const char* src, const char* dst, GError** err){

	// do use checksum
	gboolean checksum_check = gfalt_get_checksum_check(params, NULL);
	// do we care about source checksum
	gboolean skip_source_checksum = gfal2_get_opt_boolean(
			context,
			mock_config_group,
			mock_skip_transfer_config,
			NULL
		);

	// handle checksumming if necessary
	if (checksum_check)
	{
		char checksum_type[GFAL_URL_MAX_LEN] = { 0 };
	    char checksum_usr[GFAL_URL_MAX_LEN]  = { 0 };
	    char checksum_src[GFAL_URL_MAX_LEN]  = { 0 };
	    char checksum_dst[GFAL_URL_MAX_LEN]  = { 0 };

		// user defined checksum first
		gfalt_get_user_defined_checksum(
				params,
				checksum_type,
				sizeof(checksum_type),
				checksum_usr,
				sizeof(checksum_usr),
				NULL
			);
		// than the source
		if (!skip_source_checksum) gfal_plugin_mock_get_value(src, CHECKSUM, checksum_src);
		// and than the destination
		gfal_plugin_mock_get_value(dst, CHECKSUM, checksum_dst);
		// and finally verify if they are OK
		if (!gfal_plugin_mock_checksum_verify(checksum_src, checksum_dst, checksum_usr, err)) return -1;
	}

	// transfer duration
	int seconds = 0;

	// check if the duration is specified in destination
	char time_dst[GFAL_URL_MAX_LEN]  = { 0 };
	gfal_plugin_mock_get_value(dst, TIME, time_dst);

	if (time_dst[0] != '\0')
	{
		// get the value from destination
		seconds = atoi(time_dst);
	}
	else
	{
		// get the range from configuration file
		int max = gfal2_get_opt_integer_with_default(context, mock_config_group, MAX_TRANSFER_TIME, 100);
		int min = gfal2_get_opt_integer_with_default(context, mock_config_group, MIN_TRANSFER_TIME, 10);
		// determin the duration
		if (max == min) seconds = max;
		else seconds = rand() % (max - min) + min;
	}

	// mock transfer duration
	sleep(seconds);

	return 0;
}

/*
 * Init function, called before all
 * */
gfal_plugin_interface gfal_plugin_init(gfal2_context_t handle, GError** err){

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



