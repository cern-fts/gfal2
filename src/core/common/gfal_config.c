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

#include "gfal_handle.h"
#include <gfal_api.h>
#include <string.h>

#ifndef GFAL_CONFIG_DIR_DEFAULT
#error "GFAL_CONFIG_DIR_DEFAULT should be define at compile time"
#endif

const gchar *config_env_var = GFAL_CONFIG_DIR_ENV;
const gchar *default_config_dir = GFAL_CONFIG_DIR_DEFAULT "/" GFAL_CONFIG_DIR_SUFFIX "/";


typedef struct _gfal_conf_elem {
    GKeyFile *key_file;
} *gfal_conf_elem_t;

typedef struct _gfal_conf {
    GMutex *mux;
    GKeyFile *running_config;
    GConfigManager_t static_manager;
    GConfigManager_t running_manager;
} *gfal_conf_t;


typedef struct _gfal_key_value {
    char *key, *value;
} *gfal_key_value_t;


void gfal_free_keyvalue(gpointer data, gpointer user_data)
{
    gfal_key_value_t keyval = (gfal_key_value_t) data;
    g_free(keyval->key);
    g_free(keyval->value);
    g_free(keyval);
}


static gchar *check_configuration_dir(GError **err)
{
    struct stat st;
    int res;
    gchar *dir_config = NULL;
    const gchar *env_str = g_getenv(config_env_var);
    if (env_str != NULL) {
        gfal2_log(G_LOG_LEVEL_DEBUG,
            " %s env var found, try to load configuration from %s",
            config_env_var, env_str);
        dir_config = g_strdup(env_str);
    }
    else {
        gfal2_log(G_LOG_LEVEL_DEBUG,
            " no %s env var found, try to load configuration from default directory %s",
            config_env_var, default_config_dir);
        dir_config = g_strdup(default_config_dir);
    }

    res = stat(dir_config, &st);
    if (res != 0 || S_ISDIR(st.st_mode) == FALSE) {
        g_set_error(err, gfal2_get_config_quark(), EINVAL,
            " %s is not a valid directory for "
                "gfal2 configuration files, please specify %s properly",
            dir_config, config_env_var);
        g_free(dir_config);
        dir_config = NULL;
    }
    return dir_config;
}


int gfal_load_configuration_to_conf_manager(GConfigManager_t manager,
    const gchar *path, GError **err)
{
    GKeyFile *conf_file = g_key_file_new();
    int res = 0;
    GError *tmp_err1 = NULL, *tmp_err2 = NULL;
    if (g_key_file_load_from_file(conf_file, path, G_KEY_FILE_NONE, &tmp_err1) == FALSE) {
        res = -1;
        g_set_error(&tmp_err2, gfal2_get_config_quark(), EFAULT,
            "Error while loading configuration file %s : %s", path,
            tmp_err1->message);
        g_clear_error(&tmp_err1);
    }
    else {
        g_config_manager_prepend_keyvalue(manager, conf_file);
    }

    G_RETURN_ERR(res, tmp_err2, err);
}


gboolean is_config_dir(const char *conffile)
{
    char *p = NULL;
    if ((p = strstr(conffile, ".conf")) != NULL) {
        if (*(p + 5) == '\0') {
            return TRUE;
        }
    }
    return FALSE;
}


GConfigManager_t gfal2_init_config(GError **err)
{
    GError *tmp_err = NULL;
    gchar *dir_config = NULL;
    GConfigManager_t res = g_config_manager_new();
    if ((dir_config = check_configuration_dir(&tmp_err)) != NULL) {
        DIR *d = opendir(dir_config);
        struct dirent *dirinfo;
        if (d != NULL) {
            while ((dirinfo = readdir(d)) != NULL) {
                if (is_config_dir(dirinfo->d_name)) {
                    char *config_file = g_strdup_printf("%s/%s", dir_config, dirinfo->d_name);
                    gfal2_log(G_LOG_LEVEL_DEBUG, " try to load configuration file %s ...", config_file);
                    int rc = gfal_load_configuration_to_conf_manager(res, config_file, &tmp_err);
                    g_free(config_file);
                    if (rc != 0) {
                        break;
                    }
                }
            }
            closedir(d);
        }
        else {
            g_set_error(&tmp_err, gfal2_get_config_quark(), ENOENT, "Unable to open configuration directory %s",
                dir_config);
        }
        g_free(dir_config);
    }
    if (tmp_err) {
        g_config_manager_delete_full(res);
        res = NULL;
    }
    G_RETURN_ERR(res, tmp_err, err);
}


gchar *gfal2_get_opt_string(gfal2_context_t context, const gchar *group_name,
    const gchar *key, GError **error)
{
    g_assert(context != NULL);
    return g_config_manager_get_string(context->conf, group_name, key, error);
}


gchar *gfal2_get_opt_string_with_default(gfal2_context_t handle,
    const gchar *group_name, const gchar *key, const gchar *default_value)
{
    g_assert(handle != NULL);
    GError *tmp_err = NULL;

    gchar *value = gfal2_get_opt_string(handle, group_name, key, &tmp_err);
    if (tmp_err) {
        gfal2_log(G_LOG_LEVEL_DEBUG,
                "Impossible to get string parameter %s:%s, set to default value %s, err %s",
                group_name, key, default_value, tmp_err->message);
        g_clear_error(&tmp_err);
        value = g_strdup(default_value);
    }
    return value;
}


gint gfal2_set_opt_string(gfal2_context_t context, const gchar *group_name,
    const gchar *key, const gchar *value, GError **error)
{
    g_assert(context != NULL);
    return g_config_manager_set_string(context->conf, group_name, key, value, error);
}


gint gfal2_get_opt_integer(gfal2_context_t context, const gchar *group_name,
    const gchar *key, GError **error)
{
    g_assert(context != NULL);
    return g_config_manager_get_integer(context->conf, group_name, key, error);
}


gint gfal2_get_opt_integer_with_default(gfal2_context_t context,
    const gchar *group_name, const gchar *key, gint default_value)
{
    GError *tmp_err = NULL;

    gint res = gfal2_get_opt_integer(context, group_name, key, &tmp_err);
    if (tmp_err) {
        gfal2_log(G_LOG_LEVEL_DEBUG,
            "Impossible to get integer parameter %s:%s, set to default value %d, err %s",
            group_name, key, default_value, tmp_err->message);
        g_clear_error(&tmp_err);
        res = default_value;
    }
    return res;
}


gint gfal2_set_opt_integer(gfal2_context_t context, const gchar *group_name,
    const gchar *key, gint value, GError **error)
{
    g_assert(context != NULL);
    return g_config_manager_set_integer(context->conf, group_name, key, value, error);
}


gboolean gfal2_get_opt_boolean(gfal2_context_t context, const gchar *group_name,
    const gchar *key, GError **error)
{
    g_assert(context != NULL);
    return g_config_manager_get_boolean(context->conf, group_name, key, error);
}


gboolean gfal2_get_opt_boolean_with_default(gfal2_context_t context,
    const gchar *group_name, const gchar *key, gboolean default_value)
{
    GError *tmp_err = NULL;

    gboolean res = gfal2_get_opt_boolean(context, group_name, key, &tmp_err);
    if (tmp_err) {
        gfal2_log(G_LOG_LEVEL_DEBUG,
            "Impossible to get boolean parameter %s:%s, set to default value %s, err %s",
            group_name, key, ((default_value) ? "TRUE" : "FALSE"),
            tmp_err->message);
        g_clear_error(&tmp_err);
        res = default_value;
    }
    return res;
}


gint gfal2_set_opt_boolean(gfal2_context_t context, const gchar *group_name,
    const gchar *key, gboolean value, GError **error)
{
    g_assert(context != NULL);
    return g_config_manager_set_boolean(context->conf, group_name, key, value, error);
}


gchar **gfal2_get_opt_string_list(gfal2_context_t context,
    const gchar *group_name, const gchar *key, gsize *length,
    GError **error)
{
    g_assert(context != NULL);
    return g_config_manager_get_string_list(context->conf, group_name, key, length, error);
}


gint gfal2_set_opt_string_list(gfal2_context_t context, const gchar *group_name,
    const gchar *key, const gchar *const list[], gsize length,
    GError **error)
{
    g_assert(context != NULL);
    return g_config_manager_set_string_list(context->conf, group_name, key, list, length, error);
}


gchar **gfal2_get_opt_string_list_with_default(gfal2_context_t context,
    const gchar *group_name, const gchar *key, gsize *length,
    char **default_value)
{
    GError *tmp_err = NULL;

    gchar **res = gfal2_get_opt_string_list(context, group_name, key, length, &tmp_err);

    if (tmp_err) {
        if (gfal2_log_get_level() >= G_LOG_LEVEL_DEBUG) {
            gchar *list_default = g_strjoinv(",", default_value);
            gfal2_log(G_LOG_LEVEL_DEBUG,
                "Impossible to get string_list parameter %s:%s, set to a default value %s, err %s",
                group_name, key, list_default, tmp_err->message);
            g_free(list_default);
        }
        g_clear_error(&tmp_err);
        res = g_strdupv(default_value);
    }
    return res;
}


gint gfal2_load_opts_from_file(gfal2_context_t context, const char *path,
    GError **error)
{
    return gfal_load_configuration_to_conf_manager(context->conf, path, error);
}


gint gfal2_set_user_agent(gfal2_context_t handle, const char *user_agent,
    const char *version, GError **error)
{
    g_free(handle->agent_name);
    handle->agent_name = g_strdup(user_agent);
    g_free(handle->agent_version);
    handle->agent_version = g_strdup(version);
    return 0;
}


gint gfal2_get_user_agent(gfal2_context_t handle, const char **user_agent, const char **version)
{
    *user_agent = handle->agent_name;
    *version = handle->agent_version;
    return 0;
}


gint gfal2_add_client_info(gfal2_context_t handle, const char *key, const char *value, GError **error)
{
    gfal2_remove_client_info(handle, key, error);
    g_clear_error(error);

    gfal_key_value_t keyval = g_new0(struct _gfal_key_value, 1);
    keyval->key = g_strdup(key);
    keyval->value = g_strdup(value);
    g_ptr_array_add(handle->client_info, keyval);
    return 0;
}


gint gfal2_remove_client_info(gfal2_context_t handle, const char *key, GError **error)
{
    const char *value;
    int i = gfal2_get_client_info_value(handle, key, &value, error);
    if (i < 0) {
        return i;
    }

    gfal_key_value_t keyval = (gfal_key_value_t) g_ptr_array_index(handle->client_info, i);
    gfal_free_keyvalue(keyval, NULL);
    g_ptr_array_remove_index_fast(handle->client_info, i);

    return 0;
}


gint gfal2_clear_client_info(gfal2_context_t handle, GError **error)
{
    g_ptr_array_foreach(handle->client_info, gfal_free_keyvalue, NULL);
    g_ptr_array_free(handle->client_info, FALSE);
    handle->client_info = g_ptr_array_new();
    return 0;
}


gint gfal2_get_client_info_count(gfal2_context_t handle, GError **error)
{
    return handle->client_info->len;
}


gint gfal2_get_client_info_pair(gfal2_context_t handle, int index, const char **key,
    const char **value, GError **error)
{
    gfal_key_value_t keyval = (gfal_key_value_t) g_ptr_array_index(handle->client_info, index);
    if (keyval) {
        *key = keyval->key;
        *value = keyval->value;
    }
    else {
        *key = NULL;
        *value = NULL;
    }
    return 0;
}


gint gfal2_get_client_info_value(gfal2_context_t handle, const char *key,
    const char **value, GError **error)
{
    size_t i = 0;
    for (i = 0; i < handle->client_info->len; ++i) {
        gfal_key_value_t keyval = (gfal_key_value_t) g_ptr_array_index(handle->client_info, i);
        if (strcmp(keyval->key, key) == 0) {
            *value = keyval->value;
            return i;
        }
    }
    g_set_error(error, gfal2_get_config_quark(), EINVAL, "Key %s not found", key);
    return -1;
}


static char *gfal2_urlencode(const char *original)
{
    size_t len = strlen(original);
    char *encoded = g_malloc0(len * 3 + 1); // Worst case

    const char *ip; // input pointer
    char *op;      // output pointer
    for (ip = original, op = encoded; *ip; ++ip) {
        if (g_ascii_isalnum(*ip) || *ip == '.' || *ip == '-' || *ip == '_') {
            *op = *ip;
            ++op;
        }
        else {
            g_snprintf(op, 4, "%%%02X", (int) *ip);
            op += 3;
        }
    }

    return encoded;
}


char *gfal2_get_client_info_string(gfal2_context_t handle)
{
    size_t i, nitems = handle->client_info->len;
    if (nitems == 0) {
        return NULL;
    }

    char **entries = g_new0(char*, nitems + 1);

    for (i = 0; i < nitems; ++i) {
        gfal_key_value_t keyval = (gfal_key_value_t) g_ptr_array_index(handle->client_info, i);
        char *encoded_key = gfal2_urlencode(keyval->key);
        char *encoded_value = gfal2_urlencode(keyval->value);
        entries[i] = g_strdup_printf("%s=%s", encoded_key, encoded_value);
        g_free(encoded_key);
        g_free(encoded_value);
    }

    char *joined = g_strjoinv(";", entries);
    g_strfreev(entries);
    return joined;
}
