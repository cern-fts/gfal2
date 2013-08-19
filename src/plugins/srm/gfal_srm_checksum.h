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
 * @file gfal_srm_checksum.h
 * @brief header funtion to get the checksum of a file
 * @author Devresse Adrien
 * @version 2.0
 * @date 29/09/2011
 * */
#include "gfal_srm.h"
#include <common/gfal_constants.h>
#include <common/gfal_common_errverbose.h>




int gfal_srm_cheksumG_internal(plugin_handle ch, const char* surl,
											char* buf_checksum, size_t s_checksum,
											char* buf_chktype, size_t s_chktype, GError** err);

int gfal_srm_checksumG_fallback(plugin_handle handle, const char* url, const char* check_type,
                       char * checksum_buffer, size_t buffer_length,
                       off_t start_offset, size_t data_length,
                       gboolean turl_fallback,
                       GError ** err);

int gfal_srm_checksumG(plugin_handle handle, const char* url, const char* check_type,
                       char * checksum_buffer, size_t buffer_length,
                       off_t start_offset, size_t data_length,
                       GError ** err);

