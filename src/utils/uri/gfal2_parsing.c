/*
 * Copyright (c) CERN 2020
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

#include "gfal2_parsing.h"

gchar* gfal2_utf8escape_string(const char* str, size_t str_len, const char* ignore)
{
    // Allocate initial string size
    // and relay on automatic GString reallocation further
    GString *escaped_str = g_string_sized_new(str_len);
    const char *p = str;

    while (p < str+str_len) {
        gunichar uchar = g_utf8_get_char_validated(p, str_len-(p-str));

        if (uchar == (gunichar) -1 || uchar == (gunichar) -2) {
            g_string_append_printf(escaped_str, "\\x%02x", *(guint8 *) p);
            p++;
            continue;
        }

        if ((uchar < 32 || uchar == '\\') && (uchar == '\0' || !ignore || !strchr(ignore, uchar))) {
            switch (uchar) {
                case '\b':
                    g_string_append(escaped_str, "\\b");
                    break;
                case '\f':
                    g_string_append(escaped_str, "\\f");
                    break;
                case '\n':
                    g_string_append(escaped_str, "\\n");
                    break;
                case '\r':
                    g_string_append(escaped_str, "\\r");
                    break;
                case '\t':
                    g_string_append(escaped_str, "\\t");
                    break;
                case '\\':
                    g_string_append(escaped_str, "\\\\");
                    break;
                default:
                    g_string_append_printf(escaped_str, "\\x%02x", uchar);
                    break;
            }
        }
        else {
            g_string_append_unichar(escaped_str, uchar);
        }

        p = g_utf8_next_char(p);
    }

    gchar *res = escaped_str->str;
    g_string_free(escaped_str, FALSE);
    return res;
}
