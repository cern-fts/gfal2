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
 * @file gfal_common_srm_copy.c
 * @brief file for the third party transfer implementation
 * @author Devresse Adrien
 * */
#include <omp.h>

#include <common/gfal_types.h>
#include <common/gfal_common_errverbose.h>
#include <transfer/gfal_transfer.h>
#include <externals/utils/uri_util.h>

#include "gfal_common_srm_getput.h"
#include "gfal_common_srm_stat.h"
#include "gfal_common_srm_url_check.h"
#include "gfal_common_srm_internal_layer.h"
#include "gfal_common_srm_checksum.h"

GQuark srm_quark_3rd_party(){
    return g_quark_from_static_string("srm_plugin::filecopy");
}

int srm_plugin_get_3rdparty(plugin_handle handle, gfalt_params_t params, const char * surl,
                            char* buff, size_t s_buff,
                            GError ** err){
	GError * tmp_err=NULL;
	int res = -1;
	if( srm_check_url(surl) ){
        gfal_log(GFAL_VERBOSE_TRACE, "\t\tGET surl -> turl dst resolution start");
        if( (res =gfal_srm_get_rd3_turl(handle, params, surl, buff , s_buff, NULL,  err)) == 0){
            gfal_log(GFAL_VERBOSE_TRACE, "\t\tGET surl -> turl dst resolution ended : %s -> %s", surl, buff);
		}
	}else{
		res =0;
		g_strlcpy(buff, surl, s_buff);
        gfal_log(GFAL_VERBOSE_TRACE, "\t\tno SRM resolution needed on %s", surl);
	}	
	G_RETURN_ERR(res, tmp_err, err);		
}

int srm_plugin_delete_existing_copy(plugin_handle handle, gfalt_params_t params, 
									const char * surl, GError ** err){
	GError * tmp_err=NULL;
	int res = 0;
	const gboolean replace = gfalt_get_replace_existing_file(params, NULL);
	if(replace){
		if( (res = gfal_srm_unlinkG(handle, surl, &tmp_err)) ==0){
			gfal_log(GFAL_VERBOSE_TRACE, "   %s found, delete in order to replace it", surl);		
		}else{
			if(tmp_err && tmp_err->code == ENOENT){
				gfal_log(GFAL_VERBOSE_TRACE, "   %s does not exist, begin copy", surl);			
				g_clear_error(&tmp_err);
				res =0;
			}
			
		}
		
	}

	G_RETURN_ERR(res, tmp_err, err);		
}

int srm_plugin_put_3rdparty(plugin_handle handle, gfal2_context_t context,
					gfalt_params_t params,  const char * surl, 
					char* buff, size_t s_buff, 
					char** reqtoken, GError ** err){
	GError * tmp_err=NULL;
	int res = -1;
	
	if( srm_check_url(surl)){
        gfal_log(GFAL_VERBOSE_TRACE, "\t\tPUT surl -> turl src resolution start ");
		if( (res = srm_plugin_delete_existing_copy(handle, params, surl, &tmp_err)) ==0){						
            if(( res= gfal_srm_put_rd3_turl(handle, params, surl, buff , s_buff, reqtoken,  err))==0)
                gfal_log(GFAL_VERBOSE_TRACE, "\t\tPUT surl -> turl src resolution ended : %s -> %s", surl, buff);
		}
	}else{
        res =1;
		g_strlcpy(buff, surl, s_buff);
		gfal_log(GFAL_VERBOSE_TRACE, "		no SRM resolution needed on %s", surl);			
	}
	G_RETURN_ERR(res, tmp_err, err);	
}

int srm_plugin_check_checksum(plugin_handle handle, gfal2_context_t context,
                              gfalt_params_t params,
                              const char* src, char* buff_chk, GError ** err){

    char buff_user_defined[GFAL_URL_MAX_LEN]={0};
    char buff_user_defined_type[GFAL_URL_MAX_LEN]={0};
    char * chk_type=NULL;
    GError * tmp_err=NULL;
    int res=0;

    if(gfalt_get_checksum_check(params, &tmp_err)){
       gfal_log(GFAL_VERBOSE_TRACE,"\t\tCompute SRM checksum for %s",src);
       gfalt_get_user_defined_checksum(params, buff_user_defined_type, GFAL_URL_MAX_LEN,
                                        buff_user_defined, GFAL_URL_MAX_LEN, NULL); // fetch the user defined chk
       const gboolean user_defined = (*buff_user_defined!='\0' && *buff_user_defined_type!='\0');

       if(!user_defined){
           chk_type=  gfal2_get_opt_string(context, srm_config_group,srm_config_transfer_checksum,&tmp_err);
           gfal_log(GFAL_VERBOSE_TRACE, "\t\tNo checksum type defined by user, take it from configuration : %s", chk_type);
       }else{
           chk_type=g_strdup(buff_user_defined_type);
       }

       if( chk_type && (res = gfal_srm_checksumG(handle, src, chk_type,
                              buff_chk, GFAL_URL_MAX_LEN,
                              0, 0,
                              &tmp_err))==0){
           if(user_defined && strncasecmp(buff_user_defined,buff_chk,GFAL_URL_MAX_LEN ) != 0){
               g_set_error(&tmp_err, srm_quark_3rd_party(),EIO, "Checksum of %s and user defined checksum does not match %s %s",
                           src, buff_chk, buff_user_defined);
           }
       }
       g_free(chk_type);

    }
    G_RETURN_ERR(res, tmp_err, err);
}


int srm_compare_checksum_transfer(gfalt_params_t params, const char* src, const char* dst,
                                  char* src_buff_checksum,
                                  char* dst_buff_checksum, GError** err){
    int res = 0;

    if(gfalt_get_checksum_check(params, err)){
        if( strncasecmp(src_buff_checksum, dst_buff_checksum,GFAL_URL_MAX_LEN) !=0){
            g_set_error(err, srm_quark_3rd_party(),EIO, "Checksum of %s and %s does not match %s %s",
                        src, dst, src_buff_checksum, dst_buff_checksum);
            res =-1;
        }else{
            res =0;
        }
    }
    return res;
}

int plugin_filecopy(plugin_handle handle, gfal2_context_t context,
					gfalt_params_t params, 
					const char* src, const char* dst, GError ** err){
	g_return_val_err_if_fail( handle != NULL && src != NULL
			&& dst != NULL , -1, err, "[plugin_filecopy][gridftp] einval params");	
	
	gfal_log(GFAL_VERBOSE_TRACE, "  -> [srm_plugin_filecopy] ");
    GError * tmp_err=NULL;
    int res = -1;
    gboolean put_waiting= FALSE;
	char buff_turl_src[GFAL_URL_MAX_LEN];
    char buff_src_checksum[GFAL_URL_MAX_LEN];
	char buff_turl_dst[GFAL_URL_MAX_LEN];
    char buff_dst_checksum[GFAL_URL_MAX_LEN];
	char* reqtoken = NULL;
    gfalt_params_t params_turl = gfalt_params_handle_copy(params, &tmp_err);  // create underlying protocol parameters
    gfalt_set_checksum_check(params_turl, FALSE,NULL); // disable already does actions

    GError * tmp_err_get, *tmp_err_put,*tmp_err_chk_src;
    tmp_err_chk_src= tmp_err_get = tmp_err_put = NULL;

    #pragma omp parallel num_threads(3)
    {

         #pragma omp sections
        {
            #pragma omp section
            {
                srm_plugin_check_checksum(handle, context, params, src, buff_src_checksum, &tmp_err_chk_src);
            }
            #pragma omp section
            {
                srm_plugin_get_3rdparty(handle, params, src, buff_turl_src, GFAL_URL_MAX_LEN, &tmp_err_get);
            }
            #pragma omp section
            {
                int ret_put = srm_plugin_put_3rdparty(handle, context, params, dst, buff_turl_dst, GFAL_URL_MAX_LEN, &reqtoken, &tmp_err_put);
                if(!tmp_err_put && reqtoken != NULL)
                    put_waiting = TRUE;
                if(ret_put == 0){ // srm resolution done to turl, do not check dest -> already done
                    gfalt_set_replace_existing_file(params_turl,FALSE, NULL);
                    gfalt_set_strict_copy_mode(params_turl, TRUE, NULL);
                }
            }
        }

    }

   if( !gfal_error_keep_first_err(&tmp_err, &tmp_err_get, &tmp_err_chk_src, &tmp_err_put,NULL) ){ // do the first resolution

            if(!tmp_err){
                res = gfalt_copy_file(context, params_turl, buff_turl_src, buff_turl_dst, &tmp_err);
                if( res == 0 && put_waiting){
                    gfal_log(GFAL_VERBOSE_TRACE, "\ttransfer executed, execute srm put done"); // commit transaction

                    res= gfal_srm_putdone_simple(handle, dst, reqtoken, &tmp_err);
                    if(res ==0){
                        put_waiting = FALSE;
                        if( (res = srm_plugin_check_checksum(handle, context, params, dst, buff_dst_checksum, &tmp_err)) ==0 ){  // try to get resu checksum
                            res= srm_compare_checksum_transfer(params, src, dst,
                                                              buff_src_checksum,
                                                              buff_dst_checksum, &tmp_err);
                        }
                    }
                }

            }
    }

    if(put_waiting){ // abort request
           gfal_log(GFAL_VERBOSE_TRACE, "\tCancel PUT request for %s", dst);
           GError * tmp_err_cancel=NULL;
           srm_abort_request_plugin(handle, dst, reqtoken, &tmp_err_cancel);
           // log silent error
           if(tmp_err_cancel)
               gfal_log(GFAL_VERBOSE_DEBUG, " Error while canceling put on %s: %s", dst, tmp_err_cancel->message);
          // clear the trash file silently
           gfal_srm_unlinkG(handle, dst,NULL);
    }

    gfalt_params_handle_delete(params_turl, NULL);
	gfal_log(GFAL_VERBOSE_TRACE, " [srm_plugin_filecopy] <-");	
	G_RETURN_ERR(res, tmp_err, err);		
}


int srm_plugin_filecopy(plugin_handle handle, gfal2_context_t context,
                    gfalt_params_t params,
                    const char* src, const char* dst, GError ** err){
    return plugin_filecopy(handle, context, params, src, dst, err);
}
