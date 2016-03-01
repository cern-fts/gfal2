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

#include "gfal_mock_plugin.h"
#include <string.h>


static gboolean gfal_plugin_mock_checksum_verify(const char *chk1, const char *chk2)
{
    // if no checksum was defined return
    if (!chk1 || !chk2 || !*chk1 || !*chk2)
        return TRUE;

    return strcmp(chk1, chk2) == 0;
}


static void gfal_mock_cancel_transfer(gfal2_context_t context, void *userdata)
{
    int *seconds = (int *) userdata;
    *seconds = -10;
}


int gfal_plugin_mock_filecopy(plugin_handle plugin_data,
    gfal2_context_t context, gfalt_params_t params, const char *src,
    const char *dst, GError **err)
{
    // do we use checksum
    gboolean checksum_check = gfalt_get_checksum_check(params, NULL);

    // do we care about source checksum
    gboolean skip_source_checksum = gfal2_get_opt_boolean(
        context,
        "MOCK PLUGIN",
        "SKIP_SOURCE_CHECKSUM",
        NULL
    );

    // user defined checksum
    char checksum_type[GFAL_URL_MAX_LEN] = {0};
    char checksum_usr[GFAL_URL_MAX_LEN] = {0};
    char checksum_src[GFAL_URL_MAX_LEN] = {0};

    gfalt_get_user_defined_checksum(params,
        checksum_type, sizeof(checksum_type),
        checksum_usr, sizeof(checksum_usr),
        NULL);

    // validate source checksum
    if (checksum_check && !skip_source_checksum) {
        gfal_plugin_mock_get_value(src, "checksum", checksum_src, sizeof(checksum_src));
        if (!gfal_plugin_mock_checksum_verify(checksum_usr, checksum_src)) {
            gfal_plugin_mock_report_error("User and source checksums do not match", EIO, err);
            return -1;
        }
    }

    // transfer duration
    int seconds = 0;

    // check if the duration is specified in destination
    char time_dst[GFAL_URL_MAX_LEN] = {0};
    gfal_plugin_mock_get_value(dst, "time", time_dst, sizeof(time_dst));

    if (time_dst[0] != '\0') {
        // get the value from destination
        seconds = atoi(time_dst);
    }
    else {
        // get the range from configuration file
        int max = gfal2_get_opt_integer_with_default(context, "MOCK PLUGIN", "MAX_TRANSFER_TIME", 100);
        int min = gfal2_get_opt_integer_with_default(context, "MOCK PLUGIN", "MIN_TRANSFER_TIME", 10);
        // determine the duration
        if (max == min) seconds = max;
        else seconds = rand() % (max - min) + min;
    }

    // Trigger an error on the transfer?
    char transfer_errno_buffer[64] = {0};
    gfal_plugin_mock_get_value(dst, "transfer_errno", transfer_errno_buffer, sizeof(transfer_errno_buffer));
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

    // Canceled?
    if (seconds < 0) {
        gfal_plugin_mock_report_error("Transfer canceled", ECANCELED, err);
        return -1;
    }

    // Jump over to the destination stat
    MockPluginData *mdata = plugin_data;
    mdata->stat_stage = STAT_DESTINATION_AFTER_TRANSFER;

    // validate destination checksum
    if (!*err && checksum_check) {
        char checksum_dst[GFAL_URL_MAX_LEN] = {0};
        gfal_plugin_mock_get_value(dst, "checksum", checksum_dst, sizeof(checksum_dst));

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
