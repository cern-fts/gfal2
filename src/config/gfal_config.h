#pragma once
#ifndef GFAL_CONFIG_H
#define GFAL_CONFIG_H
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

#include <glib.h>
#include <global/gfal_global.h>

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

gchar * gfal2_get_opt_string(gfal2_context_t handle, const gchar *group_name,
                                    const gchar *key, GError **error);


gint gfal2_set_opt_string(gfal2_context_t handle, const gchar *group_name,
                                    const gchar *key, gchar* value, GError **error);

gint gfal2_get_opt_integer(gfal2_context_t handle, const gchar *group_name,
                                 const gchar *key, GError **error);


gint gfal2_set_opt_integer(gfal2_context_t handle, const gchar *group_name,
                                  const gchar *key, gint value,
                                  GError** error);

gint gfal2_set_opt_boolean(gfal2_context_t handle, const gchar *group_name,
                                  const gchar *key, gboolean value, GError **error);

gboolean gfal2_get_opt_boolean(gfal2_context_t handle, const gchar *group_name,
                                        const gchar *key, GError **error);


gint gfal2_set_opt_string_list(gfal_handle handle, const gchar *group_name,
                                     const gchar *key,
                                     const gchar * const list[],
                                     gsize length,
                                     GError ** error);

gchar ** gfal2_get_opt_string_list(gfal_handle handle, const gchar *group_name,
                                          const gchar *key, gsize *length, GError **error);

#ifdef __cplusplus
}
#endif  // __cplusplus


#endif /* GFAL_CONFIG_H */

