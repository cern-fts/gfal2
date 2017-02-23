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

#include "gfal_srm.h"
#include "gfal_srm_internal_layer.h"


struct extended_stat {
    struct stat stat;
    TFileLocality locality;
};

int gfal_statG_srmv2__generic_internal(srm_context_t context, struct stat *buf, TFileLocality *loc,
    const char *surl, GError **err);

int gfal_srm_cache_stat_add(plugin_handle ch, const char *surl, const struct stat *value, const TFileLocality *loc);

void gfal_srm_cache_stat_remove(plugin_handle ch, const char *surl);
