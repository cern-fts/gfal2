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

#include <stdio.h>
#include <glib.h>
#include <time.h>

#include <transfer/gfal_transfer_internal.h>
#include <transfer/gfal_transfer_types_internal.h>
#include <common/gfal_common_err_helpers.h>



GQuark GFAL_EVENT_PREPARE_ENTER;
GQuark GFAL_EVENT_PREPARE_EXIT;
GQuark GFAL_EVENT_TRANSFER_ENTER;
GQuark GFAL_EVENT_TRANSFER_EXIT;
GQuark GFAL_EVENT_CLOSE_ENTER;
GQuark GFAL_EVENT_CLOSE_EXIT;
GQuark GFAL_EVENT_CHECKSUM_ENTER;
GQuark GFAL_EVENT_CHECKSUM_EXIT;
GQuark GFAL_EVENT_CANCEL_ENTER;
GQuark GFAL_EVENT_CANCEL_EXIT;
GQuark GFAL_EVENT_OVERWRITE_DESTINATION;
GQuark GFAL_EVENT_LIST_ENTER;
GQuark GFAL_EVENT_LIST_ITEM;
GQuark GFAL_EVENT_LIST_EXIT;


__attribute__((constructor))
static void init_event_quarks() {
    GFAL_EVENT_PREPARE_ENTER = g_quark_from_static_string("PREPARE:ENTER");
    GFAL_EVENT_PREPARE_EXIT = g_quark_from_static_string("PREPARE:EXIT");
    GFAL_EVENT_TRANSFER_ENTER = g_quark_from_static_string("TRANSFER:ENTER");
    GFAL_EVENT_TRANSFER_EXIT = g_quark_from_static_string("TRANSFER:EXIT");
    GFAL_EVENT_CLOSE_ENTER = g_quark_from_static_string("CLOSE:ENTER");
    GFAL_EVENT_CLOSE_EXIT = g_quark_from_static_string("CLOSE:EXIT");
    GFAL_EVENT_CHECKSUM_ENTER = g_quark_from_static_string("CHECKSUM:ENTER");
    GFAL_EVENT_CHECKSUM_EXIT = g_quark_from_static_string("CHECKSUM:EXIT");
    GFAL_EVENT_CANCEL_ENTER = g_quark_from_static_string("CANCEL:ENTER");
    GFAL_EVENT_CANCEL_EXIT = g_quark_from_static_string("CANCEL:EXIT");
    GFAL_EVENT_OVERWRITE_DESTINATION = g_quark_from_static_string("OVERWRITE");
    GFAL_EVENT_LIST_ENTER = g_quark_from_static_string("LIST:ENTER");
    GFAL_EVENT_LIST_ITEM = g_quark_from_static_string("LIST:ITEM");
    GFAL_EVENT_LIST_EXIT = g_quark_from_static_string("LIST:EXIT");
}


static void plugin_trigger_event_callback(gpointer data, gpointer user_data)
{

    gfalt_event_t event = (gfalt_event_t)user_data;
    struct _gfalt_callback_entry* entry = (struct _gfalt_callback_entry*)data;
    gfalt_event_func callback = (gfalt_event_func)entry->func;

    callback(event, entry->udata);
}


int plugin_trigger_event(gfalt_params_t params, GQuark domain, gfal_event_side_t side,
        GQuark stage, const char* fmt, ...)
{
    char buffer[512] = { 0 };
    va_list msg_args;
    va_start(msg_args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, msg_args);
    va_end(msg_args);

    struct _gfalt_event event;
    GTimeVal tmst;

    g_get_current_time(&tmst);

    event.domain = domain;
    event.side = side;
    event.stage = stage;
    event.timestamp = tmst.tv_sec * 1000 + tmst.tv_usec / 1000;
    event.description = buffer;

    g_slist_foreach(params->event_callbacks, plugin_trigger_event_callback, &event);

    const char* side_str;
    switch (side) {
        case GFAL_EVENT_SOURCE:
            side_str = "SOURCE";
            break;
        case GFAL_EVENT_DESTINATION:
            side_str = "DESTINATION";
            break;
        default:
            side_str = "BOTH";
    }

    gfal2_log(G_LOG_LEVEL_MESSAGE, "Event triggered: %s %s %s %s", side_str,
            g_quark_to_string(domain), g_quark_to_string(stage), buffer);
    return 0;
}


struct _gfalt_monitor_data {
    gfalt_transfer_status_t* status;
    const char* src, *dst;
};


static void plugin_trigger_monitor_callback(gpointer data, gpointer user_data)
{

    struct _gfalt_monitor_data* monitor = (struct _gfalt_monitor_data*)user_data;
    struct _gfalt_callback_entry* entry = (struct _gfalt_callback_entry*)data;
    gfalt_monitor_func callback = (gfalt_monitor_func)entry->func;

    callback(*monitor->status, monitor->src, monitor->dst, entry->udata);
}


int plugin_trigger_monitor(gfalt_params_t params, gfalt_transfer_status_t status,
        const char* src, const char* dst)
{
    struct _gfalt_monitor_data monitor;
    monitor.status = &status;
    monitor.src = src;
    monitor.dst = dst;
    g_slist_foreach(params->monitor_callbacks, plugin_trigger_monitor_callback, &monitor);
    return 0;
}


void gfalt_propagate_prefixed_error(GError **dest, GError *src, const gchar *function,
        const gchar *side, const gchar *note)
{
    if (note) {
        gfal2_propagate_prefixed_error_extended(dest, src, function, "%s %s ", side,
                note);
    }
    else {
        gfal2_propagate_prefixed_error_extended(dest, src, function, "%s ", side);
    }
}


void gfalt_set_error(GError **err, GQuark domain, gint code, const gchar *function,
        const char *side, const gchar *note, const gchar *format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (note)
        gfal2_set_error(err, domain, code, function, "%s %s %s", side, note, buffer);
    else
        gfal2_set_error(err, domain, code, function, "%s %s", side, buffer);
}
