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
 * @file gfal_srm_stat.c
 * @brief file for the stat function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 16/05/2011
 * */

#include "gfal_srm.h"
#include "gfal_srm_internal_ls.h"
#include "gfal_srm_namespace.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_endpoint.h"


int gfal_statG_srmv2_internal(srm_context_t context, struct stat* buf, TFileLocality* loc, const char* surl, GError** err)
{
    return gfal_statG_srmv2__generic_internal(context, buf, loc, surl, err);
}

/*
 * stat call, for the srm interface stat and lstat are the same call !! the default behavior is similar to stat by default and ignore links
 *
 * */
int gfal_srm_statG(plugin_handle ch, const char* surl, struct stat* buf, GError** err)
{
	g_return_val_err_if_fail( ch && surl && buf, -1, err, "[gfal_srm_statG] Invalid args in handle/surl/buf");
	GError* tmp_err = NULL;
	int ret =-1;
	char key_buff[GFAL_URL_MAX_LEN];
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;
    TFileLocality loc;
    struct extended_stat xstat;

	// Try cache first
	gfal_srm_construct_key(surl, GFAL_SRM_LSTAT_PREFIX, key_buff, GFAL_URL_MAX_LEN);
	if (gsimplecache_take_one_kstr(opts->cache, key_buff, &xstat) == 0) {
        gfal2_log(G_LOG_LEVEL_DEBUG,
                " srm_statG -> value taken from the cache");
        ret = 0;
        *buf = xstat.stat;
    }
	// Ask server otherwise
    else {
        srm_context_t context = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
        if (context != NULL) {
            gfal2_log(G_LOG_LEVEL_INFO, "   [gfal_srm_statG] try to stat file %s", surl);
            ret = gfal_statG_srmv2_internal(context, buf, &loc, surl, &tmp_err);
            if (ret == 0) {
                gfal2_log(G_LOG_LEVEL_DEBUG, "   [gfal_srm_statG] store %s stat info in cache", surl);
                gfal_srm_cache_stat_add(ch, surl, buf, &loc);
            }
        }
        else {
            ret = -1;
        }
        gfal_srm_ifce_easy_context_release(opts, context);
    }

    if(tmp_err)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}
