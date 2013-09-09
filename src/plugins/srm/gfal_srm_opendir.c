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
 * @file gfal_srm_opendir.c
 * @brief file for the opendir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 09/06/2011
 * */




#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include "gfal_srm.h"
#include "gfal_srm_opendir.h"
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_filedescriptor.h>
#include "gfal_srm_stat.h"
#include "gfal_srm_endpoint.h"
#include "gfal_srm_internal_layer.h"





gfal_file_handle gfal_srm_opendir_internal(gfal_srmv2_opt* opts, char* endpoint,
        const char* surl, GError** err)
{
    g_return_val_err_if_fail(opts && endpoint && surl, NULL, err,
            "[gfal_srmv2_opendir_internal] invalid args");
    GError* tmp_err = NULL;
    gfal_file_handle resu = NULL;
    struct stat st;
    int exist = gfal_statG_srmv2_internal(opts, &st, endpoint, surl, &tmp_err);

    if (exist == 0) {
        if (S_ISDIR(st.st_mode)) {
            gfal_srm_opendir_handle h =
                    g_new0(struct _gfal_srm_opendir_handle,1);
            const size_t s = strnlen(surl, GFAL_URL_MAX_LEN);
            char* p = (char*) mempcpy(h->surl, surl, MIN(s,GFAL_URL_MAX_LEN));
            // remove trailing '/'
            for (--p; *p == '/'; --p)
                *p = '\0';

            g_strlcpy(h->endpoint, endpoint, GFAL_URL_MAX_LEN);
            h->dir_offset = 0;
            resu = gfal_file_handle_new2(gfal_srm_getName(), (gpointer) h, NULL,
                    surl);
        }
        else {
            g_set_error(&tmp_err, 0, ENOTDIR,
                    "srm-plugin: %s is not a directory, impossible to list content",
                    surl);
        }
    }

    if (tmp_err)
        g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);

    return resu;
}
	


gfal_file_handle gfal_srm_opendirG(plugin_handle ch, const char* surl, GError ** err){
	g_return_val_err_if_fail(ch && surl, NULL, err, "[gfal_srm_opendirG] Invalid args");
	gfal_srmv2_opt* opts = (gfal_srmv2_opt*) ch;
	gfal_file_handle resu = NULL;
	char endpoint[GFAL_URL_MAX_LEN];
	GError* tmp_err=NULL;
	int ret = -1;
	enum gfal_srm_proto srm_type;
	
	ret = gfal_srm_determine_endpoint(opts, surl, endpoint, GFAL_URL_MAX_LEN, &srm_type,  &tmp_err);
	if( ret >=0 ){
		if(srm_type == PROTO_SRMv2){
			resu = gfal_srm_opendir_internal(opts, endpoint, surl, &tmp_err);
		}else if (srm_type == PROTO_SRM){
			g_set_error(&tmp_err, 0, EPROTONOSUPPORT, "support for SRMv1 is removed in 2.0, failure");
			resu = NULL;
		}else {
			g_set_error(&tmp_err, 0, EPROTONOSUPPORT, "Unknow version of the protocol SRM , failure");
			resu = NULL;			
		}
		
	}
	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
	return resu;
}


int gfal_srm_closedirG(plugin_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail(handle && fh, -1, err, "[gfal_srm_opendirG] Invalid args");
	gfal_srm_opendir_handle oh = (gfal_srm_opendir_handle) fh->fdesc;	
	//gfal_srm_external_call.srm_srmv2_mdfilestatus_delete(oh->srm_ls_resu, 1); --> disable because of error in memory management in srm-ifce
	g_free(oh);
    gfal_file_handle_delete(fh);
	return 0;
}
