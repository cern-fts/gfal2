
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

#include <assert.h>
#include <errno.h>
#include <regex.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "uri_util.h"

#include "common/gfal_common_err_helpers.h"

#define URI_REGEX "(([[:alnum:]]+):/{2}){1}([[:alnum:]][-_[:alnum:]]*(\\.[-_[:alnum:]]+)*)?(:[[:digit:]]*)?(:)?([^?]*)?(.*)"


static GQuark scope_uri(){
	return g_quark_from_static_string("Gfal::Uri_util");
}


static int _cpmatch(char* out, const char* str, regmatch_t* match, size_t out_size)
{
    if (match->rm_so < 0)
        return 0;

    size_t match_len = match->rm_eo - match->rm_so + 1;
    if (match_len >= out_size)
        match_len = out_size;
    g_strlcpy(out, str + match->rm_so, match_len);

    return 1;
}


int gfal_parse_uri(const char* uri, gfal_uri* parsed, GError** err)
{
    char buffer[128];

    memset(parsed, 0, sizeof(*parsed));

    regex_t preg;
    int ret = regcomp(&preg, URI_REGEX, REG_EXTENDED | REG_ICASE);
    assert(ret == 0);

    regmatch_t pmatch[9];
    ret = regexec(&preg, uri, 9, pmatch, 0);
    if (ret != 0) {
        regerror(ret, &preg, buffer, sizeof(buffer));
        gfal2_set_error(err, scope_uri(), EINVAL, __func__, "Could not match the uri: %s", buffer);
        return -1;
    }

    _cpmatch(parsed->scheme, uri, &pmatch[2], sizeof(parsed->scheme));
    _cpmatch(parsed->domain, uri, &pmatch[3], sizeof(parsed->domain));

    if (_cpmatch(buffer, uri, &pmatch[5], sizeof(buffer)))
        parsed->port = atoi(buffer + 1);

    _cpmatch(parsed->path, uri, &pmatch[7], sizeof(parsed->path));
    _cpmatch(parsed->query, uri, &pmatch[8], sizeof(parsed->query));

    return 0;
}


int gfal_hostname_from_uri(const char * uri, char* buff_hostname, size_t s_buff, GError ** err)
{
    gfal_uri parsed;
    gfal_parse_uri(uri, &parsed, err);
    if (*err)
        return -1;
    if (parsed.domain[0] == '\0') {
        gfal2_set_error(err, scope_uri(), EINVAL, __func__, "No host in the given uri");
        return -1;
    }
    if (parsed.port)
        snprintf(buff_hostname, s_buff, "%s:%d", parsed.domain, parsed.port);
    else
        snprintf(buff_hostname, s_buff, "%s", parsed.domain);
    return 0;
}

gboolean gfal_error_keep_first_err(GError** err_out, ...){
    va_list err_list;
    GError ** tmp_err=NULL;
    gboolean done = FALSE;
    va_start(err_list, err_out);
    while( (tmp_err = va_arg(err_list,GError**)) != NULL){
        if(*tmp_err != NULL){
            if(done == FALSE){
                g_propagate_error(err_out,*tmp_err);
                done =TRUE;
            }else{
                g_clear_error(tmp_err);
            }
        }
    }
    va_end(err_list);
    return done;
}
