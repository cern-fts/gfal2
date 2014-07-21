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

    if (gfal_get_verbose() & GFAL_VERBOSE_DEBUG)
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

    if (gfal_get_verbose() & GFAL_VERBOSE_DEBUG) {
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
