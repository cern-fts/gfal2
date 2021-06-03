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

#include <string.h>

#include "gfal_srm.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_namespace.h"
#include "gfal_srm_endpoint.h"
#include "gfal_srm_internal_ls.h"


ssize_t gfal_srm_status_internal(gfal_srmv2_opt *opts, srm_context_t context, const char *path,
    void *buff, size_t s_buff, GError **err)
{
    GError *tmp_err = NULL;
    ssize_t ret = -1;
    struct extended_stat buf;
    char key_buff[GFAL_URL_MAX_LEN];

    gfal_srm_construct_key(path, GFAL_SRM_LSTAT_PREFIX, key_buff, GFAL_URL_MAX_LEN);
    if (gsimplecache_take_one_kstr(opts->cache, key_buff, &buf) == 0) {
        gfal2_log(G_LOG_LEVEL_DEBUG, " gfal_srm_status_internal -> value taken from the cache");
        ret = 0;
    }
    else {
        ret = gfal_statG_srmv2__generic_internal(context, &buf.stat, &buf.locality, path, &tmp_err);
    }

    if (ret >= 0) {
        gfal_srm_status_copy(buf.locality, (char *) buff, s_buff);
        ret = strnlen(buff, s_buff);
    }

    G_RETURN_ERR(ret, tmp_err, err);
}

/*
 * main implementation of the srm status -> getxattr
 */
ssize_t gfal_srm_status_getxattrG(plugin_handle handle, const char *surl, const char *name, void *buff, size_t s_buff,
    GError **err)
{
    g_return_val_err_if_fail(handle && surl, EINVAL, err,
        "[gfal_srm_status_getxattrG] Invalid value handle and/or surl");
    GError *tmp_err = NULL;
    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) handle;

    int ret = -1;

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (easy != NULL) {
        ret = gfal_srm_status_internal(opts, easy->srm_context, easy->path, buff, s_buff, &tmp_err);
    }
    gfal_srm_ifce_easy_context_release(opts, easy);

    if (ret < 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}
