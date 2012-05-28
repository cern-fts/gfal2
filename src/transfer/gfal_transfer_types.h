#pragma once
#ifndef _GFAL2_TRANSFER_TYPES_
#define _GFAL2_TRANSFER_TYPES_

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
 
#include <stdlib.h>
#include <uuid/uuid.h>

#include <glib.h>

#define GFALT_DEFAULT_TRANSFERT_TIMEOUT 60
#define GFALT_DEFAULT_NB_STREAM			5

 
#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

/**
 * @brief container for transfer related parameters
 * */
typedef struct _gfalt_params_t* gfalt_params_t; 

/**
 * @brief internal status of a copy file action
 * */
typedef gpointer gfalt_tfr_handle; 
typedef gfalt_tfr_handle gfalt_transfer_status_t; 

/**
 * @brief copy gfalt_monitor_transfer
 * This function is called callback_mperiod milli-seconds in order to provide informations and a control on the tranfers.
 *  @param src : URL of the source file
 *  @param dst : URL of the dest file
 *  @param user_data : external pointer provided before
 * */
typedef void (*gfalt_monitor_func)(gfalt_transfer_status_t h, const char* src, const char* dst, gpointer user_data)  ;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_GFAL2_TRANSFER_TYPES_

