#pragma once
#ifndef GFAL_URI_UTIL_H
#define GFAL_URI_UTIL_H
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

#include <glib.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SCHEME_MAX 16

typedef struct gfal_uri {
   char     scheme[SCHEME_MAX];
   char     domain[HOST_NAME_MAX];
   unsigned port;
   char     path [PATH_MAX];
   char     query[PATH_MAX];
} gfal_uri;


/*
 * Return only the hostname bit from the uri
 */
int gfal_hostname_from_uri(const char * uri, char* buff_hostname, size_t s_buff, GError ** err);

/*
 * Parse an URI
 */
int gfal_parse_uri(const char* uri, gfal_uri* parsed, GError** err);


#ifdef __cplusplus
}
#endif 

#endif
