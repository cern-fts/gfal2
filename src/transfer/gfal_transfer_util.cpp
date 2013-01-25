/*
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
#include <stdio.h>
#include <glib.h>

#include <transfer/gfal_transfer_internal.h>


GQuark GFAL_EVENT_PREPARE_ENTER = g_quark_from_static_string("PREPARE:ENTER");
GQuark GFAL_EVENT_PREPARE_EXIT = g_quark_from_static_string("PREPARE:EXIT");
GQuark GFAL_EVENT_TRANSFER_ENTER = g_quark_from_static_string("TRANSFER:ENTER");
GQuark GFAL_EVENT_TRANSFER_EXIT = g_quark_from_static_string("TRANSFER:EXIT");
GQuark GFAL_EVENT_CLOSE_ENTER = g_quark_from_static_string("CLOSE:ENTER");
GQuark GFAL_EVENT_CLOSE_EXIT = g_quark_from_static_string("CLOSE:EXIT");
GQuark GFAL_EVENT_CHECKSUM_ENTER = g_quark_from_static_string("CHECKSUM:ENTER");
GQuark GFAL_EVENT_CHECKSUM_EXIT = g_quark_from_static_string("CHECKSUM:EXIT");
GQuark GFAL_EVENT_CANCEL_ENTER = g_quark_from_static_string("CANCEL:ENTER");
GQuark GFAL_EVENT_CANCEL_EXIT = g_quark_from_static_string("CANCEL:EXIT");
