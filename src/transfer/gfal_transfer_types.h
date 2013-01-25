#pragma once
#ifndef _GFAL2_TRANSFER_TYPES_
#define _GFAL2_TRANSFER_TYPES_

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
 
#include <stdlib.h>
#include <glib.h>
#include <sys/time.h>

#define GFALT_DEFAULT_TRANSFERT_TIMEOUT 180
#define GFALT_DEFAULT_NB_STREAM			0

 
#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

/**
 * @file gfal_transfer_types.h
 * @author Adrien Devresse
 */

/**
 * @brief container for transfer related parameters
 * */
typedef struct _gfalt_params_t* gfalt_params_t; 

/**
 * @brief internal status of a copy file action
 * */
typedef struct _gfalt_transfer_status* gfalt_transfer_status_t;

/**
 * @brief copy gfalt_monitor_transfer
 * This function is called callback_mperiod milli-seconds in order to provide informations and a control on the tranfers.
 *  @param src : URL of the source file
 *  @param dst : URL of the dest file
 *  @param user_data : external pointer provided before
 * */
typedef void (*gfalt_monitor_func)(gfalt_transfer_status_t h, const char* src, const char* dst, gpointer user_data)  ;


/**
 * @brief Predefined stages.
 */
extern GQuark GFAL_EVENT_PREPARE_ENTER;
extern GQuark GFAL_EVENT_PREPARE_EXIT;
extern GQuark GFAL_EVENT_TRANSFER_ENTER;
extern GQuark GFAL_EVENT_TRANSFER_EXIT;
extern GQuark GFAL_EVENT_CLOSE_ENTER;
extern GQuark GFAL_EVENT_CLOSE_EXIT;
extern GQuark GFAL_EVENT_CHECKSUM_ENTER;
extern GQuark GFAL_EVENT_CHECKSUM_EXIT;
extern GQuark GFAL_EVENT_CANCEL_ENTER;
extern GQuark GFAL_EVENT_CANCEL_EXIT;

typedef enum {GFAL_EVENT_SOURCE = 0,
              GFAL_EVENT_DESTINATION,
              GFAL_EVENT_NONE} gfal_event_side_t;
/**
 * @brief Event message.
 */
struct _gfalt_event {
  gfal_event_side_t side;         /*< Which side triggered the stage change */
  gint64            timestamp;    /*< Timestamp in milliseconds */
  GQuark            stage;        /*< Stage. You can check the predefined ones. */
  GQuark            domain;       /*< Domain/protocol of this stage. i.e. SRM*/
  const char       *description;  /*< Additional description */
};
typedef struct _gfalt_event* gfalt_event_t;

/**
 * This function is called when a transfer changes its stage.
 * @param e : Event message.
 * @param user_data : external pointer provided before
 */
typedef void (*gfalt_event_func)(const gfalt_event_t e, gpointer user_data);



// plugin reserved API
//! @cond
typedef struct _gfalt_hook_transfer_plugin{
    gpointer plugin_transfer_data;
    int status;
    size_t average_baudrate;
    size_t instant_baudrate;
    time_t transfer_time;
    size_t bytes_transfered;
    void* futur_usage[25];
} gfalt_hook_transfer_plugin_t;
//! @endcond
#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_GFAL2_TRANSFER_TYPES_

