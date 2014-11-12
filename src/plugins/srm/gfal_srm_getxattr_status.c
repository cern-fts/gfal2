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
 * @file gfal_srm_getxattr_status.c
 * @brief file for the getxattr function  for status ( ONLINE, ... ) )on the srm url type
 * @author Devresse Adrien
 * @date 02/08/2011
 * */

#include <string.h>

#include "gfal_srm.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_namespace.h"
#include "gfal_srm_endpoint.h"
#include "gfal_srm_internal_ls.h"

void gfal_srm_status_copy(TFileLocality loc, char* buff, size_t s_buff){
	char * org_string;
	switch(loc){
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
		default:
			org_string = GFAL_XATTR_STATUS_UNKNOW;
			break;
	}
	g_strlcpy(buff, org_string, s_buff);
}


ssize_t gfal_srm_status_internal(gfal_srmv2_opt* opts, srm_context_t context, const char* path,
        void* buff, size_t s_buff, GError** err)
{
	GError* tmp_err=NULL;
	ssize_t ret = -1;
	struct extended_stat buf;
	char key_buff[GFAL_URL_MAX_LEN];

	gfal_srm_construct_key(path, GFAL_SRM_LSTAT_PREFIX, key_buff, GFAL_URL_MAX_LEN);
    if (gsimplecache_take_one_kstr(opts->cache, key_buff, &buf) == 0) {
        gfal_log(GFAL_VERBOSE_DEBUG, " gfal_srm_status_internal -> value taken from the cache");
        ret = 0;
    }
    else {
        ret = gfal_statG_srmv2__generic_internal(context, &buf.stat, &buf.locality, path, &tmp_err);
    }

    if (ret >= 0) {
        gfal_srm_status_copy(buf.locality, (char*) buff, s_buff);
        ret = MIN( strlen(buff), s_buff);
    }

	G_RETURN_ERR(ret, tmp_err, err);
}

/*
 * main implementation of the srm status -> getxattr
 */
ssize_t gfal_srm_status_getxattrG(plugin_handle handle, const char* surl, const char* name , void* buff, size_t s_buff, GError** err)
{
    g_return_val_err_if_fail(handle && surl, EINVAL, err, "[gfal_srm_status_getxattrG] Invalid value handle and/or surl");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) handle;

    int ret = -1;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (context != NULL) {
        ret = gfal_srm_status_internal(opts, context, surl, buff, s_buff, &tmp_err);
    }
    gfal_srm_ifce_easy_context_release(opts, context);

    if (ret < 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}
