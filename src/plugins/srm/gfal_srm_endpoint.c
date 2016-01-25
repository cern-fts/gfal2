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

#include <time.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include <mds/gfal_mds.h>
#include <uri/gfal_uri.h>

#include "gfal_srm_url_check.h"
#include "gfal_srm_internal_layer.h"


static enum gfal_srm_proto gfal_proto_list_prefG[]= { PROTO_SRMv2, PROTO_SRM, PROTO_ERROR_UNKNOW };


// construct a default service endpoint format, Guessing that the service endpoint follows the default DPM/dCache convention
static int gfal_srm_guess_service_endpoint(gfal_srmv2_opt *opts, const char *surl, char *buff_endpoint, size_t s_buff,
    enum gfal_srm_proto *srm_type, GError **err)
{
    guint msize = 0;
    GError *tmp_err = NULL;
    int ret = 0;

    msize = g_strlcpy(buff_endpoint, GFAL_ENDPOINT_DEFAULT_PREFIX, s_buff);
    char *p, *org_p;
    p = org_p = ((char *) surl) + strlen(GFAL_PREFIX_SRM);
    const int surl_len = strlen(surl);
    while (p < surl + surl_len && *p != '/' && *p != '\0')
        p++;
    if (org_p + 1 > p || msize >= s_buff || p - org_p + msize + strlen(GFAL_DEFAULT_SERVICE_ENDPOINT_SUFFIX) > s_buff) {
        gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EINVAL, __func__,
            "Impossible to setup default service endpoint from %s : bad URI format", surl);
        ret = -1;
    } else {
        strncat(buff_endpoint, org_p, p - org_p);
        g_strlcat(buff_endpoint, GFAL_DEFAULT_SERVICE_ENDPOINT_SUFFIX, s_buff);
        *srm_type = opts->srm_proto_type;
        ret = 0;
    }

    G_RETURN_ERR(ret, tmp_err, err);
}


/*
 *  return TRUE if a full endpoint is contained in surl  else FALSE
 *
*/
static gboolean gfal_check_fullendpoint_in_surlG(gfal_srmv2_opt *opts, const char *surl, GError **err)
{
    const int ret = regexec(&(opts->rex_full), surl, 0, NULL, 0);
    return (ret == 0) ? TRUE : FALSE;
}


/*
 *  @brief create a full endpoint from a "full-surl"
 * */
static int gfal_get_fullendpointG(const char *surl, char *buff_endpoint, size_t s_buff, GError **err)
{
    char *sfn = strstr(surl, "?SFN=");
    g_return_val_err_if_fail(sfn, -1, err,
        "[gfal_get_fullendpoint] full surl must contain ?SFN= and a valid prefix, fatal error");

    // Endpoint length not counting the scheme
    size_t endpoint_len = (sfn - surl) - GFAL_PREFIX_SRM_LEN;
    // Endpoint length counting the httpg scheme
    size_t full_endpoint_len = endpoint_len + GFAL_ENDPOINT_DEFAULT_PREFIX_LEN;

    // buff_endpoint must have enough space to allocate the full_endpoint_len + '\0'
    if (s_buff > full_endpoint_len) {
        memcpy(buff_endpoint, GFAL_ENDPOINT_DEFAULT_PREFIX, GFAL_ENDPOINT_DEFAULT_PREFIX_LEN);
        g_strlcpy(buff_endpoint + GFAL_ENDPOINT_DEFAULT_PREFIX_LEN, surl + GFAL_PREFIX_SRM_LEN, endpoint_len + 1);
        return 0;
    }
    gfal2_set_error(err, gfal2_get_plugin_srm_quark(), ENOBUFS, __func__, "buffer too small");
    return -1;
}


/*
 * map a bdii se protocol type to a gfal protocol type
 */
static enum gfal_srm_proto gfal_convert_proto_from_bdii(const char *se_type_bdii)
{
    enum gfal_srm_proto resu;
    if (strcmp(se_type_bdii, "srm_v1") == 0) {
        resu = PROTO_SRM;
    } else if (strcmp(se_type_bdii, "srm_v2") == 0) {
        resu = PROTO_SRMv2;
    } else {
        resu = PROTO_ERROR_UNKNOW;
    }
    return resu;
}


/*
 * select the best protocol choice and the best endpoint choice  from a list of protocol and endpoints obtained by the bdii
 *
 */
static int gfal_select_best_protocol_and_endpointG(gfal_srmv2_opt *opts,
    char **tab_se_type, char **tab_endpoint, char *buff_endpoint,
    size_t s_buff, enum gfal_srm_proto *srm_type, GError **err)
{
    g_return_val_err_if_fail(opts && buff_endpoint && s_buff && srm_type && tab_se_type && tab_endpoint, -1, err,
        "[gfal_select_best_protocol_and_endpoint] Invalid value");
    char **pse = tab_se_type;
    enum gfal_srm_proto *p_pref = &(opts->srm_proto_type);
    while (*p_pref != PROTO_ERROR_UNKNOW) {
        while (*pse != NULL && *tab_endpoint != NULL) {
            if (*p_pref ==
                gfal_convert_proto_from_bdii(*pse)) { // test if the response is the actual preferred response
                g_strlcpy(buff_endpoint, *tab_endpoint, s_buff);
                *srm_type = *p_pref;
                return 0;
            }
            tab_endpoint++;
            pse++;
        }
        if (p_pref ==
            &(opts->srm_proto_type)) // switch desired proto to the list if the default choice is not in the list
            p_pref = gfal_proto_list_prefG;
        else
            p_pref++;
    }
    gfal2_set_error(err, gfal2_get_plugin_srm_quark(), EINVAL, __func__,
        "cannot obtain a valid protocol from the bdii response, fatal error");
    return -2;

}


/*
 * get endpoint from the bdii system only
 * 0 == success
 * < 0 error
 * > 0 : bddi disabled
 *
 * */
static int gfal_get_endpoint_and_setype_from_bdiiG(gfal_srmv2_opt *opts, const char *surl,
    char *buff_endpoint, size_t s_buff, enum gfal_srm_proto *srm_type, GError **err)
{
    g_return_val_err_if_fail(opts && buff_endpoint && srm_type && surl && s_buff, -1, err,
        "[gfal_get_endpoint_and_setype_from_bdiiG] invalid parameters");

    char **tab_endpoint = NULL;
    char **tab_se_type = NULL;
    char hostname[GFAL_URL_MAX_LEN];
    int ret = -1;
    GError *tmp_err = NULL;

    ret = gfal2_hostname_from_uri(surl, hostname, sizeof(hostname), &tmp_err);
    if (ret == 0) { // get the hostname

        if ((ret = gfal_mds_get_se_types_and_endpoints(opts->handle, hostname, &tab_se_type, &tab_endpoint,
            &tmp_err)) == 0) { // questioning the bdii
            ret = gfal_select_best_protocol_and_endpointG(opts, tab_se_type, tab_endpoint, buff_endpoint,
                GFAL_URL_MAX_LEN, srm_type, &tmp_err); // map the response if correct
            g_strfreev(tab_endpoint);
            g_strfreev(tab_se_type);
        }
    }
    G_RETURN_ERR(ret, tmp_err, err);
}


int gfal_srm_determine_endpoint(gfal_srmv2_opt *opts, const char *surl,
    char *buff_endpoint, size_t s_buff, enum gfal_srm_proto *srm_type,
    GError **err)
{
    g_return_val_err_if_fail(
        opts && buff_endpoint && srm_type && surl && s_buff, -1, err,
        "[gfal_srm_determine_endpoint] invalid value in params"); // check params

    GError *tmp_err = NULL;
    int ret = -1;
    gboolean isFullEndpoint = gfal_check_fullendpoint_in_surlG(opts, surl, &tmp_err); // check if a full endpoint exist
    if (!tmp_err) {
        if (isFullEndpoint == TRUE) { // if full endpoint contained in url, get it and set type to default type
            if (gfal_get_fullendpointG(surl, buff_endpoint, s_buff, &tmp_err) == 0) {
                *srm_type = opts->srm_proto_type;
                ret = 0;
                gfal2_log(G_LOG_LEVEL_DEBUG,
                    "Service endpoint resolution, resolved from FULL SURL %s -> %s",
                    surl, buff_endpoint);
            }
        }
        else {
            if (gfal_get_nobdiiG(opts->handle) == TRUE ||
                ((ret = gfal_get_endpoint_and_setype_from_bdiiG(opts, surl, buff_endpoint, s_buff, srm_type,
                    &tmp_err)) != 0)) {
                if (tmp_err) {
                    gfal2_log(G_LOG_LEVEL_WARNING,
                        "Error while bdii SRM service resolution : %s, fallback on the default service path."
                        "This can lead to wrong service path, you should use FULL SURL format or register your endpoint into the BDII",
                        tmp_err->message);
                    g_clear_error(&tmp_err);
                }
                else {
                    gfal2_log(G_LOG_LEVEL_WARNING,
                        "BDII usage disabled, fallback on the default service path."
                            "This can lead to wrong service path, you should use FULL SURL format or register your endpoint into the BDII");

                }
                ret = gfal_srm_guess_service_endpoint(opts, surl, buff_endpoint, s_buff, srm_type, &tmp_err);
                if (ret == 0) {
                    gfal2_log(G_LOG_LEVEL_DEBUG,
                        "Service endpoint resolution, set to default path %s -> %s",
                        surl, buff_endpoint);
                }
            }
            else {
                gfal2_log(G_LOG_LEVEL_DEBUG,
                    "Service endpoint resolution, resolved from BDII %s -> %s",
                    surl, buff_endpoint);

            }
        }

    }
    G_RETURN_ERR(ret, tmp_err, err);
}


int is_castor_endpoint(plugin_handle handle, const char* surl)
{
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*)handle;

    if (!srm_check_url(surl)) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Endpoint not SRM: %s", surl);
        return 0;
    }

    GError *tmp_err = NULL;
    srm_context_t context = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (tmp_err)
        g_error_free(tmp_err);
    if (!context) {
        gfal2_log(G_LOG_LEVEL_WARNING, "Could not get a context for %s", surl);
        return -1;
    }

    struct srm_xping_output output;
    if (gfal_srm_external_call.srm_xping(context, &output) < 0) {
        gfal2_log(G_LOG_LEVEL_WARNING, "Failed to ping %s", surl);
        gfal_srm_ifce_easy_context_release(opts, context);
        return -1;
    }

    int i, is_castor = 0;
    for (i = 0; i < output.n_extra && !is_castor; ++i) {
        if (strcmp(output.extra[i].key, "backend_type") == 0) {
            gfal2_log(G_LOG_LEVEL_MESSAGE, "Endpoint of type %s: %s", output.extra[i].value, surl);
            is_castor = (strcasecmp(output.extra[i].value, "CASTOR") == 0);
        }
    }
    srm_xping_output_free(output);
    gfal_srm_ifce_easy_context_release(opts, context);
    return is_castor;
}
