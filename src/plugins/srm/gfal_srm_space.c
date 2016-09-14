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

#include <inttypes.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include "gfal_srm_space.h"

#include "space/gfal2_space.h"


static void json_putc(char *buff, size_t s_buff, char c, size_t *offset)
{
    if (*offset < s_buff)
        buff[(*offset)++] = c;
}


static void json_puts(char *buff, size_t s_buff, const char *str, size_t *offset)
{
    json_putc(buff, s_buff, '"', offset);
    const char *p = str;
    while (*p != '\0') {
        if (*p == '\\') {
            json_putc(buff, s_buff, '\\', offset);
            json_putc(buff, s_buff, '\\', offset);
        }
        else if (*p == '"') {
            json_putc(buff, s_buff, '\\', offset);
            json_putc(buff, s_buff, '"', offset);
        }
        else {
            json_putc(buff, s_buff, *p, offset);
        }
        ++p;
    }
    json_putc(buff, s_buff, '"', offset);
}


static ssize_t gfal_srm_space_list(srm_context_t context,
    char *buff, size_t s_buff, GError **err)
{
    GError *tmp_err = NULL;
    struct srm_getspacetokens_input input;
    struct srm_getspacetokens_output output;
    ssize_t ret_size = 0;

    input.spacetokendesc = NULL;

    if (gfal_srm_external_call.srm_getspacetokens(context, &input, &output) < 0) {
        gfal_srm_report_error(context->errbuf, &tmp_err);
        if (tmp_err->code == EINVAL && !strstr(tmp_err->message, "[EINVAL] Invalid arguments")) {
            // This means there is no space token that belongs to the user, so we can just return empty
            g_error_free(tmp_err);
            tmp_err = NULL;
        }
        else {
            ret_size = -1;
        }
    }
    else {
        int i;
        size_t offset = 0;
        json_putc(buff, s_buff, '[', &offset);
        for (i = 0; i < output.nbtokens; ++i) {
            json_puts(buff, s_buff, output.spacetokens[i], &offset);
            json_putc(buff, s_buff, ',', &offset);
        }
        if (buff[offset - 1] == ',')
            --offset; // Strip last comma
        json_putc(buff, s_buff, ']', &offset);
        json_putc(buff, s_buff, '\0', &offset);
        ret_size = offset;
    }

    if (tmp_err != NULL)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    return ret_size;
}


static ssize_t gfal_srm_space_token_info(srm_context_t context, const char *token,
    char *buff, size_t s_buff, GError **err)
{
    GError *tmp_err = NULL;
    struct srm_getspacemd_input input;
    struct srm_spacemd *spaces = NULL;
    ssize_t ret_size = 0;
    char *spacetokens[] = {(char *) token, NULL};

    input.nbtokens = 1;
    input.spacetokens = spacetokens;

    if (gfal_srm_external_call.srm_getspacemd(context, &input, &spaces) < 0) {
        gfal_srm_report_error(context->errbuf, &tmp_err);
        ret_size = -1;
    }
    else {
        struct space_report report = {0};

        uint64_t guaranteed = (uint64_t)spaces[0].guaranteedsize;

        report.spacetoken = spaces[0].spacetoken;
        report.owner = spaces[0].owner;
        report.total = (uint64_t)spaces[0].totalsize;
        report.largest_chunk = &guaranteed;
        report.free = (uint64_t)spaces[0].unusedsize;
        report.lifetime_assigned = &spaces[0].lifetimeassigned;
        report.lifetime_left = &spaces[0].lifetimeleft;
        report.retention = (enum space_retention_policy)spaces[0].retentionpolicy;
        report.latency = (enum space_latency)spaces[0].accesslatency;

        ret_size = gfal2_space_generate_json(&report, buff, s_buff);
    }

    if (tmp_err != NULL)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    return ret_size;
}


static ssize_t gfal_srm_space_token_descr_info(srm_context_t context,
    const char *token_desc, char *buff, size_t s_buff, GError **err)
{
    GError *tmp_err = NULL;
    struct srm_getspacetokens_input input;
    struct srm_getspacetokens_output output;
    ssize_t ret_size = 0;

    input.spacetokendesc = (char *) token_desc;

    if (gfal_srm_external_call.srm_getspacetokens(context, &input, &output) < 0) {
        gfal_srm_report_error(context->errbuf, &tmp_err);
        ret_size = -1;
    }
    else {
        int i;
        size_t offset = 0;
        json_putc(buff, s_buff, '[', &offset);
        for (i = 0; i < output.nbtokens; ++i) {
            ssize_t s = gfal_srm_space_token_info(context, output.spacetokens[i],
                buff + offset, s_buff - offset, &tmp_err);
            if (s < 0) {
                ret_size = -1;
                break;
            }
            offset += (s - 1); // String \0
            json_putc(buff, s_buff, ',', &offset);
        }
        if (ret_size >= 0) {
            if (buff[offset - 1] == ',')
                --offset; // Strip last comma
            json_putc(buff, s_buff, ']', &offset);
            json_putc(buff, s_buff, '\0', &offset);
            ret_size = offset;
        }
    }

    if (tmp_err != NULL)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    return ret_size;
}


static ssize_t gfal_srm_space_property(srm_context_t context, const char *name,
    char *buff, size_t s_buff, GError **err)
{
    if (name[0] == '\0') {
        return gfal_srm_space_list(context, buff, s_buff, err);
    }
    else if (strncmp(name, "token?", 6) == 0) {
        return gfal_srm_space_token_info(context, name + 6, buff, s_buff, err);
    }
    else if (strncmp(name, "description?", 12) == 0) {
        return gfal_srm_space_token_descr_info(context, name + 12, buff, s_buff, err);
    }
    else {
        gfal2_set_error(err, gfal2_get_plugin_srm_quark(), ENOATTR, __func__,
            "Unknown space token attribute %s", name);
        return -1;
    }
}


ssize_t gfal_srm_space_getxattrG(plugin_handle handle, const char *path,
    const char *name, void *buff, size_t s_buff, GError **err)
{
    if (strncmp(name, GFAL_XATTR_SPACETOKEN, 10) != 0) {
        gfal2_set_error(err, gfal2_get_plugin_srm_quark(), ENOATTR,
            __func__, "Unknown attribute %s", name);
        return -1;
    }
    const char *subprop_name = name + 10;
    if (*subprop_name == '.') {
        ++subprop_name;
    }
    else if (subprop_name[0] != '\0') {
        gfal2_set_error(err, gfal2_get_plugin_srm_quark(), ENOATTR,
            __func__, "Unknown space token attribute %s", name);
        return -1;
    }

    GError *nested_error = NULL;
    ssize_t ret_size = 0;

    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) handle;
    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, path, &nested_error);
    if (easy) {
        ret_size = gfal_srm_space_property(easy->srm_context, subprop_name, (char *) buff, s_buff, &nested_error);
    }
    gfal_srm_ifce_easy_context_release(opts, easy);

    if (nested_error != NULL)
        gfal2_propagate_prefixed_error(err, nested_error, __func__);
    return ret_size;
}
