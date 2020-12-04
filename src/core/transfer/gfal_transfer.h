/*
 * Copyright (c) CERN 2013-2017
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
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

#pragma once
#ifndef GFAL_TRANSFER_H_
#define GFAL_TRANSFER_H_

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif


#include <common/gfal_common.h>
#include <logger/gfal_logger.h>
#include <common/gfal_constants.h>


#ifdef __cplusplus
extern "C"
{
#endif

/*!
    \defgroup transfer_group File Transfer API
*/

/*!
    \addtogroup transfer_group
    @{
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
typedef void (*gfalt_monitor_func)(gfalt_transfer_status_t h, const char* src, const char* dst, gpointer user_data);

/**
 * @brief Predefined stages.
 */
extern GQuark GFAL_EVENT_PREPARE_ENTER;   /**< Triggered before entering preparation */
extern GQuark GFAL_EVENT_PREPARE_EXIT;    /**< Triggered after exiting the preparation */
extern GQuark GFAL_EVENT_TRANSFER_ENTER;  /**< Triggered before entering the transfer */
extern GQuark GFAL_EVENT_TRANSFER_EXIT;   /**< Triggered after exiting the transfer */
extern GQuark GFAL_EVENT_CLOSE_ENTER;     /**< Triggered before entering the closing (putdone) */
extern GQuark GFAL_EVENT_CLOSE_EXIT;      /**< Triggered after exiting the closing (putdone) */
extern GQuark GFAL_EVENT_CHECKSUM_ENTER;  /**< Triggered before entering checksum validation */
extern GQuark GFAL_EVENT_CHECKSUM_EXIT;   /**< Triggered after exiting checksum validation */
extern GQuark GFAL_EVENT_CANCEL_ENTER;    /**< Triggered before the cancellation logic */
extern GQuark GFAL_EVENT_CANCEL_EXIT;     /**< Triggered after the cancellation logic */
extern GQuark GFAL_EVENT_OVERWRITE_DESTINATION; /**< Triggered before overwriting */
extern GQuark GFAL_EVENT_LIST_ENTER;      /**< Triggered before listing the urls to be transferred */
extern GQuark GFAL_EVENT_LIST_ITEM;       /**< Triggered once per url pair to be transferred */
extern GQuark GFAL_EVENT_LIST_EXIT;       /**< Triggered after listing the urls to be transferred */
extern GQuark GFAL_EVENT_TRANSFER_TYPE;   /**< Triggered to register the transfer type being done */

/**
 * Types for for GFAL_EVENT_TRANSFER_TYPE
 */
#define GFAL_TRANSFER_TYPE_STREAMED "streamed"
#define GFAL_TRANSFER_TYPE_PUSH "3rd push"
#define GFAL_TRANSFER_TYPE_PULL "3rd pull"

/** Trigger of the event */
typedef enum {
    GFAL_EVENT_SOURCE = 0,  /**< Event triggered by the source */
    GFAL_EVENT_DESTINATION, /**< Event triggered by the destination */
    GFAL_EVENT_NONE         /**< Event triggered by the transfer */
} gfal_event_side_t;

/**
 * @brief Event message.
 */
struct _gfalt_event {
    gfal_event_side_t side;         /**< Which side triggered the stage change */
    gint64            timestamp;    /**< Timestamp in milliseconds */
    GQuark            stage;        /**< Stage. You can check the predefined ones. */
    GQuark            domain;       /**< Domain/protocol of this stage. i.e. SRM*/
    const char       *description;  /**< Additional description */
};

/**
 * Event message
 */
typedef struct _gfalt_event* gfalt_event_t;

/**
 * This function is called when a transfer changes its stage.
 * @param e : Event message.
 * @param user_data : external pointer provided before
 */
typedef void (*gfalt_event_func)(const gfalt_event_t e, gpointer user_data);

/**
 * Checksum verification mode
 */
typedef enum {
    /// Don't verify checksum
    GFALT_CHECKSUM_NONE    = 0x00,
    /// Compare user provided checksum vs source
    GFALT_CHECKSUM_SOURCE  = 0x01,
    /// Compare user provided checksum vs destination
    GFALT_CHECKSUM_TARGET  = 0x02,
    /// Compare user provided checksum vs both, *or* source checksum vs target checksum
    GFALT_CHECKSUM_BOTH = (GFALT_CHECKSUM_SOURCE | GFALT_CHECKSUM_TARGET)
} gfalt_checksum_mode_t;

/**
 * Create a new parameter handle
 */
gfalt_params_t gfalt_params_handle_new(GError ** err);

/**
 * Delete a created parameters handle
 */
void gfalt_params_handle_delete(gfalt_params_t params, GError ** err);


/**
 Create a copy of a parameter handle
 */
gfalt_params_t gfalt_params_handle_copy(gfalt_params_t params, GError ** err);

/**
 * Define the maximum time acceptable for the file tranfer
 */
gint gfalt_set_timeout(gfalt_params_t, guint64 timeout, GError** err);

/**
 * Get the maximum connexion timeout
 */
guint64 gfalt_get_timeout(gfalt_params_t handle, GError** err);

/**
 * Define the maximum number of parallels connexion to use for the file tranfer
 */
gint gfalt_set_nbstreams(gfalt_params_t, guint nbstreams, GError** err);

/**
 * Get the maximum number of parallels streams to use for the transfer
 */
guint gfalt_get_nbstreams(gfalt_params_t params, GError** err);

/**
 * Define the size of the tcp buffer size for network transfer
 */
gint gfalt_set_tcp_buffer_size(gfalt_params_t, guint64 tcp_buffer_size, GError** err);

/**
 * get the size of the tcp buffer size for network transfer
 */
guint64 gfalt_get_tcp_buffer_size(gfalt_params_t params, GError** err);

/**
 * Enable or disable the non-third party transfer execution ( default : true )
 */
gint gfalt_set_local_transfer_perm(gfalt_params_t, gboolean local_transfer_status, GError ** err);

/**
 * Get the current authorization for the non-third party transfer execution
 */
gboolean gfalt_get_local_transfer_perm(gfalt_params_t, GError ** err);

/**
 * Set the source spacetoken for SRM transfers
 */
gint gfalt_set_src_spacetoken(gfalt_params_t params, const char* srm_spacetoken, GError** err);

/**
 * Get the source spacetoken for SRM transfers
 */
const gchar* gfalt_get_src_spacetoken(gfalt_params_t params, GError** err);

/**
 * Set the destination spacetoken for SRM transfers
 */
gint gfalt_set_dst_spacetoken(gfalt_params_t params, const char* srm_spacetoken, GError** err);

/**
 * Get the destination spacetoken for SRM transfers
 */
const gchar* gfalt_get_dst_spacetoken(gfalt_params_t params, GError** err);

/**
 * set the replace/overwritte option.
 * default : false
 * when True, if a destination file already exist, it is deleted before the copy.
 */
gint gfalt_set_replace_existing_file(gfalt_params_t, gboolean replace, GError** err);

/**
 * Get the policy in case of destination file already existing ( replace or cancel )
 * default : cancel
 */
gboolean gfalt_get_replace_existing_file(gfalt_params_t,  GError** err);

/**
 * Set the strict copy mode
 * default : false
 * In the strict copy mode, the destination/source checks are skipped.
 * only the minimum of the operations are done
 * This option can leads to undefined behavior depending of the underlying protocol
 */
gint gfalt_set_strict_copy_mode(gfalt_params_t, gboolean strict_mode, GError** err);

/**
 * Get the strict copy mode value
 */
gboolean gfalt_get_strict_copy_mode(gfalt_params_t, GError** err);

/**
 * @deprecated Equivalent to gfalt_get_checksum_check(params, GFALT_CHECKSUM_BOTH, err)
 * Force additional checksum verification between source and destination
 * an Error is return by the copy function is case of checksum failure.
 * @warning for safety reason, even in case of checksum failure the destination file is not removed.
 */
GFAL2_DEPRECATED(gfalt_set_checksum) gint gfalt_set_checksum_check(gfalt_params_t, gboolean value, GError** err);

/**
 * @deprecated
 * Get the checksum verification boolean
 */
GFAL2_DEPRECATED(gfalt_get_checksum) gboolean gfalt_get_checksum_check(gfalt_params_t, GError** err);

/**
 * @deprecated gfalt_set_checksum
 * Set an user-defined checksum for file content verification
 * Setting NULL & NULL clear the current one.
 * This function requires to enable global checksum verification with \ref gfalt_set_checksum_check

 * @param param : parameter handle
 * @param chktype : checksum type string ( MD5, ADLER32, CRC32, etc... )
 * @param checksum : value of checksum in string format
 * @param err : GError error report
 */
GFAL2_DEPRECATED(gfalt_set_checksum) gint gfalt_set_user_defined_checksum(gfalt_params_t param,
    const gchar* chktype, const gchar* checksum, GError** err);

/**
 * @deprecated gfalt_get_checksum
 * Get the current user-defined checksum for file content verification
 * If current user-defined checksum is NULL, both of the buffer are set to empty string
 * If the value is set, but not the type, ADLER32 will be assumed
 */
GFAL2_DEPRECATED(gfalt_get_checksum) gint gfalt_get_user_defined_checksum(gfalt_params_t params,
    gchar* chktype_buff, size_t chk_type_len, gchar* checksum_buff, size_t checksum_len, GError** err);

/**
 * Set the checksum configuration to use
 * @param mode      For GFALT_CHECKSUM_SOURCE or GFALT_CHECKSUM_TARGET only, the checksum value is mandatory.
 *                  For GFALT_CHECKSUM_BOTH, the checksum value can be NULL, as the verification can be done end to end.
 * @param type      Checksum algorithm to use. Support depends on protocol and storage, but ADLER32 and MD5
 *                  are normally safe bets. If NULL, previous type is kept.
 * @param checksum  Expected checksum value. Can be NULL for GFALT_CHECKSUM_BOTH mode. If NULL, clears value.
 * @param err       GError error report
 * @return          0 on success, < 0 on failure
 * @version         2.13.0
 */
gint gfalt_set_checksum(gfalt_params_t params, gfalt_checksum_mode_t mode,
    const gchar* type, const gchar *checksum, GError **err);

/**
 * Get the checksum configuration
 * @param type_buff         Put in this buffer the configured algorithm (or "\0" if none).
 * @param type_buff_len     algorithm_buffer capacity.
 * @param checksum_buff     Put in this buffer the configured checksum value ("\0" if none).
 * @param checksum_buff_len checksum_buff capacity.
 * @param err               GError error report
 * @return                  The configured checksum mode
 * @version                 2.13.0
 */
gfalt_checksum_mode_t gfalt_get_checksum(gfalt_params_t params,
    gchar* type_buff, size_t type_buff_len, gchar* checksum_buff, size_t checksum_buff_len,
    GError **err);

/**
 * Get only the checksum mode configured
 * @return  The configured checksum mode
 */
gfalt_checksum_mode_t gfalt_get_checksum_mode(gfalt_params_t params, GError **err);

/**
 * Enable or disable the destination parent directory creation
 */
gint gfalt_set_create_parent_dir(gfalt_params_t, gboolean create_parent, GError** err);

/**
 * Get the parent directory creation value
 */
gboolean gfalt_get_create_parent_dir(gfalt_params_t, GError** err);

/**
 * Enable or disable usage of TPC proxy delegation
 */
gint gfalt_set_use_proxy_delegation(gfalt_params_t, gboolean proxy_delegation, GError** err);

/**
 * Get the usage of TPC proxy delegation value
 */
gboolean gfalt_get_use_proxy_delegation(gfalt_params_t, GError** err);

/**
 * @brief Add a new callback for monitoring the current transfer
 * Adding the same callback with a different udata will just change the udata and the free method, but the callback will not be called twice.
 * In this case, udata_free will be called with the old data.
 * udata_free can be left to NULL
 */
gint gfalt_add_monitor_callback(gfalt_params_t params, gfalt_monitor_func callback,
        gpointer udata, GDestroyNotify udata_free, GError** err);

/**
 * @brief Remove an installed monitor callback
 * It will call the method registered to free the user data
 */
gint gfalt_remove_monitor_callback(gfalt_params_t params, gfalt_monitor_func callback, GError** err);

/**
 * @brief Add a new callback for event monitoring
 * Adding the same callback with a different udata will just change the udata, but the callback will not be called twice.
 * In this case, udata_free will be called with the old data.
 * udata_free can be left to NULL
 */
gint gfalt_add_event_callback(gfalt_params_t params, gfalt_event_func callback,
        gpointer udata, GDestroyNotify udata_free, GError** err);

/**
 * @brief Remove an installed callback
 * It will call the method registered to free the user data
 */
gint gfalt_remove_event_callback(gfalt_params_t params, gfalt_event_func callback, GError** err);

/**
 *	@brief copy function
 *  start a synchronous copy of the file
 *  @param context : gfal2 context
 *  @param params parameter handle ( \ref gfalt_parameters_new )
 *  @param src source URL supported by GFAL
 *  @param dst destination URL supported by GFAL
 *  @param err the error is put here
 */
int gfalt_copy_file(gfal2_context_t context, gfalt_params_t params, const char* src,
        const char* dst, GError** err);

/**
 * @brief bulk copy operation
 * If not provided by the plugin, it will fallback to a serialized implementation
 * Note that file_erros will point to an array of nbfiles pointers to GError, where each one
 * corresponds to the source and destination pair in the same position
 * op_error will contain an error if something happened _before_ file transfering could be attempted
 */
int gfalt_copy_bulk(gfal2_context_t context, gfalt_params_t params, size_t nbfiles,
        const char* const * srcs, const char* const * dsts, const char* const* checksums,
        GError** op_error, GError*** file_erros);

/**
 * Get a transfer status indicator
 */
gint gfalt_copy_get_status(gfalt_transfer_status_t, GError ** err);

/**
 * Get an estimation of the average baudrate in bytes/s
 */
size_t gfalt_copy_get_average_baudrate(gfalt_transfer_status_t, GError ** err);

/**
 * Get an estimation of the instant baudrate in bytes/s
 */
size_t gfalt_copy_get_instant_baudrate(gfalt_transfer_status_t, GError ** err);

/**
 * Get the current number of bytes transfered
 */
size_t gfalt_copy_get_bytes_transfered(gfalt_transfer_status_t, GError ** err);
/**
 * Get the elapsed tiem since the call to \ref gfalt_copy_file
 */
time_t gfalt_copy_get_elapsed_time(gfalt_transfer_status_t, GError ** err);

/**
    @}
    End of the File Transfer API
*/

#ifdef __cplusplus
}
#endif

#endif /* GFAL_TRANSFER_H_ */

