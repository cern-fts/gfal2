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

#include <stdarg.h>
#include <stdio.h>
#include <logger/gfal_logger.h>
#include "gfal_common_err_helpers.h"

#if (GLIB_CHECK_VERSION(2,16,0) != TRUE)
#include "future/glib.h"
#endif


void gfal2_set_error(GError **err, GQuark domain, gint code,
        const gchar *function, const gchar *format, ...)
{
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (gfal2_log_get_level() >= G_LOG_LEVEL_DEBUG)
        g_set_error(err, domain, code, "[%s] %s", function, buffer);
    else
        g_set_error_literal(err, domain, code, buffer);
}


void gfal2_propagate_prefixed_error_extended(GError **dest, GError *src,
        const gchar *function, const gchar *format, ...)
{
    if (dest == NULL) {
        g_error_free(src);
        return;
    }

    if (gfal2_log_get_level() >= G_LOG_LEVEL_DEBUG) {
        if (src->message[0] == '[')
            g_propagate_prefixed_error(dest, src, "[%s]", function);
        else
            g_propagate_prefixed_error(dest, src, "[%s] ", function);
    }
    else {
        *dest = src;
    }

    if (format != NULL) {
        char buffer[512];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        g_prefix_error(dest, "%s", buffer);
    }
}


void gfal2_propagate_prefixed_error(GError **dest, GError *src,
        const gchar *function)
{
    gfal2_propagate_prefixed_error_extended(dest, src, function, NULL);
}
