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

#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include "gfal_srm.h"
#include "gfal_srm_opendir.h"
#include "gfal_srm_namespace.h"
#include "gfal_srm_endpoint.h"
#include "gfal_srm_internal_layer.h"


// Replaces the first occurence of ';' with '/0', so
// we have a valid surl. Returns a pointer
// to the beginning of the additional parameters, if any
static char *_strip_parameters(char *surl)
{
    char *first_semicolon = strchr(surl, ';');
    if (first_semicolon) {
        *first_semicolon = '\0';
        return first_semicolon + 1;
    }
    else {
        return NULL;
    }
}

// Parse any possible additional parameters passed in the URL and set the
// corresponding values in h
// parameters should be already in the form of key=value;key=value
static void _parse_opendir_parameters(char *parameters, gfal_srm_opendir_handle h)
{
    if (parameters) {
        char *saveptr = NULL;
        char *pair = strtok_r(parameters, ";", &saveptr);
        if (pair) {
            do {
                char *key = pair;
                char *value = strchr(pair, '=');
                if (value) {
                    *value = '\0';
                    ++value;
                    if (strcasecmp("offset", key) == 0) {
                        h->chunk_offset = atoi(value);
                    }
                    else if (strcasecmp("count", key) == 0) {
                        h->chunk_size = atoi(value);
                    }
                }
            } while ((pair = strtok_r(NULL, ";", &saveptr)));
        }
    }
    else {
        h->chunk_offset = 0;
        h->chunk_size = 0;
    }
    if (h->chunk_offset || h->chunk_size) {
        h->is_chunked_listing = 1;
    }
}

static gfal_file_handle gfal_srm_opendir_internal(gfal_srm_easy_t easy, GError **err)
{
    // As extra parameters may be passed separated with ';',
    // we need to remove those from the surl, and then process them
    char *real_path = g_strdup(easy->path);
    char *parameters = _strip_parameters(real_path);

    GError *tmp_err = NULL;
    gfal_file_handle resu = NULL;
    struct stat st;
    int exist = gfal_statG_srmv2_internal(easy->srm_context, &st, NULL, real_path, &tmp_err);

    if (exist == 0) {
        if (S_ISDIR(st.st_mode)) {
            gfal_srm_opendir_handle h = g_new0(struct _gfal_srm_opendir_handle, 1);
            h->easy = easy;

            char *p = stpncpy(h->surl, real_path, GFAL_URL_MAX_LEN);
            // remove trailing '/'
            for (--p; *p == '/'; --p) {
                *p = '\0';
            }

            _parse_opendir_parameters(parameters, h);
            resu = gfal_file_handle_new2(gfal_srm_getName(), (gpointer) h, NULL, real_path);
        }
        else {
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), ENOTDIR, __func__,
                "srm-plugin: %s is not a directory, impossible to list content", easy->path);
        }
    }

    if (tmp_err)
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);

    g_free(real_path);
    return resu;
}


gfal_file_handle gfal_srm_opendirG(plugin_handle handle, const char *surl, GError **err)
{
    g_return_val_err_if_fail(handle && surl, NULL, err, "[gfal_srm_opendirG] Invalid args");

    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) handle;
    gfal_file_handle resu = NULL;
    GError *tmp_err = NULL;

    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, surl, &tmp_err);
    if (easy) {
        resu = gfal_srm_opendir_internal(easy, &tmp_err);
    }
    if (tmp_err) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    }
    return resu;
}


int gfal_srm_closedirG(plugin_handle handle, gfal_file_handle fh, GError **err)
{
    g_return_val_err_if_fail(handle && fh, -1, err, "[gfal_srm_opendirG] Invalid args");

    gfal_srmv2_opt *opts = (gfal_srmv2_opt *) handle;
    gfal_srm_opendir_handle oh = (gfal_srm_opendir_handle)gfal_file_handle_get_fdesc(fh);

    gfal_srm_external_call.srm_srmv2_mdfilestatus_delete(oh->srm_file_statuses, 1);
    gfal_srm_ifce_easy_context_release(opts, oh->easy);

    g_free(oh);
    gfal_file_handle_delete(fh);
    return 0;
}
