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

#include <file/gfal_file_api.h>
#include <common/gfal_common_internal.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_errverbose.h>

// fiel descriptor checksum calculation
GQuark gfal_checksum_file_quark(){
    return g_quark_from_static_string("gfal2_checksum");
}

int gfal2_checksum(gfal2_context_t handle, const char* url, const char* check_type,
                 off_t start_offset, size_t data_length,
                char * checksum_buffer, size_t buffer_length, GError ** err){
    if( !(handle != NULL && url != NULL && check_type != NULL
                             && checksum_buffer != NULL && buffer_length != 0)){
        g_set_error(err, gfal_checksum_file_quark(),EFAULT, "Invalid parameters to %s",__func__);
        return -1;
    }

    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    gfal_log(GFAL_VERBOSE_VERBOSE, " gfal2_checksum ->");
    int res = -1;
    GError * tmp_err=NULL;
    gfal_plugin_interface* p = gfal_find_plugin(handle, url, GFAL_PLUGIN_CHECKSUM, &tmp_err);

    if(p)
        res =  p->checksum_calcG(gfal_get_plugin_handle(p), url, check_type, checksum_buffer, buffer_length, start_offset,
                                    data_length, &tmp_err);
    gfal_log(GFAL_VERBOSE_VERBOSE, " gfal2_checksum <-");
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}
