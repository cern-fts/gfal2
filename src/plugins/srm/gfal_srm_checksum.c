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
 * @file gfal_srm_checksum.c
 * @brief funtion to get the checksum of a file
 * @author Devresse Adrien
 * @version 2.0
 * @date 29/09/2011
 * */

#include "gfal_srm_internal_layer.h"
#include "gfal_srm_getput.h"
#include "gfal_srm_url_check.h"

static int gfal_checksumG_srmv2_internal(srm_context_t context, const char* surl,
        char* buf_checksum, size_t s_checksum, char* buf_chktype, size_t s_chktype, GError** err)
{
	g_return_val_err_if_fail(context && surl && buf_checksum && buf_chktype,
                             -1, err, "[gfal_checksumG_srmv2_internal] Invalid input parameters : endpoint, surl, checksum, checksum_type");
	GError* tmp_err=NULL;
	struct srm_ls_input input;
	struct srm_ls_output output;
	struct srmv2_mdfilestatus *srmv2_mdstatuses=NULL;
	const int nb_request=1;
	int ret=-1;
	char* tab_surl[] = { (char*)surl, NULL};

    input.nbfiles = nb_request;
    input.surls = tab_surl;
    input.numlevels = 0;
    input.offset = 0;
    input.count = 0;

    ret = gfal_srm_external_call.srm_ls(context,&input,&output);

    if(ret >=0){
        srmv2_mdstatuses = output.statuses;
        if(srmv2_mdstatuses->status != 0){
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), srmv2_mdstatuses->status, __func__,
                    "Error reported from srm_ifce : %d %s", srmv2_mdstatuses->status, srmv2_mdstatuses->explanation);
            ret = -1;
        }else{
            if(srmv2_mdstatuses->checksum && srmv2_mdstatuses->checksumtype){
                g_strlcpy(buf_checksum, srmv2_mdstatuses->checksum, s_checksum);
                g_strlcpy(buf_chktype, srmv2_mdstatuses->checksumtype, s_chktype);
            }else{
                if(s_checksum > 0)
                    buf_checksum='\0';
                if(s_chktype > 0)
                    buf_chktype ='\0';
            }
            ret = 0;
        }
    }else{
        gfal_srm_report_error(context->errbuf, &tmp_err);
        ret=-1;
    }
    gfal_srm_external_call.srm_srmv2_mdfilestatus_delete(srmv2_mdstatuses, 1);
    gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);

    G_RETURN_ERR(ret, tmp_err, err);
}

/*
 * get checksum from a remote SRM URL
 *
 * */
static int gfal_srm_cheksumG_internal(plugin_handle ch, const char* surl,
											char* buf_checksum, size_t s_checksum,
											char* buf_chktype, size_t s_chktype, GError** err)
{
    g_return_val_err_if_fail(ch && surl, EINVAL, err, "[gfal_srm_cheksumG_internal] Invalid value handle, surl or buffers");
    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;

    int ret = -1;

    srm_context_t context = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (context != NULL) {
        ret = gfal_checksumG_srmv2_internal(context, surl, buf_checksum, s_checksum, buf_chktype, s_chktype, &tmp_err);
    }
    gfal_srm_ifce_easy_context_release(opts, context);

    if (ret != 0)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    return ret;
}


int gfal_srm_checksumG_fallback(plugin_handle handle, const char* url, const char* check_type,
                       char * checksum_buffer, size_t buffer_length,
                       off_t start_offset, size_t data_length,
                       gboolean turl_fallback,
                       GError ** err){
    gfal_log(GFAL_VERBOSE_TRACE, " [gfal_srm_checksumG] ->");
    gfal_log(GFAL_VERBOSE_DEBUG, "[gfal_srm_checksumG] try to get checksum %s for %s", check_type, url);

    char buffer_type[GFAL_URL_MAX_LEN]={0};
    GError * tmp_err=NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt*)handle;
    const gboolean srm_url = srm_check_url(url);
    int res =  -1;

    // try SRM checksum only if full file checksum is requested
    if(srm_url && start_offset==0 && data_length==0 ) {
        res = gfal_srm_cheksumG_internal(handle, url,
                                   checksum_buffer, buffer_length,
                                   buffer_type, GFAL_URL_MAX_LEN, &tmp_err);
    }

    // Make sure the returned type matches the requested one
    if(res == 0) {
        gfal_log(GFAL_VERBOSE_DEBUG, "registered checksum type %s", buffer_type);
        if(strncasecmp(check_type, buffer_type,GFAL_URL_MAX_LEN) != 0){
            // does not match the correct type
            // this can be because checksum is populated on DPM server, cause the first gsiftp checksum calculation
            res = -1; // cancel result
        }
    }



    // If we got no error, but neither a valid checksum,
    // fallback into the turl
    if(res != 0 && !tmp_err && turl_fallback){
        gfal_log(GFAL_VERBOSE_TRACE, "\t\tNo valid SRM checksum, fallback to the TURL checksum");
        char buff_turl[GFAL_URL_MAX_LEN];
        char *res_turl;
        if(srm_url){ // SRM URL do TURL resolution
            if( (res = gfal_srm_getTURL_checksum(handle, url, buff_turl, GFAL_URL_MAX_LEN,  &tmp_err)) >= 0){
                 res_turl = buff_turl;
            }else{
                res = -1;
            }
        }else{ // native protocol -> act like this
            res_turl = (char*)url;
            res =0;
        }
        if(res == 0){
          gfal_log(GFAL_VERBOSE_TRACE, "\t\t\tExecute checksum on turl %s", res_turl);
          res= gfal2_checksum(opts->handle, res_turl, check_type, 0,0, checksum_buffer, buffer_length, &tmp_err);
        }

    }
    // If no fallback, then return an empty value
    else if (!turl_fallback && (tmp_err || res  != 0)) {
        res = 0;
        memset(checksum_buffer, '\0', buffer_length);
    }

    G_RETURN_ERR(res, tmp_err, err);
}


// Wrapper for gfal_srm_checksumG_fallback so it matches
// the expected gfal2 signature
int gfal_srm_checksumG(plugin_handle handle, const char* url, const char* check_type,
                       char * checksum_buffer, size_t buffer_length,
                       off_t start_offset, size_t data_length,
                       GError ** err)
{
    return gfal_srm_checksumG_fallback(handle, url,
            check_type, checksum_buffer, buffer_length,
            start_offset, data_length,
            TRUE,
            err);
}
