/**
 * @file   glib.h
 * @brief  Advanced functions of glib for the old versions
 * @author Devresse Adrien
 **/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>

// GError
#if (GLIB_CHECK_VERSION(2,16,0) != TRUE)            // add a advanced functions of glib for the old versions

#define ERROR_OVERWRITTEN_WARNING "GError set over the top of a previous GError or uninitialized memory.\n"

void     g_propagate_prefixed_error   (GError       **dest,
                                       GError        *src,
                                       const gchar   *format,
                                       ...) G_GNUC_PRINTF (3, 4);

void     g_prefix_error               (GError       **err,
                                       const gchar   *format,
                                       ...) G_GNUC_PRINTF (2, 3);

#endif


// GList
#if (GLIB_CHECK_VERSION(2,28,0) != TRUE)
void g_list_free_full(GList *list, GDestroyNotify free_func);
#endif

#ifdef __cplusplus
}
#endif
