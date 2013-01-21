#pragma once
#ifndef _GFAL_CANCEL_H_
#define _GFAL_CANCEL_H_
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
#include <common/gfal_constants.h>

/**
 * @file gfal_cancel.h
 *
 * canceling operation for gfal 2.0
 *
 **/

#ifdef __cplusplus
extern "C"
{
#endif 

typedef struct gfal_cancel_token_s* gfal_cancel_token_t;
typedef void gfal_cancel_hook_cb(gfal2_context_t context, void* userdata);

/// @brief cancel operation
///
/// cancel all pending operation on the given context
/// blocking until all operations finish
/// all operations will return and trigger an ECANCELED if interrupted.
/// Thread safe
/// @param context : gfal 2 context
/// @return number of operations canceled
int gfal2_cancel(gfal2_context_t context);

/// @brief cancel status
/// return true if  \ref gfal2_cancel has been called
///
///  @param context
/// @return true if success
gboolean gfal2_is_canceled(gfal2_context_t context);


///
/// register a cancel hook, called in each cancellation
/// thread-safe
gfal_cancel_token_t gfal2_register_cancel_callback(gfal2_context_t context, gfal_cancel_hook_cb cb, void* userdata);

///
/// remove a cancel hook
/// thread-safe
void gfal2_remove_cancel_callback(gfal2_context_t context, gfal_cancel_token_t token);

/// scope of a cancel action
GQuark gfal_cancel_quark();

#ifdef __cplusplus
}
#endif 


#endif // _GFAL_CANCEL_H_
