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

#include <assert.h>
#include <errno.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gfal2_uri.h"


// From RFC3986, appendix B
#define URI_REGEX "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?"
//                  12            3  4          5       6  7        8 9
#define AUTHORITY_REGEX "^(([^@]*)@)?(([[:alnum:]][-_[:alnum:]]*(\\.[-_[:alnum:]]+)*)|(\\[[a-zA-Z0-9:]+\\]))?(:[[:digit:]]+)?"
//                        12         34                         5                    6                  3 7

static GQuark scope_uri(){
	return g_quark_from_static_string("Gfal::Uri_util");
}


static char *_strdupmatch(const char *str, regmatch_t *match)
{
    if (match->rm_so < 0)
        return NULL;

    size_t match_len = match->rm_eo - match->rm_so;
    return g_strndup(str + match->rm_so, match_len);
}


gfal2_uri *gfal2_parse_uri(const char *uri, GError **err)
{
    char buffer[128];

    regex_t preg;
    int ret = regcomp(&preg, URI_REGEX, REG_EXTENDED | REG_ICASE);
    assert(ret == 0);

    regmatch_t pmatch[10];
    ret = regexec(&preg, uri, 10, pmatch, 0);
    if (ret != 0) {
        regerror(ret, &preg, buffer, sizeof(buffer));
        regfree(&preg);
        gfal2_set_error(err, scope_uri(), EINVAL, __func__, "Could not match the uri: %s", buffer);
        return NULL;
    }

    // URI
    gfal2_uri *parsed = g_malloc0(sizeof(*parsed));
    parsed->scheme = _strdupmatch(uri, &pmatch[2]);
    parsed->path = _strdupmatch(uri, &pmatch[5]);
    parsed->query = _strdupmatch(uri, &pmatch[7]);
    parsed->fragment = _strdupmatch(uri, &pmatch[9]);
    parsed->original = uri;


    // Authority not defined
    if (pmatch[4].rm_so < 0) {
        parsed->userinfo = parsed->host = NULL;
        parsed->port = 0;
    }
    // Authority defined but empty
    else if (pmatch[4].rm_so == pmatch[4].rm_eo) {
        parsed->host = g_strdup("");
    }
    // Authority has content
    if (pmatch[4].rm_so != pmatch[4].rm_eo) {
        char *authority = _strdupmatch(uri, &pmatch[4]);

        regex_t authreg;
        ret = regcomp(&authreg, AUTHORITY_REGEX, REG_EXTENDED | REG_ICASE);
        assert(ret == 0);

        regmatch_t authmatch[8];
        ret = regexec(&authreg, authority, 8, authmatch, 0);
        if (ret != 0) {
            regerror(ret, &preg, buffer, sizeof(buffer));
            regfree(&preg);
            gfal2_free_uri(parsed);
            gfal2_set_error(err, scope_uri(), EINVAL, __func__, "Could not match the authority: %s", buffer);
            return NULL;
        }

        parsed->userinfo = _strdupmatch(authority, &authmatch[2]);
        parsed->host = _strdupmatch(authority, &authmatch[3]);
        if (authmatch[7].rm_so > -1) {
            parsed->port = atol(authority + authmatch[7].rm_so + 1);
        }

        regfree(&authreg);
        g_free(authority);
    }

    regfree(&preg);

    return parsed;
}


void gfal2_free_uri(gfal2_uri* uri)
{
    if (uri) {
        g_free(uri->scheme);
        g_free(uri->userinfo);
        g_free(uri->host);
        g_free(uri->path);
        g_free(uri->query);
        g_free(uri->fragment);
    }
    g_free(uri);
}

int _push_component(gchar **str_array, int i, gchar *component)
{
    if (component) {
        str_array[i++] = component;
    }
    return i;
}

char *gfal2_join_uri(gfal2_uri* uri)
{
    gchar *str_array[11];
    char port[12];
    int i = 0;

    if (uri->scheme) {
        i = _push_component(str_array, i, uri->scheme);
        i = _push_component(str_array, i, "://");
    }
    if (uri->userinfo) {
        i = _push_component(str_array, i, uri->userinfo);
        i = _push_component(str_array, i, "@");
    }
    i = _push_component(str_array, i, uri->host);
    if (uri->port) {
        snprintf(port, sizeof(port), ":%d", uri->port);
        i = _push_component(str_array, i, port);
    }
    i = _push_component(str_array, i, uri->path);
    if (uri->query) {
        i = _push_component(str_array, i, "?");
        i = _push_component(str_array, i, uri->query);
    }
    if (uri->fragment) {
        i = _push_component(str_array, i, "#");
        i = _push_component(str_array, i, uri->fragment);
    }
    str_array[i] = NULL;

    return g_strjoinv("", str_array);
}

char *gfal2_urldecode(char *str)
{
    if (str == NULL) {
        return NULL;
    }
    
    char *r = str, *w = str;
    while (*r != '\0') {
        if (*r == '%') {
            *w = strtol(r + 1, &r, 16);
            ++w;
        } else {
            *w = *r;
            ++r;
            ++w;
        }
    }
    *w = '\0';
    return str;
}
