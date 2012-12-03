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

#include <glib.h>
#include <global/gfal_global.h>
#include <common/gfal_common_internal.h>

GQuark gfal2_get_core_quark(){
    return g_quark_from_string("GFAL2-CORE");
}



gfal2_context_t gfal2_context_new(GError ** err){
    return gfal_initG(err);
} 
	

void gfal2_context_free(gfal2_context_t context){
    gfal_handle_freeG(context);
}

gfal_context_t gfal_context_new(GError ** err){
    return gfal2_context_new(err);
}

void gfal_context_free(gfal_context_t context){
    return gfal2_context_free(context);
}

