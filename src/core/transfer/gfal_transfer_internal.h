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
#ifndef GFAL_TRANSFER_INTERNAL_H_
#define GFAL_TRANSFER_INTERNAL_H_

#include "gfal_transfer.h"
#include "gfal_transfer_plugins.h"
#include <sys/types.h>

struct _gfalt_params_t {
    gboolean lock;              // lock enabled after the start of the transfer
    guint64 timeout;            // connexion timeout
    guint64 tcp_buffer_size;
    gboolean replace_existing;  // replace destination or not
    off_t start_offset;         // start offset in case of restart
    guint nb_data_streams;      // nb of parallels streams
    gboolean strict_mode;       // state of the strict copy mode
    gboolean local_transfers;   // local transfer authorized
    gboolean parent_dir_create; // force the creation of the parent dir
    gboolean proxy_delegation;  // use TPC proxy delegation
    // spacetoken management for SRM
    gchar *src_space_token;
    gchar *dst_space_token;
    // checksums
    gfalt_checksum_mode_t checksum_mode;
    gchar *checksum_value;
    gchar *checksum_type;

    // callback lists
    GSList *monitor_callbacks;
    GSList *event_callbacks;
};


struct _gfalt_callback_entry {
    gpointer func, udata;
    GDestroyNotify udata_free;
};


int perform_local_copy(gfal2_context_t context, gfalt_params_t params,
    const char *src, const char *dst, GError **error);

#endif /* GFAL_TRANSFER_INTERNAL_H_ */
