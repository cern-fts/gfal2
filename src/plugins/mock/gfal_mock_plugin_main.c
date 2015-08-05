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

#include <glib.h>
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

#include <gfal_plugins_api.h>
#include <transfer/gfal_transfer.h>
#include <common/gfal_common_err_helpers.h>
#include <checksums/checksums.h>

const char* mock_config_group = "MOCK PLUGIN";
const char* mock_skip_transfer_config = "SKIP_SOURCE_CHECKSUM";

const char* MAX_TRANSFER_TIME = "MAX_TRANSFER_TIME";
const char* MIN_TRANSFER_TIME = "MIN_TRANSFER_TIME";

const char* FILE_SIZE = "size";
const char* FILE_SIZE_PRE = "size_pre";
const char* FILE_SIZE_POST = "size_post";
const char* CHECKSUM = "checksum";
const char* TIME = "time";
const char* ERRNO = "errno";
const char* TRANSFER_ERRNO = "transfer_errno";
const char* STAGING_TIME = "staging_time";
const char* STAGING_ERRNO = "staging_errno";
const char* RELEASE_ERRNO = "release_errno";

// This is the order FTS3 performs its stats
typedef enum {
	STAT_SOURCE = 0,
	STAT_DESTINATION_BEFORE_TRANSFER,
	STAT_DESTINATION_AFTER_TRANSFER
} StatStage;


typedef struct {
    StatStage stat_stage;
    time_t staging_end;
    int staging_errno;
    int release_errno;
} MockPluginData;


GQuark gfal2_get_plugin_mock_quark()
{
    return g_quark_from_static_string(GFAL2_QUARK_PLUGINS "::FILE");
}


static gboolean is_mock_uri(const char* src)
{
    return strncmp(src, "mock:", 5) == 0;
}


const char* gfal_mock_plugin_getName()
{
    return GFAL2_PLUGIN_VERSIONED("mock", VERSION);
}


static gboolean gfal_mock_check_url(plugin_handle handle, const char* url, plugin_mode mode, GError** err)
{
    g_return_val_err_if_fail(url != NULL, EINVAL, err, "[gfal_lfile_path_checker] Invalid url ");

	switch(mode){
        //case GFAL_PLUGIN_ACCESS:
        //case GFAL_PLUGIN_MKDIR:
		case GFAL_PLUGIN_STAT:
		case GFAL_PLUGIN_LSTAT:
		//case GFAL_PLUGIN_RMDIR:
		//case GFAL_PLUGIN_OPENDIR:
		//case GFAL_PLUGIN_OPEN:
		//case GFAL_PLUGIN_CHMOD:
		case GFAL_PLUGIN_UNLINK:
		//case GFAL_PLUGIN_GETXATTR:
		//case GFAL_PLUGIN_LISTXATTR:
        //case GFAL_PLUGIN_SETXATTR:
        //case GFAL_PLUGIN_RENAME:
        //case GFAL_PLUGIN_SYMLINK:
        //case GFAL_PLUGIN_CHECKSUM:
		case GFAL_PLUGIN_BRING_ONLINE:
            return is_mock_uri(url);
		default:
			return FALSE;
	}
}

void gfal_plugin_mock_report_error(const char* msg, int errn, GError** err)
{
    g_set_error(err, gfal2_get_plugin_mock_quark(), errn, "%s", msg);
}


void gfal_plugin_mock_get_value(const char* url, const char* key, char* value, size_t val_size)
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
		int size = (end - str) + 1;
		g_strlcpy(value, str, size < val_size ? size : val_size);
	}
	else
	{
		// if it is the last parameter just copy the string until it ends
	    g_strlcpy(value, str, val_size);
	}
}


static int gfal_plugin_mock_get_int_from_str(const char* buff)
{
	if (buff == 0 || buff[0] == '\0')
	    return 0;
	return atoi(buff);
}


int gfal_plugin_mock_stat(plugin_handle plugin_data, const char* path, struct stat* buf, GError** err)
{
    char arg_buffer[64] = {0};
    int size = 0, errcode = 0;

    // Check errno first
    gfal_plugin_mock_get_value(path, ERRNO, arg_buffer, sizeof(arg_buffer));
    errcode = gfal_plugin_mock_get_int_from_str(arg_buffer);
    if (errcode > 0) {
        gfal_plugin_mock_report_error(strerror(errcode), errcode, err);
        return -1;
    }

    // Default size first
    gfal_plugin_mock_get_value(path, FILE_SIZE, arg_buffer, sizeof(arg_buffer));
    size = gfal_plugin_mock_get_int_from_str(arg_buffer);

    // Try specific stage then
    MockPluginData* mdata = plugin_data;
    switch (mdata->stat_stage) {
        case STAT_DESTINATION_BEFORE_TRANSFER:
            gfal_plugin_mock_get_value(path, FILE_SIZE_PRE, arg_buffer, sizeof(arg_buffer));
            size = gfal_plugin_mock_get_int_from_str(arg_buffer);
            // If this size were <= 0, consider it a ENOENT
            if (size <= 0) {
                gfal_plugin_mock_report_error(strerror(ENOENT), ENOENT, err);
                mdata->stat_stage++;
                return -1;
            }
            break;
        case STAT_DESTINATION_AFTER_TRANSFER:
            gfal_plugin_mock_get_value(path, FILE_SIZE_POST, arg_buffer, sizeof(arg_buffer));
            size = gfal_plugin_mock_get_int_from_str(arg_buffer);
            break;
        default:
            break;
    }
    mdata->stat_stage++;

    // Set the struct
    memset(buf, 0x00, sizeof(*buf));
    buf->st_size = size;
    buf->st_mode = 0755;

    return 0;
}


int gfal_plugin_mock_unlink(plugin_handle plugin_data, const char* url, GError** err)
{
    struct stat buf;
    if (gfal_plugin_mock_stat(plugin_data, url, &buf, err) < 0)
        return -1;
    return 0;
}


gboolean gfal_plugin_mock_check_url_transfer(plugin_handle handle, gfal2_context_t ctx, const char* src, const char* dst, gfal_url2_check type)
{
    gboolean res = FALSE;
    if (src != NULL && dst != NULL) {
        if (type == GFAL_FILE_COPY && is_mock_uri(src) && is_mock_uri(dst)) {
            res = TRUE;
        }
    }
    return res;
}


gboolean gfal_plugin_mock_checksum_verify(const char* chk1, const char* chk2)
{
	// if no checksum was defined return
	if (!chk1 || !chk2 || !*chk1 || !*chk2)
	    return TRUE;

	return strcmp(chk1, chk2) == 0;
}


static void gfal_mock_cancel_transfer(gfal2_context_t context, void* userdata)
{
    int *seconds = (int*)userdata;
    *seconds = 0;
}


int gfal_plugin_mock_filecopy(plugin_handle plugin_data,
    gfal2_context_t context, gfalt_params_t params, const char* src,
    const char* dst, GError** err)
{
	// do we use checksum
	gboolean checksum_check = gfalt_get_checksum_check(params, NULL);

	// do we care about source checksum
	gboolean skip_source_checksum = gfal2_get_opt_boolean(
			context,
			mock_config_group,
			mock_skip_transfer_config,
			NULL
		);

    // user defined checksum
    char checksum_type[GFAL_URL_MAX_LEN] = { 0 };
    char checksum_usr[GFAL_URL_MAX_LEN]  = { 0 };
    char checksum_src[GFAL_URL_MAX_LEN]  = { 0 };

    gfalt_get_user_defined_checksum(params,
            checksum_type, sizeof(checksum_type),
            checksum_usr, sizeof(checksum_usr),
            NULL);

	// validate source checksum
	if (checksum_check && !skip_source_checksum)
	{
	    gfal_plugin_mock_get_value(src, CHECKSUM, checksum_src, sizeof(checksum_src));
		if (!gfal_plugin_mock_checksum_verify(checksum_usr, checksum_src)) {
		    gfal_plugin_mock_report_error("User and source checksums do not match", EIO, err);
		    return -1;
		}
	}

	// transfer duration
	int seconds = 0;

	// check if the duration is specified in destination
	char time_dst[GFAL_URL_MAX_LEN]  = { 0 };
	gfal_plugin_mock_get_value(dst, TIME, time_dst, sizeof(time_dst));

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
		// determine the duration
		if (max == min) seconds = max;
		else seconds = rand() % (max - min) + min;
	}

	// Trigger an error on the transfer?
	char transfer_errno_buffer[64] = {0};
	gfal_plugin_mock_get_value(dst, TRANSFER_ERRNO, transfer_errno_buffer, sizeof(transfer_errno_buffer));
	int transfer_errno = gfal_plugin_mock_get_int_from_str(transfer_errno_buffer);

	// mock transfer duration
	gfal_cancel_token_t cancel_token;
	cancel_token = gfal2_register_cancel_callback(context,
	        gfal_mock_cancel_transfer, &seconds);


    plugin_trigger_event(params, gfal2_get_plugin_mock_quark(), GFAL_EVENT_NONE,
            GFAL_EVENT_TRANSFER_ENTER, "Mock copy start, sleep %d", seconds);
    while (seconds > 0) {
        sleep(1);
        --seconds;

        // Fail here
        if (transfer_errno) {
            gfal_plugin_mock_report_error(strerror(transfer_errno), transfer_errno, err);
            break;
        }
    }
    plugin_trigger_event(params, gfal2_get_plugin_mock_quark(), GFAL_EVENT_NONE,
            GFAL_EVENT_TRANSFER_EXIT, "Mock copy start, sleep %d", seconds);

    gfal2_remove_cancel_callback(context, cancel_token);

    // Jump over to the destination stat
    MockPluginData* mdata = plugin_data;
    mdata->stat_stage = STAT_DESTINATION_AFTER_TRANSFER;

    // validate destination checksum
    if (!*err && checksum_check)
    {
        char checksum_dst[GFAL_URL_MAX_LEN]  = { 0 };
        gfal_plugin_mock_get_value(dst, CHECKSUM, checksum_dst, sizeof(checksum_dst));

        if (skip_source_checksum) {
            if (!gfal_plugin_mock_checksum_verify(checksum_usr, checksum_dst)) {
                gfal_plugin_mock_report_error("User and destination checksums do not match", EIO, err);
            }
        }
        else {
            if (!gfal_plugin_mock_checksum_verify(checksum_src, checksum_dst)) {
                gfal_plugin_mock_report_error("Source and destination checksums do not match", EIO, err);
            }
        }
    }

    if (*err)
        return -1;
    return 0;
}


int gfal_plugin_mock_bring_online(plugin_handle plugin_data, const char* url,
    time_t pintime, time_t timeout, char* token, size_t tsize, int async,
    GError** err)
{
    struct stat buf;
    if (gfal_plugin_mock_stat(plugin_data, url, &buf, err)) {
        return -1;
    }

    MockPluginData *mdata = plugin_data;
    char arg_buffer[64];

    // Bring online errno
    gfal_plugin_mock_get_value(url, STAGING_ERRNO, arg_buffer, sizeof(arg_buffer));
    mdata->staging_errno = gfal_plugin_mock_get_int_from_str(arg_buffer);

    // Release errno
    gfal_plugin_mock_get_value(url, RELEASE_ERRNO, arg_buffer, sizeof(arg_buffer));
    mdata->release_errno = gfal_plugin_mock_get_int_from_str(arg_buffer);

    // Polling time
    gfal_plugin_mock_get_value(url, STAGING_TIME, arg_buffer, sizeof(arg_buffer));
    mdata->staging_end = time(NULL) + gfal_plugin_mock_get_int_from_str(arg_buffer);

    // Fake token
    g_strlcpy(token, "mock-token", tsize);

    // Now, if remaining is <= 0, or blocking call, we are done
    if (mdata->staging_end <= time(NULL) || !async) {
        if (mdata->staging_errno) {
            gfal_plugin_mock_report_error(strerror(mdata->staging_errno), mdata->staging_errno, err);
            return -1;
        }
        return 1;
    }

    return 0;
}


int gfal_plugin_mock_bring_online_poll(plugin_handle plugin_data,
    const char* url, const char* token, GError** err)
{
    MockPluginData *mdata = plugin_data;

    if (mdata->staging_end <= time(NULL)) {
        if (mdata->staging_errno) {
            gfal_plugin_mock_report_error(strerror(mdata->staging_errno), mdata->staging_errno, err);
            return -1;
        }
        return 1;
    }

    return 0;
}



int gfal_plugin_mock_release_file(plugin_handle plugin_data, const char* url,
    const char* token, GError** err)
{
    MockPluginData *mdata = plugin_data;
    if (mdata->release_errno) {
        gfal_plugin_mock_report_error(strerror(mdata->release_errno), mdata->release_errno, err);
        return -1;
    }
    return 0;
}


void gfal_plugin_mock_delete(plugin_handle plugin_data)
{
    free(plugin_data);
}


/*
 * Init function, called before all
 * */
gfal_plugin_interface gfal_plugin_init(gfal2_context_t handle, GError** err)
{
	gfal_plugin_interface mock_plugin;
    memset(&mock_plugin,0,sizeof(gfal_plugin_interface));	// clear the plugin

    mock_plugin.plugin_data = calloc(1, sizeof(MockPluginData));
    mock_plugin.plugin_delete = gfal_plugin_mock_delete;
    mock_plugin.check_plugin_url = &gfal_mock_check_url;
    mock_plugin.getName= &gfal_mock_plugin_getName;

    mock_plugin.statG = &gfal_plugin_mock_stat;
    mock_plugin.lstatG = &gfal_plugin_mock_stat;
    mock_plugin.unlinkG = &gfal_plugin_mock_unlink;

    mock_plugin.bring_online = gfal_plugin_mock_bring_online;
    mock_plugin.bring_online_poll = gfal_plugin_mock_bring_online_poll;
    mock_plugin.release_file = gfal_plugin_mock_release_file;

    mock_plugin.check_plugin_url_transfer = &gfal_plugin_mock_check_url_transfer;
    mock_plugin.copy_file = &gfal_plugin_mock_filecopy;

	return mock_plugin;
}



