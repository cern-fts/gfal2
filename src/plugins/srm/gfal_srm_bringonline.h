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

#include <glib.h>

#include "gfal_srm.h"


int gfal_srmv2_bring_onlineG(plugin_handle ch, const char *surl,
    time_t pintime, time_t timeout,
    char *token, size_t tsize,
    int async,
    GError **err);

int gfal_srmv2_bring_online_pollG(plugin_handle ch, const char *surl,
    const char *token, GError **err);

int gfal_srmv2_release_fileG(plugin_handle ch, const char *surl,
    const char *token, GError **err);


int gfal_srmv2_bring_online_listG(plugin_handle ch, int nbfiles, const char *const *surls,
    time_t pintime, time_t timeout,
    char *token, size_t tsize,
    int async,
    GError **err);

int gfal_srmv2_bring_online_poll_listG(plugin_handle ch, int nbfiles, const char *const *surls,
    const char *token, GError **err);

int gfal_srmv2_release_file_listG(plugin_handle ch, int nbfiles, const char *const *surls,
    const char *token, GError **err);

int gfal_srm2_abort_filesG(plugin_handle ch, int nbfiles, const char *const *surls, const char *token, GError **err);
