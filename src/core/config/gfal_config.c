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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include <config/gfal_config_internal.h>
#include <common/gfal_common_err_helpers.h>
#include <logger/gfal_logger.h>

#ifndef GFAL_CONFIG_DIR_DEFAULT
#error "GFAL_CONFIG_DIR_DEFAULT should be define at compile time"
#endif

const gchar* config_env_var = GFAL_CONFIG_DIR_ENV;
const gchar* default_config_dir =  GFAL_CONFIG_DIR_DEFAULT
                                    "/"
                                    GFAL_CONFIG_DIR_SUFFIX
                                    "/";

gfal_conf_t gfal_handle_to_conf(gfal2_context_t h){
    return h->conf;
}

static gchar* check_configuration_dir(GError ** err){
    struct stat st;
    int res;
    gchar* dir_config = NULL;
    const gchar * env_str = g_getenv(config_env_var);
    if(env_str != NULL){
        gfal_log(GFAL_VERBOSE_TRACE, " %s env var found, try to load configuration from %s", config_env_var, env_str);
        dir_config = g_strdup(env_str);
    }else{
        gfal_log(GFAL_VERBOSE_TRACE, " no %s env var found, try to load configuration from default directory %s",
                                    config_env_var, default_config_dir);
        dir_config = g_strdup(default_config_dir);
    }

    res = stat(dir_config, &st);
    if( res != 0 || S_ISDIR(st.st_mode) == FALSE){
        g_set_error(err, gfal2_get_config_quark(),EINVAL, " %s is not a valid directory for "
                        "gfal2 configuration files, please specify %s properly", dir_config, config_env_var);
        g_free(dir_config);
        dir_config = NULL;
    }
    return dir_config;
}

int gfal_load_configuration_to_conf_manager(GConfigManager_t manager, const gchar * path, GError ** err){
    GKeyFile * conf_file = g_key_file_new();
    int res = 0;
    GError * tmp_err1=NULL, *tmp_err2=NULL;
    if(g_key_file_load_from_file (conf_file, path, G_KEY_FILE_NONE, &tmp_err1) == FALSE){
           res = -1;
           g_set_error(&tmp_err2, gfal2_get_config_quark(), EFAULT, "Error while loading configuration file %s : %s", path, tmp_err1->message);
           g_clear_error(&tmp_err1);
    }else{
        g_config_manager_prepend_keyvalue(manager, conf_file);
    }

    G_RETURN_ERR(res, tmp_err2, err);
}

gboolean is_config_dir(const char* conffile){
    char* p=NULL;
    if( (p = strstr(conffile, ".conf")) != NULL){
        if( *(p+5) == '\0')
            return TRUE;
    }
    return FALSE;
}

GConfigManager_t gfal_load_static_configuration(GError ** err){
    GError * tmp_err=NULL;
    gchar* dir_config = NULL;
    GConfigManager_t res = g_config_manager_new();
    if( (dir_config = check_configuration_dir(&tmp_err)) != NULL){
        DIR* d = opendir(dir_config);
        struct dirent* dirinfo;
        if(d != NULL){
            while( (dirinfo = readdir(d)) != NULL){
                if( is_config_dir(dirinfo->d_name)){
                    char buff[strlen(dir_config) + strlen(dirinfo->d_name) + 2];
                    strcpy(buff, dir_config);
                    strcat(buff,"/");
                    strcat(buff, dirinfo->d_name);

                    gfal_log(GFAL_VERBOSE_TRACE, " try to load configuration file %s ...", buff);
                    if(gfal_load_configuration_to_conf_manager(res, buff, &tmp_err) != 0)
                        break;
                }
            }
            closedir(d);
        }else{
            g_set_error(&tmp_err, gfal2_get_config_quark(), ENOENT, "Unable to open configuration directory %s", dir_config);
        }
        g_free(dir_config);
    }
    if(tmp_err){
        g_config_manager_delete_full(res);
        res = NULL;
    }
    G_RETURN_ERR(res, tmp_err, err);
}

gfal_conf_t gfal_conf_new(GError ** err){
    GError * tmp_err=NULL;

	gfal_conf_t res = g_new0(struct _gfal_conf, 1);
    res->running_config= g_key_file_new();
    g_key_file_load_from_data(res->running_config, " ", 1, G_KEY_FILE_NONE, NULL);
    res->running_manager = g_config_manager_new();
    res->static_manager= gfal_load_static_configuration(&tmp_err);
	//g_mutex_init(&(res->mux));
    if(tmp_err){
        gfal_conf_delete(res);
        res = NULL;
    }else{
        // add the running config to the configuration manager
        g_config_manager_prepend_manager(res->running_manager, res->static_manager);
        g_config_manager_prepend_keyvalue(res->running_manager, res->running_config);
    }
    G_RETURN_ERR(res, tmp_err, err);
}

void gfal_conf_delete(gfal_conf_t conf){
	if(conf){
	//	g_mutex_clear(&(conf->mux));
        g_config_manager_delete(conf->static_manager);
        g_config_manager_delete_full(conf->running_manager);
		g_free(conf);
	}
}

void gfal_config_propagate_error_external(GError ** err, GError ** tmp_err){
    if(tmp_err && *tmp_err){
        g_set_error(err, gfal2_get_config_quark(), EINVAL, " Configuration Error : %s", (*tmp_err)->message);
        g_clear_error(tmp_err);
    }
}


gchar * gfal2_get_opt_string(gfal2_context_t handle, const gchar *group_name,
                                    const gchar *key, GError **error){
    g_assert(handle != NULL);
    GError * tmp_err=NULL;
    gfal_conf_t c = gfal_handle_to_conf(handle);

    gchar * res = g_config_manager_get_string(c->running_manager, group_name, key, &tmp_err);
    gfal_config_propagate_error_external(error, &tmp_err);
    return res;
}

gint gfal2_set_opt_string(gfal2_context_t handle, const gchar *group_name,
                                    const gchar *key, gchar* value, GError **error){
    g_assert(handle != NULL);
    GError * tmp_err=NULL;
    gfal_conf_t c = gfal_handle_to_conf(handle);

    gint res = g_config_manager_set_string(c->running_manager, group_name, key, value, &tmp_err);
    gfal_config_propagate_error_external(error, &tmp_err);
    return res;
}

gint gfal2_get_opt_integer(gfal2_context_t handle, const gchar *group_name,
                                 const gchar *key, GError **error){
    g_assert(handle != NULL);
    GError * tmp_err=NULL;
    gfal_conf_t c = gfal_handle_to_conf(handle);

    gint res = g_config_manager_get_integer(c->running_manager, group_name, key, &tmp_err);
    gfal_config_propagate_error_external(error, &tmp_err);
    return res;
}

gint gfal2_get_opt_integer_with_default(gfal2_context_t context, const gchar *group_name,
                                        const gchar *key, gint default_value){
    GError * tmp_err=NULL;

    gint res = gfal2_get_opt_integer(context, group_name, key, &tmp_err);
    if(tmp_err){
        gfal_log(GFAL_VERBOSE_DEBUG, " impossible to get integer parameter %s:%s, set to default value %d, err %s", group_name, key,
                        default_value, tmp_err->message );
        g_clear_error(&tmp_err);
        res = default_value;
    }
    return res;
}

gint gfal2_set_opt_integer(gfal2_context_t handle, const gchar *group_name,
                                  const gchar *key, gint value,
                                  GError** error){
    g_assert(handle != NULL);
    GError * tmp_err=NULL;
    gfal_conf_t c = gfal_handle_to_conf(handle);

    gint res = g_config_manager_set_integer(c->running_manager, group_name, key, value, &tmp_err);
    gfal_config_propagate_error_external(error, &tmp_err);
    return res;
}



gboolean gfal2_get_opt_boolean(gfal2_context_t handle, const gchar *group_name,
                                        const gchar *key, GError **error){
    g_assert(handle != NULL);
    GError * tmp_err=NULL;
    gfal_conf_t c = gfal_handle_to_conf(handle);

    gboolean res = g_config_manager_get_boolean(c->running_manager, group_name, key, &tmp_err);
    gfal_config_propagate_error_external(error, &tmp_err);
    return res;
}

gboolean gfal2_get_opt_boolean_with_default(gfal2_context_t handle, const gchar *group_name,
                                           const gchar *key, gboolean default_value){
    GError * tmp_err=NULL;

    gboolean res = gfal2_get_opt_boolean(handle, group_name, key, &tmp_err);
    if(tmp_err){
        gfal_log(GFAL_VERBOSE_DEBUG, " impossible to get boolean parameter %s:%s, set to default value %s, err %s",group_name, key,
                        ((default_value)?"TRUE":"FALSE"), tmp_err->message );
        g_clear_error(&tmp_err);
        res = default_value;
    }
    return res;
}

gint gfal2_set_opt_boolean(gfal2_context_t handle, const gchar *group_name,
                                  const gchar *key, gboolean value, GError **error){
    g_assert(handle != NULL);
    GError * tmp_err=NULL;
    gfal_conf_t c = gfal_handle_to_conf(handle);

    gint res = g_config_manager_set_boolean(c->running_manager, group_name, key, value, &tmp_err);
    gfal_config_propagate_error_external(error, &tmp_err);
    return res;
}

gchar ** gfal2_get_opt_string_list(gfal2_context_t handle, const gchar *group_name,
                                          const gchar *key, gsize *length, GError **error){
    g_assert(handle != NULL);
    GError * tmp_err=NULL;
    gfal_conf_t c = gfal_handle_to_conf(handle);

    gchar** res = g_config_manager_get_string_list(c->running_manager, group_name, key, length, &tmp_err);
    gfal_config_propagate_error_external(error, &tmp_err);
    return res;
}

gint gfal2_set_opt_string_list(gfal2_context_t handle, const gchar *group_name,
                                     const gchar *key,
                                     const gchar * const list[],
                                     gsize length,
                                     GError ** error){
    g_assert(handle != NULL);
    GError * tmp_err=NULL;
    gfal_conf_t c = gfal_handle_to_conf(handle);

    gint res  = g_config_manager_set_string_list(c->running_manager, group_name, key, list, length, &tmp_err);
    gfal_config_propagate_error_external(error, &tmp_err);
    return res;
}


gchar ** gfal2_get_opt_string_list_with_default(gfal2_context_t handle, const gchar *group_name,
                                          const gchar *key, gsize *length, char** default_value){
    GError * tmp_err=NULL;

    gchar** res = gfal2_get_opt_string_list(handle, group_name, key, length, &tmp_err);
    if(tmp_err){

        if(gfal_get_verbose() >= GFAL_VERBOSE_DEBUG){
            gchar* list_default = g_strjoinv (",", default_value);
            gfal_log(GFAL_VERBOSE_DEBUG, " impossible to get string_list parameter %s:%s, set to a default value %s, err %s",group_name, key,
                            list_default, tmp_err->message );
            g_free(list_default);
        }
        g_clear_error(&tmp_err);
        res = g_strdupv(default_value);
    }
    return res;
}


gint gfal2_load_opts_from_file(gfal2_context_t handle, const char* path, GError** error)
{
    gfal_conf_t c = gfal_handle_to_conf(handle);
    GError* tmp_err = NULL;
    int res = gfal_load_configuration_to_conf_manager(c->running_manager, path, &tmp_err);
    gfal_config_propagate_error_external(error, &tmp_err);
    return res;
}
