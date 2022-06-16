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

#pragma once

#include <glib.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Given a string and it's length, returns a pointer to an UTF8-escaped Glib string.
 * The last parameter takes a list of characters for which to ignore escaping.
 * Note: the caller must handle memory deallocation of the new string
 */
gchar* gfal2_utf8escape_string(const char* str, size_t str_len, const char* ignore);

/*
 * Given a path, returns a pointer to a new string with all slashes collapsed.
 * Note: the caller must handle memory deallocation of the new string
 */
char* gfal2_path_collapse_slashes(const char* path);

#ifdef __cplusplus
}
#endif
