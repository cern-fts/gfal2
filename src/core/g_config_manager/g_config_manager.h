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
#ifndef G_CONFIG_MANAGER_H_
#define G_CONFIG_MANAGER_H_

#include <glib.h>

#ifdef __cplusplus
extern "C"{
#endif


typedef struct _GConfigManager* GConfigManager_t;

typedef enum _G_CONFIG_MANAGER_ERRORS{
    G_CONFIG_MANAGER_NOT_FOUND =G_KEY_FILE_ERROR_NOT_FOUND, /**< key/value not found */
    G_CONFIG_MANAGER_NO_CONFIG, /**< No configuration are loaded inside this configuration manager */
    G_CONFIG_MANAGER_UNKNOW
} G_CONFIG_MANAGER_ERRORS;

GQuark g_config_manager_quark();

GConfigManager_t g_config_manager_new();

void g_config_manager_delete(GConfigManager_t config_manager);

void g_config_manager_delete_full(GConfigManager_t config_manager);

void g_config_manager_prepend_manager(GConfigManager_t config_manager, GConfigManager_t new_config);

void g_config_manager_prepend_keyvalue(GConfigManager_t config_manager, GKeyFile * keyvalue_config);

gchar * g_config_manager_get_string(GConfigManager_t config_manager, const gchar *group_name,
                                    const gchar *key, GError **error);

gint g_config_manager_set_string(GConfigManager_t config_manager,const gchar *group_name,
                                    const gchar *key, const gchar *string, GError** error);

gchar ** g_config_manager_get_string_list(GConfigManager_t config_manager,const gchar *group_name,
                                          const gchar *key, gsize *length, GError **error);

gint g_config_manager_set_string_list(GConfigManager_t config_manager, const gchar *group_name,
                                     const gchar *key,
                                     const gchar * const list[],
                                     gsize length,
                                     GError ** error);

gint g_config_manager_get_integer(GConfigManager_t config_manager, const gchar *group_name,
                                 const gchar *key, GError **error);

gint g_config_manager_set_integer(GConfigManager_t config_manager, const gchar *group_name,
                                  const gchar *key, gint value,
                                  GError** err);

gboolean g_config_manager_get_boolean(GConfigManager_t config_manager, const gchar *group_name,
                                        const gchar *key, GError **error);

gint g_config_manager_set_boolean(GConfigManager_t config_manager, const gchar *group_name,
                                  const gchar *key, gboolean value, GError **error);

#ifdef __cplusplus
}
#endif

#endif /* G_CONFIG_MANAGER_H_ */

