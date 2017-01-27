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

#include <string.h>

#include "gfal_srm.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_namespace.h"
#include "gfal_srm_space.h"


#define GFAL_XATTR_SRM_TYPE "srm.type"

static char *srm_listxattr[] = {SRM_XATTR_GETURL, GFAL_XATTR_STATUS, GFAL_XATTR_SRM_TYPE, GFAL_XATTR_SPACETOKEN, NULL};


static ssize_t gfal_srm_get_endpoint_type_xattrG(plugin_handle handle, const char *path,
    const char *name, void *buff, size_t s_buff, GError **err)
{
    GError *tmp_err = NULL;

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(handle, path, &tmp_err);
    if (!easy) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }

    struct srm_xping_output output;
    if (gfal_srm_external_call.srm_xping(easy->srm_context, &output) < 0) {
        gfal2_set_error(err, gfal2_get_plugin_srm_quark(), errno, __func__,
            "Could not get the storage type");
        return -1;
    }

    memset(buff, 0, s_buff);
    int i;
    for (i = 0; i < output.n_extra; ++i) {
        if (strcmp(output.extra[i].key, "backend_type") == 0) {
            g_strlcpy(buff, output.extra[i].value, s_buff);
            break;
        }
    }
    srm_xping_output_free(output);
    gfal_srm_ifce_easy_context_release(handle, easy);
    return strnlen(buff, s_buff);
}


ssize_t gfal_srm_geturl_getxattrG(plugin_handle handle, const char *path,
    const char *name, void *buff, size_t s_buff, GError **err)
{
    GError *tmp_err = NULL;
    ssize_t ret = -1;
    if (s_buff == 0 || buff == NULL)
        return GFAL_URL_MAX_LEN;

    ret = gfal_srm_getTURLS_plugin(handle, path, buff, s_buff, NULL, &tmp_err);
    if (ret >= 0) {
        ret = strnlen(buff, s_buff) * sizeof(char);
    }

    G_RETURN_ERR(ret, tmp_err, err);
}


/*
 * implementation of the getxattr for turl resolution, pin management and spacetoken set/get
 *
 * */
ssize_t gfal_srm_getxattrG(plugin_handle handle, const char *path,
    const char *name, void *buff, size_t s_buff, GError **err)
{
    GError *tmp_err = NULL;
    ssize_t ret = -1;
    gfal2_log(G_LOG_LEVEL_DEBUG, " gfal_srm_getxattrG ->");
    if (strcmp(name, SRM_XATTR_GETURL) == 0) {
        ret = gfal_srm_geturl_getxattrG(handle, path, name, buff, s_buff,
            &tmp_err);
    }
    else if (strcmp(name, GFAL_XATTR_STATUS) == 0) {
        ret = gfal_srm_status_getxattrG(handle, path, name, buff, s_buff,
            &tmp_err);
    }
    else if (strcmp(name, GFAL_XATTR_SRM_TYPE) == 0) {
        ret = gfal_srm_get_endpoint_type_xattrG(handle, path, name, buff, s_buff, err);
    }
    else if (strncmp(name, GFAL_XATTR_SPACETOKEN, 10) == 0) {
        return gfal_srm_space_getxattrG(handle, path, name, buff, s_buff, err);
    }
    else {
        gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), ENOATTR,
            __func__, "not an existing extended attribute");
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, " gfal_srm_getxattrG <- ");
    G_RETURN_ERR(ret, tmp_err, err);
}


/*
 * lfc getxattr implem
 * */
ssize_t gfal_srm_listxattrG(plugin_handle handle, const char *path, char *list, size_t size, GError **err)
{
    ssize_t res = 0;
    char **p = srm_listxattr;
    char *plist = list;
    while (*p != NULL) {
        const int size_str = strlen(*p) + 1;
        if (size > res && size - res >= size_str) {
            memcpy(plist, *p, size_str);
            plist += size_str;
        }
        res += size_str;
        p++;
    }
    return res;
}
