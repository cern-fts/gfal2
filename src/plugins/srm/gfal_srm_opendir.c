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
#include <common/gfal_common_err_helpers.h>
#include <common/gfal_common_filedescriptor.h>
#include "gfal_srm_stat.h"
#include "gfal_srm_endpoint.h"
#include "gfal_srm_internal_layer.h"


// Replaces the first occurence of ';' with '/0', so
// we have a valid surl. Returns a pointer
// to the beginning of the additional parameters, if any
static char* _strip_parameters(char* surl)
{
    char* first_semicolon = strchr(surl, ';');
    if (first_semicolon) {
        *first_semicolon = '\0';
        return first_semicolon + 1;
    }
    else {
        return NULL;
    }
}

// Parse any possible additional parameters passed in the URL and set the
// corresponding values in h
// parameters should be already in the form of key=value;key=value
static void _parse_opendir_parameters(char* parameters, gfal_srm_opendir_handle h)
{
    if (parameters) {
        char* saveptr = NULL;
        char* pair = strtok_r(parameters, ";", &saveptr);
        if (pair) {
            do {
                char* key = pair;
                char* value = strchr(pair, '=');
                if (value) {
                    *value = '\0';
                    ++value;
                    if (strcasecmp("offset", key) == 0) {
                        h->slice_offset = atoi(value);
                    }
                    else if (strcasecmp("count", key) == 0) {
                        h->max_count = atoi(value);
                    }
                }
            } while ((pair = strtok_r(NULL, ";", &saveptr)));
        }
    }
    else {
        h->slice_offset = 0;
        h->max_count = 0;
    }
}

gfal_file_handle gfal_srm_opendir_internal(gfal_srmv2_opt* opts, char* endpoint,
        const char* surl, GError** err)
{
    g_return_val_err_if_fail(opts && endpoint && surl, NULL, err,
            "[gfal_srmv2_opendir_internal] invalid args");

    // As extra parameters may be passed separated with ';',
    // we need to remove those from the surl, and then process them
    char* real_surl  = g_strdup(surl);
    char* parameters = _strip_parameters(real_surl);

    GError* tmp_err = NULL;
    gfal_file_handle resu = NULL;
    struct stat st;
    int exist = gfal_statG_srmv2_internal(opts, &st, endpoint, real_surl, &tmp_err);

    if (exist == 0) {
        if (S_ISDIR(st.st_mode)) {
            gfal_srm_opendir_handle h =
                    g_new0(struct _gfal_srm_opendir_handle, 1);

            char *p = stpncpy(h->surl, real_surl, GFAL_URL_MAX_LEN);
            // remove trailing '/'
            for (--p; *p == '/'; --p) {
                *p = '\0';
            }

            g_strlcpy(h->endpoint, endpoint, GFAL_URL_MAX_LEN);
            _parse_opendir_parameters(parameters, h);
            resu = gfal_file_handle_new2(gfal_srm_getName(), (gpointer) h, NULL,
                                         real_surl);
        }
        else {
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), ENOTDIR, __func__,
                    "srm-plugin: %s is not a directory, impossible to list content", surl);
        }
    }

    if (tmp_err)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    g_free(real_surl);
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
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT, __func__,
                    "support for SRMv1 is removed in 2.0, failure");
			resu = NULL;
		}else {
		    gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT, __func__,
		            "unknow version of the protocol SRM , failure");
			resu = NULL;			
		}
		
	}
	
	if(tmp_err)
		gfal2_propagate_prefixed_error(err, tmp_err, __func__);
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
