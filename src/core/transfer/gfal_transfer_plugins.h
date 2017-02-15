/*
 * Copyright (c) CERN 2013-2015
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
#ifndef GFAL_TRANSFER_PLUGINS_H_
#define GFAL_TRANSFER_PLUGINS_H_

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif

#include <transfer/gfal_transfer.h>

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

struct _gfalt_transfer_status {
    gpointer plugin_transfer_data;
    int status;
    size_t average_baudrate;
    size_t instant_baudrate;
    time_t transfer_time;
    size_t bytes_transfered;
};

/**
 * Convenience method for event callback
 * @param params The transfer parameters.
 * @param domain The plugin domain.
 * @param side   The side that triggered the change, if any.
 * @param stage  The new stage.
 * @param fmt    A format string for a message
 */
int plugin_trigger_event(gfalt_params_t params, GQuark domain,
                         gfal_event_side_t side, GQuark stage,
                         const char* fmt, ...);

/**
 * Convenience method for monitoring callbacks
 * @param params The transfer parameters.
 * @param status The transfer status.
 * @param src    Source surl.
 * @param dst    Destination surl.
 */
int plugin_trigger_monitor(gfalt_params_t params, gfalt_transfer_status_t status,
        const char* src, const char* dst);

/**
 * Convenience error methods for copy implementations
 */
void gfalt_propagate_prefixed_error(GError **dest, GError *src, const gchar *function, const gchar *side, const gchar *note);

void gfalt_set_error(GError **err, GQuark domain, gint code, const gchar *function,
        const char *side, const gchar *note, const gchar *format, ...) G_GNUC_PRINTF (7, 8);

#define GFALT_ERROR_SOURCE      "SOURCE"
#define GFALT_ERROR_DESTINATION "DESTINATION"
#define GFALT_ERROR_TRANSFER    "TRANSFER"
#define GFALT_ERROR_CHECKSUM    "CHECKSUM"
#define GFALT_ERROR_EXISTS      "EXISTS"
#define GFALT_ERROR_OVERWRITE   "OVERWRITE"
#define GFALT_ERROR_PARENT      "MAKE_PARENT"

/**
 * GFALT_ERROR_CHECKSUM occurs during the retrieval of the checksum
 * GFALT_ERROR_CHECKSUM_MISMATCH means the checksum was successfully retrieved,
 * but the comparison failed
 */
#define GFALT_ERROR_CHECKSUM_MISMATCH "CHECKSUM MISMATCH"
/**
 * Size verification failed
 */
#define GFALT_ERROR_SIZE_MISMATCH     "SIZE MISMATCH"

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* GFAL_TRANSFER_PLUGINS_H_ */
