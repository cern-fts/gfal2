#pragma once
#ifndef _GFAL2_TRANSFER_
#define _GFAL2_TRANSFER_

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



#include <transfer/gfal_transfer_types.h>
#include <global/gfal_global.h>
#include <logger/gfal_logger.h>
#include <common/gfal_constants.h>

 
#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus


/**
 * @file gfal_transfer.h
 *  gfal2 API for file transfers .
 *  This API provide :
 *    - third party file transfers
 *    - monitoring of the transfer
 *    - flow control for the transfer
 *  @author Adrien Devresse
 */

/*!
    \defgroup transfer_group File Transfer API
*/

/*!
    \addtogroup transfer_group
    @{
*/





/**
	initiate a new parameter handle
*/
gfalt_params_t gfalt_params_handle_new(GError ** err);

/**
  delete a created parameters handle
*/
void gfalt_params_handle_delete(gfalt_params_t params, GError ** err);


/**
    create a copy of a parameter handle
*/
gfalt_params_t gfalt_params_handle_copy(gfalt_params_t params, GError ** err);

//
// parameters management functions
// these functions provide a way to configure a context for a transfer
// they must be called before starting the transfer
//


// TIMEOUT OPTIONS
/**
 * define the maximum time acceptable for the file tranfer
 * */
gint gfalt_set_timeout(gfalt_params_t, guint64 timeout, GError** err);

/**
 * get the maximum connexion timeout
 **/
guint64 gfalt_get_timeout(gfalt_params_t handle, GError** err);

// STREAM OPTIONS
/**
 * define the maximum number of parallels connexion to use for the file tranfer
 * */
gint gfalt_set_nbstreams(gfalt_params_t, guint nbstreams, GError** err);

/**
 * get the maximum number of parallels streams to use for the transfer
 **/
guint gfalt_get_nbstreams(gfalt_params_t params, GError** err);


/**
 * define the size of the tcp buffer size for network transfer
 * */
gint gfalt_set_tcp_buffer_size(gfalt_params_t, guint64 tcp_buffer_size, GError** err);

/**
 * get the size of the tcp buffer size for network transfer
 **/
guint64 gfalt_get_tcp_buffer_size(gfalt_params_t params, GError** err);

/**
  enable or disable the non-third party transfer execution ( default : true )
 */
gint gfalt_set_local_transfer_perm(gfalt_params_t, gboolean local_transfer_status, GError ** err);

/**
  get the current authorisation for the non-third party transfer execution
 */
gboolean gfalt_get_local_transfer_perm(gfalt_params_t, GError ** err);

// SRM SPECIFIC OPTIONS
/**
  set the source spacetoken for SRM transfers
*/
gint gfalt_set_src_spacetoken(gfalt_params_t params, const char* srm_spacetoken, GError** err);

/**
  get the source spacetoken for SRM transfers
*/
gchar* gfalt_get_src_spacetoken(gfalt_params_t params, GError** err);

/**
  set the destination spacetoken for SRM transfers
*/
gint gfalt_set_dst_spacetoken(gfalt_params_t params, const char* srm_spacetoken, GError** err);

/**
  get the destination spacetoken for SRM transfers
*/
gchar* gfalt_get_dst_spacetoken(gfalt_params_t params, GError** err);

// CONSISTENCY OPTIONS

/**
 * set the replace/overwritte option.
 * default : false
 * when True, if a destination file already exist, it is deleted before the copy.
 * */
gint gfalt_set_replace_existing_file(gfalt_params_t, gboolean replace, GError** err);

/**
 *  get the policy in case of destination file already existing ( replace or cancel )
 * default : cancel
 * */
gboolean gfalt_get_replace_existing_file(gfalt_params_t,  GError** err);

/**
 * set the strict copy mode
 * default : false
 * In the strict copy mode, the destination/source checks are skipped.
 * only the minimum of the operations are done
 * This option can leads to undefined behavior depending of the underlying protocol
 */
gint gfalt_set_strict_copy_mode(gfalt_params_t, gboolean strict_mode, GError** err);


/**
 * get the strict copy mode value
 *
 */
gboolean gfalt_get_strict_copy_mode(gfalt_params_t, GError** err);

/**
  force additional checksum verification between source and destination
  an Error is return by the copy function is case of checksum failure.
  @warning for safety reason, even in case of checksum failure the destination file is not removed.
*/
gint gfalt_set_checksum_check(gfalt_params_t, gboolean value, GError** err);

/**
  get the checksum verification boolean
*/
gboolean gfalt_get_checksum_check(gfalt_params_t, GError** err);

/**
  set an user-defined checksum for file content verification
  set NULL & NULL clear the current one.
  This function requires to enable global checksum verification with \ref gfalt_set_checksum_check

  @param param : parameter handle
  @param chktype : checksum type string ( MD5, ADLER32, CRC32, etc... )
  @param checksum : value of checksum in string format
  @param err : GError error report

*/
gint gfalt_set_user_defined_checksum(gfalt_params_t param, const gchar* chktype,
                                const gchar* checksum, GError** err);

/**
  get the current user-defined checksum for file content verification
  if current user-defined checksum is NULL, both of the buffer are set to empty string

*/
gint gfalt_get_user_defined_checksum(gfalt_params_t params, gchar* chktype_buff, size_t chk_type_len,
                                gchar* checksum_buff, size_t checksum_len, GError** err);

// Utility functions
/**
   enable or disable the destination parent directory creation
*/
gint gfalt_set_create_parent_dir(gfalt_params_t, gboolean value, GError** err);

/**
   enable or disable the destination parent directory creation
*/
gboolean gfalt_get_create_parent_dir(gfalt_params_t, GError** err);


// Monitoring functions

/**
 * set the user_data pointer for the monitoring callback
 * */
gint gfalt_set_user_data(gfalt_params_t, gpointer user_data, GError** err);


/**
 * get the user_data pointer for the monitoring callback
 * */
gpointer gfalt_get_user_data(gfalt_params_t,  GError** err);


     
/**
 * @brief define a callback for monitoring the current transfer
 * The default value is NULL and no monitoring will occur
*/
gint gfalt_set_monitor_callback(gfalt_params_t params, gfalt_monitor_func callback, GError** err);

/**
 * @brief get the current monitor callback
*/
gfalt_monitor_func gfalt_get_monitor_callback(gfalt_params_t params, GError** err);

/**
 * @brief Define a callback for event monitoring
 * The default value is NULL and no monitoring will occur
 */
gint gfalt_set_event_callback(gfalt_params_t params, gfalt_event_func callback, GError** err);

/**
 * @brief Get the current event monitor callback.
 */
gfalt_event_func gfalt_get_event_callback (gfalt_params_t params, GError** err);


//
// Main function for transfer launch 
//

/**
 *	@brief copy function
 *  start a synchronous copy of the file
 *  @param context : gfal context
 *  @param params parameter handle ( \ref gfalt_parameters_new )
 *  @param src source URL supported by GFAL
 *  @param dst destination URL supported by GFAL
*/
int gfalt_copy_file(gfal2_context_t context, gfalt_params_t params, const char* src, const char* dst, GError** err);


//
//  Monitoring and flow control functions
//

/**
 * cancel the current file copy
 * NOT YET implemented
 * */
gint gfalt_copy_cancel(gfalt_transfer_status_t, GError** err);

/**
 *  get a transfer status indicator
 *
 * */
gint gfalt_copy_get_status(gfalt_transfer_status_t, GError ** err);
/**
 * get an estimation of the average baudrate in bytes/s
 * */
size_t gfalt_copy_get_average_baudrate(gfalt_transfer_status_t, GError ** err);

/**
 * get an estimation of the instant baudrate in bytes/s
 * */
size_t gfalt_copy_get_instant_baudrate(gfalt_transfer_status_t, GError ** err);

/**
 * get the current number of bytes transfered
 * */
size_t gfalt_copy_get_bytes_transfered(gfalt_transfer_status_t, GError ** err);
/**
 * get the elapsed tiem since the call to \ref gfalt_copy_file
 * */
time_t gfalt_copy_get_elapsed_time(gfalt_transfer_status_t, GError ** err);

/**
    @}
    End of the File Transfer API
*/

// plugin reserved API
//! @cond
gfalt_transfer_status_t gfalt_transfer_status_create(const gfalt_hook_transfer_plugin_t * hook);
void gfalt_transfer_status_delete(gfalt_transfer_status_t state);
//! @endcond


#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_GFAL2_TRANSFER_

