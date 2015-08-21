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
#include <uri/gfal_uri.h>

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
    return strstr(surl, ":/") != NULL;
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


static const char* gfal2_srm_surl_find_path(const gfal_uri* parsed)
{
    const char* SFN = strstr(parsed->query, "SFN=");
    if (!SFN)
        return parsed->path;
    SFN += 4;
    return SFN;
}


int gfal2_srm_surl_cmp(const char* surl1, const char* surl2)
{
    GError* error = NULL;
    gfal_uri parsed1, parsed2;

    // Parse urls
    gfal2_parse_uri(surl1, &parsed1, &error);
    if (error)
        goto srm_surl_cmp_fallback;
    gfal2_parse_uri(surl2, &parsed2, &error);
    if (error)
        goto srm_surl_cmp_fallback;

    // If hosts are different, surls are different
    if (strcmp(parsed1.domain, parsed2.domain) != 0 || parsed1.port != parsed2.port)
        return -1;

    // If no SFN is found, the path is as-is
    // Otherwise, the path is whatever is found in the SFN
    const char* sfn1 = gfal2_srm_surl_find_path(&parsed1);
    const char* sfn2 = gfal2_srm_surl_find_path(&parsed2);

    int cmp = strcmp(sfn1, sfn2);

    return cmp;

    // Fallback to raw strcmp
srm_surl_cmp_fallback:
    g_error_free(error);
    return strcmp(surl1, surl2);
}
