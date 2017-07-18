/*
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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
#include "gfal_srm.h"


int gfal_srm_put_rd3_turl(plugin_handle ch, gfalt_params_t p, const char *surl,
    size_t surl_file_size, char *buff_turl, int size_turl,
    char *reqtoken, size_t size_reqtoken,
    GError **err);

int gfal_srm_get_rd3_turl(plugin_handle ch, gfalt_params_t params,
    const char *surl, char *buff_turl, int size_turl,
    char *reqtoken, size_t size_reqtoken,
    GError **err);

int gfal_srm_getTURL_checksum(plugin_handle ch, const char *surl,
    char *buff_turl, int size_turl, GError **err);

int reorder_rd3_sup_protocols(char **sup_protocols, const char *surl);

int srm_abort_request_plugin(plugin_handle *handle, const char *surl,
    const char *reqtoken, GError **err);
