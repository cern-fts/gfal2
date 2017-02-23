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

#pragma once
#ifndef GFAL_URI_H
#define GFAL_URI_H

#include <glib.h>
#include <gfal_api.h>

#ifdef __cplusplus
extern "C"
{
#endif

// URI struct
// Please, note that there is a difference between a component being NULL (undefined)
// and being the empty string.
// i.e. empty = separator was present, but value missing
//      undefined = separator was not present
typedef struct gfal2_uri {
    char *scheme;
    char *userinfo;
    char *host;
    unsigned port;
    char *path;
    char *query;
    char *fragment;

    const char *original;
} gfal2_uri;

/*
 * Parse an URI
 */
gfal2_uri* gfal2_parse_uri(const char *uri, GError **err);

/*
 * Free an URI. It is safe to call if uri is NULL.
 */
void gfal2_free_uri(gfal2_uri* uri);

/*
 * Returns a newly allocated string with the serialized uri
 */
char *gfal2_join_uri(gfal2_uri* uri);

/*
 * url-decodes the string, and returns a pointer to the same string
 * Modifies the buffer pointed by str!!
 */
char *gfal2_urldecode(char *str);


#ifdef __cplusplus
}
#endif

#endif
