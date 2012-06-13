#pragma once
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
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

/**
 * @file gfal_common_config.h
 * @author Devresse Adrien
 * @version 1.0
 * @date 04/11/2011
 * */

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>

// create or delete configuration manager for gfal 2.0, internal
gfal_conf_t gfal_conf_new();

void gfal_conf_delete(gfal_conf_t conf);

GQuark gfal_quark_config_loader();


gchar * gfalc_get_opt_string(gfal_handle handle, const gchar *group_name,
                                    const gchar *key, GError **error);


gint gfalc_set_opt_string(gfal_handle handle, const gchar *group_name,
                                    const gchar *key, gchar* value, GError **error);

gint gfalc_get_opt_integer(gfal_handle handle, const gchar *group_name,
                                 const gchar *key, GError **error);


gint gfalc_set_opt_integer(gfal_handle handle, const gchar *group_name,
                                  const gchar *key, gint value,
                                  GError** error);

gint gfalc_set_opt_boolean(gfal_handle handle, const gchar *group_name,
                                  const gchar *key, gboolean value, GError **error);

gboolean gfalc_get_opt_boolean(gfal_handle handle, const gchar *group_name,
                                        const gchar *key, GError **error);


gint gfalc_set_opt_string_list(gfal_handle handle, const gchar *group_name,
                                     const gchar *key,
                                     const gchar * const list[],
                                     gsize length,
                                     GError ** error);

gchar ** gfalc_get_opt_string_list(gfal_handle handle, const gchar *group_name,
                                          const gchar *key, gsize *length, GError **error);
