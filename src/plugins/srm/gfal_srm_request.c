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

#include "gfal_srm_request.h"
#include "gfal_srm_internal_layer.h"


static gchar *get_spacetoken_from_config(gfal_srmv2_opt *handle)
{
    GError *error = NULL;
    gchar *stoken = gfal2_get_opt_string(handle->handle, srm_config_group, srm_spacetokendesc, &error);
    if (error) g_error_free(error); // Do not really care about this
    return stoken;
}


gfal_srm_params_t gfal_srm_params_new(gfal_srmv2_opt *handle)
{
    gfal_srm_params_t
        res = g_new0(struct _gfal_srm_params, 1);
    res->protocols = srm_get_turls_sup_protocol(handle->handle);
    res->proto_version = handle->srm_proto_type;
    res->spacetokendesc = get_spacetoken_from_config(handle);
    res->file_size = 0;
    return res;
}


void gfal_srm_params_free(gfal_srm_params_t params)
{
    if (params) {
        g_free(params->spacetokendesc);
        g_strfreev(params->protocols);
        g_free(params);
    }
}


char **gfal_srm_params_get_protocols(gfal_srm_params_t params)
{
    return params->protocols;
}


gchar *gfal_srm_params_get_spacetoken(gfal_srm_params_t params)
{
    return params->spacetokendesc;
}


void gfal_srm_params_set_spacetoken(gfal_srm_params_t params, const char *spacetoken)
{
    g_free(params->spacetokendesc);
    params->spacetokendesc = g_strdup(spacetoken);
}


void gfal_srm_params_set_protocols(gfal_srm_params_t params, char **protocols)
{
    if (params->protocols)
        g_strfreev(params->protocols);
    params->protocols = protocols;
}


void gfal_srm_params_set_size(gfal_srm_params_t params, size_t file_size)
{
    params->file_size = file_size;
}
