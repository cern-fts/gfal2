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
#include <uri/gfal2_uri.h>

#include "gfal_srm_url_check.h"

const char * surl_prefix = GFAL_PREFIX_SRM;


gboolean srm_check_url(const char * surl){
    gboolean res = FALSE;
    const size_t prefix_len = strlen(surl_prefix);
    size_t surl_len = strnlen(surl, GFAL_URL_MAX_LEN);
    if ((surl_len < GFAL_URL_MAX_LEN)
            && (strncmp(surl, surl_prefix, prefix_len) == 0)) {
        res = TRUE;
    }
    return res;
}

static gboolean srm_has_schema(const char * surl){
    return strchr(surl, ':') != NULL;
}


gboolean plugin_url_check2(plugin_handle handle, gfal2_context_t context,
        const char* src, const char* dst, gfal_url2_check type)
{
    g_return_val_if_fail(handle != NULL && src != NULL && dst != NULL, FALSE);
    gboolean src_srm = srm_check_url(src);
    gboolean dst_srm = srm_check_url(dst);
    gboolean src_valid_url = src_srm || srm_has_schema(src);
    gboolean dst_valid_url = dst_srm || srm_has_schema(dst);

    return (type == GFAL_FILE_COPY && ((src_srm && dst_valid_url) || (dst_srm && src_valid_url)));
}


static char* gfal2_srm_surl_find_path(gfal2_uri* parsed)
{
    char *path;
    if (parsed->query != NULL && (path = strstr(parsed->query, "SFN=")) != NULL) {
        path += 4;
    } else {
        path = parsed->path;
    }
    return path;
}

char *gfal2_srm_get_decoded_path(const char *surl)
{
    GError *err = NULL;
    gfal2_uri *parsed = gfal2_parse_uri(surl, &err);
    if (err != NULL) {
        g_clear_error(&err);
        return g_strdup(surl);
    }
    char *path = gfal2_srm_surl_find_path(parsed);
    gfal2_urldecode(path);

    char *decoded = g_strconcat("srm://", parsed->host, path, NULL);

    gfal2_free_uri(parsed);
    return decoded;
}


int gfal2_srm_surl_cmp(const char* surl1, const char* surl2)
{
    int cmp;

    GError* error = NULL;
    gfal2_uri *parsed1, *parsed2;

    // Parse urls
    parsed1 = gfal2_parse_uri(surl1, &error);
    if (error)
        goto srm_surl_cmp_fallback;
    parsed2 = gfal2_parse_uri(surl2, &error);
    if (error)
        goto srm_surl_cmp_fallback;

    // If hosts are different, surls are different
    if (strcmp(parsed1->host, parsed2->host) != 0 || parsed1->port != parsed2->port) {
        cmp = -1;
        goto srm_surl_cmp_done;
    }

    // If no SFN is found, the path is as-is
    // Otherwise, the path is whatever is found in the SFN
    const char* sfn1 = gfal2_srm_surl_find_path(parsed1);
    const char* sfn2 = gfal2_srm_surl_find_path(parsed2);

    cmp = strcmp(sfn1, sfn2);
    goto srm_surl_cmp_done;

    // Fallback to raw strcmp
srm_surl_cmp_fallback:
    g_error_free(error);
    cmp = strcmp(surl1, surl2);

srm_surl_cmp_done:
    gfal2_free_uri(parsed1);
    gfal2_free_uri(parsed2);
    return cmp;
}
