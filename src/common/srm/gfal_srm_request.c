/*
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#include "gfal_srm_request.h"

gfal_srm_params_t gfal_srm_params_new(gfal_srm_plugin_t handle, GError ** err ){
	gfal_srm_params_t res = g_new0(struct _gfal_srm_params,1);
	res->protocols = handle->opt_srmv2_protocols;
	res->desiredpintime= handle->opt_srmv2_desiredpintime;
	res->proto_version = handle->srm_proto_type;
	res->spacetokendesc= handle->opt_srmv2_spacetokendesc;
	return res;
}

 void gfal_srm_params_free(gfal_srm_params_t params){
	 g_free(params);
 }
 
 
char ** gfal_srm_params_get_protocols(gfal_srm_params_t params){
		return params->protocols;
}

void gfal_srm_params_set_protocols(gfal_srm_params_t params, char** protocols){
	params->protocols = protocols;
}
