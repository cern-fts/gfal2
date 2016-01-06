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

#pragma once
#ifndef GFAL_URI_H
#define GFAL_URI_H

#include <glib.h>
#include <gfal_api.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 512
#endif

#define SCHEME_MAX 16

typedef struct gfal_uri {
   char     scheme[SCHEME_MAX];
#ifdef HOST_NAME_MAX
   char     domain[HOST_NAME_MAX];
#else
   char     domain[64];
#endif
   unsigned port;
#ifdef PATH_MAX
   char     path [PATH_MAX];
   char     query[PATH_MAX];
#else
   char     path [4096];
   char     query[4096];
#endif
} gfal2_uri;

typedef gfal2_uri gfal_uri;


/*
 * Return only the hostname bit from the uri
 */
GFAL2_DEPRECATED(gfal2_hostname_from_uri) int gfal_hostname_from_uri(const char * uri, char* buff_hostname, size_t s_buff, GError ** err);
int gfal2_hostname_from_uri(const char * uri, char* buff_hostname, size_t s_buff, GError ** err);

/*
 * Parse an URI
 */
GFAL2_DEPRECATED(gfal2_parse_uri) int gfal_parse_uri(const char* uri, gfal_uri* parsed, GError** err);
int gfal2_parse_uri(const char* uri, gfal2_uri* parsed, GError** err);


#ifdef __cplusplus
}
#endif

#endif
