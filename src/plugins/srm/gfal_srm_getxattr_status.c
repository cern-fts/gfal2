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
#include <common/gfal_constants.h>
#include <common/gfal_common_err_helpers.h>
#include "gfal_srm_internal_layer.h" 
#include "gfal_srm_getxattr.h"
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


ssize_t gfal_srm_status_internal(plugin_handle handle, const char* path, void* buff, size_t s_buff, GError** err){
	GError* tmp_err=NULL;
	ssize_t ret = -1;	
	gfal_srmv2_opt* ops = (gfal_srmv2_opt*) handle;
	TFileLocality loc;
	char full_endpoint[GFAL_URL_MAX_LEN]={0};
	enum gfal_srm_proto srm_types;
	
	if((gfal_srm_determine_endpoint(ops, path, full_endpoint, GFAL_URL_MAX_LEN, &srm_types, &tmp_err)) == 0){		// check & get endpoint										
		gfal_log(GFAL_VERBOSE_NORMAL, "[gfal_srm_status_internal] endpoint %s", full_endpoint);

		if (srm_types == PROTO_SRMv2){
			if( (ret = gfal_Locality_srmv2_generic_internal(ops, full_endpoint, path, &loc, &tmp_err)) >= 0){
				gfal_srm_status_copy(loc, (char*) buff, s_buff);
				ret = MIN( strlen(buff), s_buff);
			}
		} else if(srm_types == PROTO_SRM){
            g_set_error(&tmp_err,gfal2_get_plugin_srm_quark(), EPROTONOSUPPORT, "support for SRMv1 is removed in gfal 2.0, failure");
		} else{
            g_set_error(&tmp_err,gfal2_get_plugin_srm_quark(),EPROTONOSUPPORT, "Unknow SRM protocol, failure ");
		}		
	}	
	G_RETURN_ERR(ret, tmp_err, err);		
}

/*
 * main implementation of the srm status -> getxattr
 */
ssize_t gfal_srm_status_getxattrG(plugin_handle handle, const char* path, const char* name , void* buff, size_t s_buff, GError** err){
	GError* tmp_err=NULL;
	ssize_t ret = -1;
	if(s_buff ==0 || buff == NULL)
		return GFAL_URL_MAX_LEN;

	ret = gfal_srm_status_internal(handle, path, buff, s_buff, &tmp_err);

	G_RETURN_ERR(ret, tmp_err, err);	
}
