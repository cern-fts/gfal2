#pragma once
#ifndef _GFAL2_TRANSFER_TYPES_INTERNAL_
#define _GFAL2_TRANSFER_TYPES_INTERNAL_

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
#include <uuid/uuid.h>
#include "gfal_transfer_types.h"


struct _gfalt_params_t{
	gboolean lock; 				// lock enabled after the start of the transfer
	uuid_t uuid;				// unique id of the transfer
	guint64 timeout; 			// connexion timeout
    guint64 tcp_buffer_size;
	gboolean replace_existing;  // replace destination or not
	off_t start_offset;			// start offset in case of restart
	guint nb_data_streams;		// nb of parallels streams
    gboolean strict_mode;  		// state of the strict copy mode
    gboolean local_transfers;   // local transfer authorized
    gboolean parent_dir_create;  // force the creation of the parent dir
    // spacetoken management for SRM
    gchar * src_space_token;
    gchar* dst_space_token;
    // checksum management
    gboolean checksum_check;
    gchar* user_checksum;
    gchar* user_checksum_type;

    // performance callback
	gpointer user_data;			// user data information for the monitoring callback
	size_t buffer_size; // internal buffer size per flow for non-third party transfer

	// callback lists
	GSList* monitor_callbacks;
	GSList* event_callbacks;
};


struct _gfalt_transfer_status {
    const gfalt_hook_transfer_plugin_t* hook;
};

struct _gfalt_callback_entry {
    gpointer func, udata;
    GDestroyNotify udata_free;
};

#endif //_GFAL2_TRANSFER_TYPES_INTERNAL_

