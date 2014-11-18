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
#include <transfer/gfal_transfer_types_internal.h>

#include <ctime>


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
GQuark GFAL_EVENT_OVERWRITE_DESTINATION = g_quark_from_static_string("OVERWRITE");


int plugin_trigger_event(gfalt_params_t params, GQuark domain,
        gfal_event_side_t side, GQuark stage, const char* fmt, ...)
{
    char buffer[512] = { 0 };
    va_list msg_args;
    va_start(msg_args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, msg_args);
    va_end(msg_args);

    if (params->event_callback) {
        struct _gfalt_event event;
        GTimeVal tmst;

        g_get_current_time(&tmst);

        event.domain = domain;
        event.side = side;
        event.stage = stage;
        event.timestamp = tmst.tv_sec * 1000 + tmst.tv_usec / 1000;
        event.description = buffer;

        params->event_callback(&event, params->user_data);
    }

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

    gfal_log(GFAL_VERBOSE_VERBOSE, "Event triggered: %s %s %s %s",
            side_str, g_quark_to_string(domain), g_quark_to_string(stage), buffer);;
    return 0;
}

void gfalt_propagate_prefixed_error(GError **dest, GError *src, const gchar *function,
        const gchar *side, const gchar *note)
{
    if (note)
        gfal2_propagate_prefixed_error_extended(dest, src, function, "%s %s ", side, note);
    else
        gfal2_propagate_prefixed_error_extended(dest, src, function, "%s ", side);
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
