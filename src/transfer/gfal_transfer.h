#pragma once
#ifndef _GFAL2_TRANSFER_
#define _GFAL2_TRANSFER_

/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
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

/**
 * @file gfal_transfer.h
 *  gfal API for file transfers of the gfal2_transfer shared library.
 *  This API provide : 
 *    - third party file transfers 
 *    - monitoring of the transfer
 *    - flow control for the transfer
 *  @author Adrien Devresse 
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
	initiate a new parameter handle
*/
gfalt_params_t gfalt_params_handle_new(GError ** err);

/**
  delete a created parameters handle
*/
void gfalt_params_handle_delete(gfalt_params_t params, GError ** err);


//
// parameters management functions
// these functions provide a way to configure a context for a transfer
// they must be called before starting the transfer
//

/**
 * define the maximum time acceptable for the file tranfer
 * */
gint gfalt_set_timeout(gfalt_params_t, guint64 timeout, GError** err);

/**
 * get the maximum connexion timeout
 **/
guint64 gfalt_get_timeout(gfalt_params_t handle, GError** err);

/**
 * define the maximum number of parallels connexion to use for the file tranfer
 * */
gint gfalt_set_nbstreams(gfalt_params_t, guint nbstreams, GError** err);

/**
 * get the maximum number of parallels streams to use for the transfer
 **/
guint gfalt_get_nbstreams(gfalt_params_t params, GError** err);

/**
 * set the policy in case of destination file already existing ( replace or cancel )
 * default : cancel
 * */
gint gfalt_set_replace_existing_file(gfalt_params_t, gboolean replace, GError** err);

/**
 *  get the policy in case of destination file already existing ( replace or cancel )
 * default : cancel
 * */
gboolean gfalt_get_replace_existing_file(gfalt_params_t,  GError** err);

/**
 * default offset for the copy of the file ( retry function )
 * default : 0
 * */
gint gfalt_set_offset_from_source(gfalt_params_t, off_t offset, GError** err);

/**
 * set the user_data pointer for statefull usages.
 * */
gint gfalt_set_user_data(gfalt_params_t, gpointer user_data, GError** err);

/**
 * set the uid of the transfer
 * */
gint gfalt_set_uuid(gfalt_params_t, uuid_t uuid, GError** err);

/**
 * set the minimum among of time between two calls of gfalt_monitor_tfr
 * 
 * */
gint gfalt_set_callback_mperiod(gfalt_params_t, guint mtime, GError** err); // time in ms between two callback calls.
// etc ......     
     
/**
 * @brief define a callback for monitoring the current transfer
 * The default value is NULL and no monitoring will occures
*/
void gfalt_set_monitor_callback(gfalt_params_t params, gfalt_monitor_func callback);



//
// Main function for transfer launch 
//

/**
 *	@brief copy function
 *  start a synchronous copy of the file
 *  @param context : global gfal context
 *  @param params parameter handle ( \ref gfalt_parameters_new )
 *  @param src source URL supported by GFAL
 *  @param dst destination URL supported by GFAL
*/
int gfalt_copy_file(gfal_context_t context, gfalt_params_t params, const char* src, const char* dst, GError** err);


//
//  Monitoring and flow control functions
//

/**
 * cancel the current file copy
 * */
gint gfalt_copy_cancel(gfalt_transfer_status_t, GError** err);
/**
 * temporary put the transfer in pause
 * */
gint gfalt_copy_pause(gfalt_transfer_status_t, GError ** err );
/**
 * resume a transfer in pause
 * */
gint  gfalt_copy_resume(gfalt_transfer_status_t, GError ** err);

/**
 * 
 * */
gint gfalt_copy_get_status(gfalt_transfer_status_t, GError ** err);
/**
 * get an estimation of the baudrate
 * */
gint gfalt_copy_get_baudrate(gfalt_transfer_status_t, GError ** err);
/**
 * get the current number of bytes transfered
 * */
size_t gfalt_copy_get_bytes_transfered(gfalt_transfer_status_t, GError ** err);
/**
 * get the elapsed tiem since the call to \ref gfalt_copy_file
 * */
time_t gfalt_copy_get_elapsed_time(gfalt_transfer_status_t, GError ** err);



#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_GFAL2_TRANSFER_

