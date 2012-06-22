#pragma once
#ifndef _GFAL2_FILE_API_
#define _GFAL2_FILE_API_
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
/**
 * @file gfal_posix_api.h
 * @brief main header file for API of the posix lib
 * @author Devresse Adrien
 * @version 2.0.1
 * @date 30/09/2011
 * 
 * */



#include <glib.h>
#include <unistd.h>

#include <global/gfal_global.h>
#include <common/gfal_constants.h>

#ifdef __cplusplus
extern "C"
{
#endif 


/*!
    \defgroup file_group GFAL 2.0 generic file API ( ng api )
*/

/*!
    \addtogroup file_group
	@{
*/

/**
 *  checksum calculation function for a file url
 *
 * @param handle : gfal2 context
 * @param url : url of the file
 * @param check_type : string of the checksum type ( \ref GFAL_CHKSUM_MD5, \ref GFAL_CHKSUM_SHA1, .. )
 * @param start_offset : offset in the file where the checksum calculation will start ( 0 : from begining )
 * @param data_length : size of data to compute for the checksum after start_offset ( 0 -: full file )
 * @param checksum_buffer : buffer with checksum string as result
 * @param buffer_length : maximum buffer length
 * @param err : GError error support
 * @return 0 if success, else -1 and err MUST be set
 *  error code MUST be ENOSUPPORT in case of :
 *         - checksum calculation with offset is not supported
 *         - the specified checksum algorithm is not supported
 */
int gfal2_checksum(gfal2_context_t handle, const char* url, const char* check_type,
                 off_t start_offset, size_t data_length,
                char * checksum_buffer, size_t buffer_length, GError ** err);

/**
	@} 
    End of the FILE group
*/
#ifdef __cplusplus
}
#endif 


#endif // _GFAL2_FILE_API_
