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

#include <glib.h>
#include "glib.h"


#if (GLIB_CHECK_VERSION(2,16,0) != TRUE)            // add code of glib 2.16 for link with a very old glib version
static void
g_error_add_prefix (gchar       **string,
                    const gchar  *format,
                    va_list       ap)
{
  gchar *oldstring;
  gchar *prefix;

  prefix = g_strdup_vprintf (format, ap);
  oldstring = *string;
  *string = g_strconcat (prefix, oldstring, NULL);
  g_free (oldstring);
  g_free (prefix);
}

void
g_propagate_prefixed_error (GError      **dest,
                            GError       *src,
                            const gchar  *format,
                            ...)
{
  g_propagate_error (dest, src);

  if (dest && *dest)
    {
      va_list ap;

      va_start (ap, format);
      g_error_add_prefix (&(*dest)->message, format, ap);
      va_end (ap);
    }
}

void
g_prefix_error (GError      **err,
                const gchar  *format,
                ...)
{
  if (err && *err)
    {
      va_list ap;

      va_start (ap, format);
      g_error_add_prefix (&(*err)->message, format, ap);
      va_end (ap);
    }
}

#endif

#if (GLIB_CHECK_VERSION(2,18,0) != TRUE)
void
g_set_error_literal (GError **err,
                     GQuark domain,
                     gint code,
                     const gchar *message)
{
    g_set_error(err, domain, code, "%s", message);
}
#endif

#if (GLIB_CHECK_VERSION(2,28,0) != TRUE)

void g_list_free_full(GList *list, GDestroyNotify free_func)
{
    GList* tmp_list= list;
    while( tmp_list != NULL){
        free_func(tmp_list->data);
        tmp_list = g_list_next(tmp_list);
    }
    g_list_free(list);
}

#endif
