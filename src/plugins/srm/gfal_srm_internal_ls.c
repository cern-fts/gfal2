/*
 * Copyright (c) CERN 2013-2017
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

#include "gfal_srm_internal_ls.h"
#include "gfal_srm_endpoint.h"


/*
 * clear memory used by the internal srm_ifce items
 * required by the old design of srm_ifce
 *
 * */
void gfal_srm_ls_memory_management(struct srm_ls_input *input, struct srm_ls_output *output)
{
    if (input) {
        // nothing

    }
    if (output) {
        gfal_srm_external_call.srm_srmv2_mdfilestatus_delete(output->statuses, 1);
        gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output->retstatus);

    }
}

/*
 * Utility function to convert SRM File Locality to string.
 */
void gfal_srm_status_copy(TFileLocality loc, char *buff, size_t s_buff)
{
    char *org_string;
    switch (loc) {
        case GFAL_LOCALITY_ONLINE_:
            org_string = GFAL_XATTR_STATUS_ONLINE;
            break;
        case GFAL_LOCALITY_LOST:
            org_string = GFAL_XATTR_STATUS_LOST;
            break;
        case GFAL_LOCALITY_NEARLINE_:
            org_string = GFAL_XATTR_STATUS_NEARLINE;
            break;
        case GFAL_LOCALITY_UNAVAILABLE:
            org_string = GFAL_XATTR_STATUS_UNAVAILABLE;
            break;
        case GFAL_LOCALITY_ONLINE_USCOREAND_USCORENEARLINE:
            org_string = GFAL_XATTR_STATUS_NEARLINE_ONLINE;
            break;
        case GFAL_LOCALITY_NONE_:
            org_string = GFAL_XATTR_STATUS_NONE;
            break;
        default:
            org_string = GFAL_XATTR_STATUS_UNKNOWN;
            break;
    }
    g_strlcpy(buff, org_string, s_buff);
}

/*
 *  concentrate the srm_ls logical in one point for stat, readdir, status, and access
 *
 * */
static int gfal_srm_ls_internal(srm_context_t context,
    struct srm_ls_input *input, struct srm_ls_output *output,
    GError **err)
{
    GError *tmp_err = NULL;
    int ret = -1;

    if ((ret = gfal_srm_external_call.srm_ls(context, input, output)) < 0) {
        gfal_srm_report_error(context->errbuf, &tmp_err);
        ret = -1;
    }

    G_RETURN_ERR(ret, tmp_err, err);
}


static time_t gfal_srm_local_timezone_offset(void)
{
    time_t epoch = 0;
    return -mktime(gmtime(&epoch));
}


static void gfal_srm_adjust_time(struct stat *buf)
{
    tzset();
    time_t tz_offset = gfal_srm_local_timezone_offset();

    // Do not apply offset if the value is 0
    if (buf->st_ctime != 0)
        buf->st_ctime += tz_offset;
    if (buf->st_atime != 0)
        buf->st_atime += tz_offset;
    if (buf->st_mtime != 0)
        buf->st_mtime += tz_offset;
}


int gfal_statG_srmv2__generic_internal(srm_context_t context, struct stat *buf, TFileLocality *loc,
    const char *surl, GError **err)
{
    g_return_val_err_if_fail(context && surl
                             && buf && (sizeof(struct stat) == sizeof(struct stat64)),
        -1, err, "[gfal_statG_srmv2_generic_internal] Invalid args handle/endpoint or invalid stat struct size");
    GError *tmp_err = NULL;
    struct srm_ls_input input;
    struct srm_ls_output output;
    struct srmv2_mdfilestatus *srmv2_mdstatuses = NULL;
    const int nb_request = 1;
    int ret = -1;
    char *tab_surl[] = {(char *) surl, NULL};

    input.nbfiles = nb_request;
    input.surls = tab_surl;
    input.numlevels = 0;
    input.offset = 0;
    input.count = 0;

    ret = gfal_srm_ls_internal(context, &input, &output, &tmp_err);

    if (ret >= 0) {
        srmv2_mdstatuses = output.statuses;
        if (srmv2_mdstatuses->status != 0) {
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), srmv2_mdstatuses->status, __func__,
                "Error reported from srm_ifce : %d %s",
                srmv2_mdstatuses->status, srmv2_mdstatuses->explanation);
            ret = -1;
        } else {
            memcpy(buf, &(srmv2_mdstatuses->stat), sizeof(struct stat));
            if (loc)
                *loc = srmv2_mdstatuses->locality;
            errno = 0;
            ret = 0;
            // SRM returns the time in UTC
            gfal_srm_adjust_time(buf);
        }
    }
    gfal_srm_ls_memory_management(&input, &output);

    G_RETURN_ERR(ret, tmp_err, err);
}

int gfal_srm_cache_stat_add(plugin_handle ch, const char *surl, const struct stat *value, const TFileLocality *loc)
{
    char buff_key[GFAL_URL_MAX_LEN];
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;
    gfal_srm_construct_key(surl, GFAL_SRM_LSTAT_PREFIX, buff_key, GFAL_URL_MAX_LEN);

    struct extended_stat xstat;
    xstat.stat = *value;
    xstat.locality = *loc;

    gsimplecache_add_item_kstr(opts->cache, buff_key, &xstat);
    return 0;
}

void gfal_srm_cache_stat_remove(plugin_handle ch, const char *surl)
{
    char buff_key[GFAL_URL_MAX_LEN];
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) ch;
    gfal_srm_construct_key(surl, GFAL_SRM_LSTAT_PREFIX, buff_key, GFAL_URL_MAX_LEN);
    gsimplecache_remove_kstr(opts->cache, buff_key);
}
