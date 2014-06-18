#include <inttypes.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <attr/xattr.h>
#include "gfal_srm_space.h"

static void json_putc(char* buff, size_t s_buff, char c, size_t* offset)
{
    if (*offset < s_buff)
        buff[(*offset)++] = c;
}


static void json_puts(char* buff, size_t s_buff, const char* str, size_t* offset)
{
    json_putc(buff, s_buff, '"', offset);
    const char* p = str;
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


static void json_put_null(char* buff, size_t s_buff, size_t* offset)
{
    json_putc(buff, s_buff, 'n', offset);
    json_putc(buff, s_buff, 'u', offset);
    json_putc(buff, s_buff, 'l', offset);
    json_putc(buff, s_buff, 'l', offset);
}


static void json_putattrs(char* buff, size_t s_buff, const char* attr, const char* value, size_t* offset)
{
    json_puts(buff, s_buff, attr, offset);
    json_putc(buff, s_buff, ':', offset);
    if (value)
        json_puts(buff, s_buff, value, offset);
    else
        json_put_null(buff, s_buff, offset);
}


static void json_putattri(char* buff, size_t s_buff, const char* attr, int64_t value, size_t* offset)
{
    json_puts(buff, s_buff, attr, offset);
    json_putc(buff, s_buff, ':', offset);
    char buffer[128];
    sprintf(buffer, "%"PRId64, value);
    char* p;
    for (p = buffer; *p != '\0'; ++p) {
        json_putc(buff, s_buff, *p, offset);
    }
}


static ssize_t gfal_srm_space_list(srm_context_t context,
        char* buff, size_t s_buff, GError** err)
{
    GError* tmp_err = NULL;
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


static const char* retention2str(TRetentionPolicy retentionpolicy)
{
    switch(retentionpolicy) {
        case GFAL_POLICY_REPLICA:
            return "REPLICA";
        case GFAL_POLICY_OUTPUT:
            return "OUTPUT";
        case GFAL_POLICY_CUSTODIAL:
            return "CUSTODIAL";
        default:
            return "UNKNOWN";
    }
}

static const char* accesslatency2str(TAccessLatency accesslatency)
{
    switch(accesslatency) {
        case GFAL_LATENCY_ONLINE:
            return "ONLINE";
        case GFAL_LATENCY_NEARLINE:
            return "NEARLINE";
        default:
            return "UNKNOWN";
    }
}


static ssize_t gfal_srm_space_token_info(srm_context_t context, const char* token,
        char* buff, size_t s_buff, GError** err)
{
    GError* tmp_err = NULL;
    struct srm_getspacemd_input input;
    struct srm_spacemd *spaces = NULL;
    ssize_t ret_size = 0;
    char* spacetokens[] = {(char*)token, NULL};

    input.nbtokens = 1;
    input.spacetokens = spacetokens;

    if (gfal_srm_external_call.srm_getspacemd(context, &input, &spaces) < 0) {
        gfal_srm_report_error(context->errbuf, &tmp_err);
        ret_size = -1;
    }
    else {
        size_t offset = 0;
        json_putc(buff, s_buff, '{', &offset);
        json_putattrs(buff, s_buff, "spacetoken", spaces[0].spacetoken, &offset);
        json_putc(buff, s_buff, ',', &offset);
        json_putattrs(buff, s_buff, "owner", spaces[0].owner, &offset);
        json_putc(buff, s_buff, ',', &offset);
        json_putattri(buff, s_buff, "totalsize", spaces[0].totalsize, &offset);
        json_putc(buff, s_buff, ',', &offset);
        json_putattri(buff, s_buff, "guaranteedsize", spaces[0].guaranteedsize, &offset);
        json_putc(buff, s_buff, ',', &offset);
        json_putattri(buff, s_buff, "unusedsize", spaces[0].unusedsize, &offset);
        json_putc(buff, s_buff, ',', &offset);
        json_putattri(buff, s_buff, "lifetimeassigned", spaces[0].lifetimeassigned, &offset);
        json_putc(buff, s_buff, ',', &offset);
        json_putattri(buff, s_buff, "lifetimeleft", spaces[0].lifetimeleft, &offset);
        json_putc(buff, s_buff, ',', &offset);
        json_putattrs(buff, s_buff, "retentionpolicy", retention2str(spaces[0].retentionpolicy), &offset);
        json_putc(buff, s_buff, ',', &offset);
        json_putattrs(buff, s_buff, "accesslatency", accesslatency2str(spaces[0].accesslatency), &offset);

        json_putc(buff, s_buff, '}', &offset);
        json_putc(buff, s_buff, '\0', &offset);

        ret_size = offset;
    }

    if (tmp_err != NULL)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    return ret_size;
}


static ssize_t gfal_srm_space_token_descr_info(srm_context_t context,
        const char* token_desc, char* buff, size_t s_buff, GError** err)
{
    GError* tmp_err = NULL;
    struct srm_getspacetokens_input input;
    struct srm_getspacetokens_output output;
    ssize_t ret_size = 0;

    input.spacetokendesc = (char*)token_desc;

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


static ssize_t gfal_srm_space_property(srm_context_t context, const char* name,
        char* buff, size_t s_buff, GError** err)
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


ssize_t gfal_srm_space_getxattrG(plugin_handle handle, const char* path,
        const char* name, void* buff, size_t s_buff, GError** err)
{
    if (strncmp(name, "spacetoken", 10) != 0) {
        gfal2_set_error(err, gfal2_get_plugin_srm_quark(), ENOATTR,
                __func__, "Unknown attribute %s", name);
        return -1;
    }
    const char* subprop_name = name + 10;
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

    gfal_srmv2_opt* opts = (gfal_srmv2_opt*)handle;
    srm_context_t context = gfal_srm_ifce_easy_context(opts, path, &nested_error);
    if (context) {
        ret_size = gfal_srm_space_property(context, subprop_name, (char*)buff, s_buff, &nested_error);
        gfal_srm_ifce_context_release(context);
    }

    if (nested_error != NULL)
        gfal2_propagate_prefixed_error(err, nested_error, __func__);
    return ret_size;
}
