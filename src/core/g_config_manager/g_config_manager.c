#include "g_config_manager.h"
#include "common/gfal_common_errverbose.h"

struct _GConfigManager{
    GList* configs;
};

struct ConfigFuncArgs{
    const gchar *group_name;
    const gchar *key;
    gsize* length;
    union{
        const gchar* mstring;
        gint mint;
        gboolean mbool;
        gchar** mstrlst;
    } value;
};

const gchar * g_config_manager_no_config_msg = "No configuration are loaded inside this configuration manager";

GConfigManager_t g_config_manager_new(){
    GConfigManager_t res = g_new0(struct _GConfigManager,1);
    return res;
}


GQuark g_config_manager_quark(){
    return g_quark_from_static_string ("g_config_manager::GKeyFileError");
}

void config_manager_destroyer(gpointer data){
    g_key_file_free((GKeyFile*)data);
}

gboolean check_kv_exist(GConfigManager_t config_manager){
    return (config_manager->configs != NULL);
}

gboolean check_kv_exist_error(GConfigManager_t config_manager, GError ** err){
    gboolean res;
    if( (res = check_kv_exist(config_manager) )== FALSE){
        g_set_error(err, g_config_manager_quark(), G_CONFIG_MANAGER_NO_CONFIG, "%s", g_config_manager_no_config_msg);
    }
    return res;
}

void g_config_manager_prepend_manager(GConfigManager_t config_manager, GConfigManager_t new_config){
    g_assert(config_manager != NULL);
    g_assert(new_config != NULL);
    if(new_config->configs){
        config_manager->configs = g_list_concat( config_manager->configs, g_list_copy(new_config->configs));
    }
}


void g_config_manager_prepend_keyvalue(GConfigManager_t config_manager, GKeyFile * keyvalue_config){
    g_assert(config_manager != NULL);
    config_manager->configs = g_list_prepend(config_manager->configs, keyvalue_config);
}

void g_config_manager_delete(GConfigManager_t config_manager){
    if(config_manager){
        g_list_free(config_manager->configs);
        g_free(config_manager);
    }
}

void g_config_manager_delete_full(GConfigManager_t config_manager){
    if(config_manager){
        g_list_free_full(config_manager->configs, &config_manager_destroyer);
        g_free(config_manager);
    }
}

typedef gpointer (*GConfigFunction)(GKeyFile* keyfile, gpointer userdata, GError** err);

gpointer g_config_manager_find_first_valid(GConfigManager_t config_manager, GConfigFunction func,
                                    gpointer userdata, GError ** err){
    g_assert(config_manager != NULL);
    GError* tmp_err=NULL;
    GList* list = g_list_first(config_manager->configs);
    while(list != NULL){
        gpointer res = func((GKeyFile*) list->data, userdata, &tmp_err);
        if(tmp_err){
                if(g_error_matches(tmp_err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND)
                              || g_error_matches(tmp_err, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND) ){
                    g_clear_error(&tmp_err);
                }else{
                    g_propagate_error(err, tmp_err);
                    return NULL;
                }
         }else{
            return res;
         }
        list = g_list_next(list);
    }
    g_set_error(err, g_config_manager_quark(), G_CONFIG_MANAGER_NOT_FOUND, "No configuration parameter with this value found");
    return NULL;
}

gpointer g_config_manager_exec_on_first(GConfigManager_t config_manager, GConfigFunction func,
                                    gpointer userdata, GError ** err){
    g_assert(config_manager != NULL);
    if (check_kv_exist_error(config_manager, err)){
        return func((GKeyFile*) config_manager->configs->data, userdata, err);
    }
    return NULL;
}

static void propagate_config_manager_error(GError ** err, GError * tmp_err){
    if(tmp_err){
            g_propagate_error(err, tmp_err);

    }
}

gpointer GConfigFunction_get_string(GKeyFile* keyfile, gpointer userdata, GError** err){
    struct ConfigFuncArgs *args = userdata;
    GError * tmp_err=NULL;
    gchar * res = g_key_file_get_string(keyfile,
                                        args->group_name,
                                        args->key,
                                        &tmp_err);
    propagate_config_manager_error(err, tmp_err);
    return res;
}


gchar * g_config_manager_get_string(GConfigManager_t config_manager,
                                    const gchar *group_name,
                                    const gchar *key,
                                    GError **error){

    struct ConfigFuncArgs args = { .group_name = group_name, .key = key };
    return (char*) g_config_manager_find_first_valid( config_manager, &GConfigFunction_get_string,
                                        &args, error);
}

gpointer GConfigFunction_get_string_list(GKeyFile* keyfile, gpointer userdata, GError** err){
    struct ConfigFuncArgs *args = userdata;
    GError * tmp_err=NULL;
    gchar ** res = g_key_file_get_string_list(keyfile,
                                        args->group_name,
                                        args->key,
                                        args->length,
                                        &tmp_err);
    propagate_config_manager_error(err, tmp_err);
    return res;
}

gchar ** g_config_manager_get_string_list(GConfigManager_t config_manager,const gchar *group_name,
                                          const gchar *key, gsize *length, GError **error){
    struct ConfigFuncArgs args = { .group_name = group_name, .key = key, .length = length };
    return (gchar**) g_config_manager_find_first_valid( config_manager, &GConfigFunction_get_string_list,
                                        &args, error);
}

gpointer GConfigFunction_set_string_list(GKeyFile* keyfile, gpointer userdata, GError** err){
    struct ConfigFuncArgs *args = userdata;
    g_key_file_set_string_list(keyfile,
                                        args->group_name,
                                        args->key,
                                        (const gchar * const*) args->value.mstrlst,
                                        *(args->length));
    return NULL;
}

gint g_config_manager_set_string_list(GConfigManager_t config_manager, const gchar *group_name,
                                     const gchar *key,
                                     const gchar * const list[],
                                     gsize length,
                                     GError ** error){
    GError * tmp_err=NULL;
    struct ConfigFuncArgs args = { .group_name = group_name, .key = key, .value = {.mstrlst = (gchar**) list }, .length = &length };
    g_config_manager_find_first_valid( config_manager, &GConfigFunction_set_string_list,
                                        &args, error);
    if(tmp_err){
        g_propagate_error(error, tmp_err);
        return -1;
    }
    return 0;
}


gpointer GConfigFunction_set_string(GKeyFile* keyfile, gpointer userdata, GError** err){
    struct ConfigFuncArgs *args = userdata;
    g_key_file_set_string(keyfile,
                            args->group_name,
                            args->key,
                            args->value.mstring);
    return NULL;
}

gint g_config_manager_set_string(GConfigManager_t config_manager,const gchar *group_name,
                                    const gchar *key, const gchar *string, GError** error){

    struct ConfigFuncArgs args = { .group_name = group_name, .key = key, .value  = { .mstring = string } };
    GError * tmp_err=NULL;
    g_config_manager_exec_on_first( config_manager, &GConfigFunction_set_string,
                                          &args, &tmp_err);
    if(tmp_err){
        g_propagate_error(error, tmp_err);
        return -1;
    }
    return 0;
}

gpointer GConfigFunction_get_int(GKeyFile* keyfile, gpointer userdata, GError** err){
    struct ConfigFuncArgs *args = userdata;
    GError * tmp_err=NULL;
    int res = g_key_file_get_integer(keyfile,
                                        args->group_name,
                                        args->key,
                                        &tmp_err);
    propagate_config_manager_error(err, tmp_err);
    return GINT_TO_POINTER(res);
}


gint g_config_manager_get_integer(GConfigManager_t config_manager, const gchar *group_name,
                                 const gchar *key, GError **error){
    struct ConfigFuncArgs args = { .group_name = group_name, .key = key };
    int res =  GPOINTER_TO_INT(g_config_manager_find_first_valid( config_manager, &GConfigFunction_get_int,
                                        &args, error) );
    return res;
}

gpointer GConfigFunction_set_int(GKeyFile* keyfile, gpointer userdata, GError** err){
    struct ConfigFuncArgs *args = userdata;
    g_key_file_set_integer(keyfile,
                            args->group_name,
                            args->key,
                            args->value.mint);
    return NULL;
}

gint g_config_manager_set_integer(GConfigManager_t config_manager, const gchar *group_name,
                                  const gchar *key, gint value,
                                  GError** error){
    GError * tmp_err=NULL;
    struct ConfigFuncArgs args = { .group_name = group_name, .key = key, .value = { .mint = value} };
    g_config_manager_exec_on_first( config_manager, &GConfigFunction_set_int,
                                          &args, &tmp_err);
    if(tmp_err){
        g_propagate_error(error, tmp_err);
        return -1;
    }
    return 0;
}

gpointer GConfigFunction_get_boolean(GKeyFile* keyfile, gpointer userdata, GError** err){
    struct ConfigFuncArgs *args = userdata;
    GError * tmp_err=NULL;
    int res = g_key_file_get_boolean(keyfile,
                                        args->group_name,
                                        args->key,
                                        &tmp_err);
    propagate_config_manager_error(err, tmp_err);
    return GINT_TO_POINTER(res);
}


gboolean g_config_manager_get_boolean(GConfigManager_t config_manager, const gchar *group_name,
                                        const gchar *key, GError **error){
    struct ConfigFuncArgs args = { .group_name = group_name, .key = key };
    gboolean res =  GPOINTER_TO_INT(g_config_manager_find_first_valid( config_manager, &GConfigFunction_get_boolean,
                                        &args, error) );
    return res;
}

gpointer GConfigFunction_set_boolean(GKeyFile* keyfile, gpointer userdata, GError** err){
    struct ConfigFuncArgs *args = userdata;
     g_key_file_set_boolean(keyfile,
                                        args->group_name,
                                        args->key,
                                        args->value.mbool);
    return NULL;
}

gint g_config_manager_set_boolean(GConfigManager_t config_manager, const gchar *group_name,
                                  const gchar *key, gboolean value, GError **error){
    GError * tmp_err=NULL;
    struct ConfigFuncArgs args = { .group_name = group_name, .key = key, .value = { .mbool = value} };
    g_config_manager_exec_on_first( config_manager, &GConfigFunction_set_boolean,
                                          &args, &tmp_err);
    if(tmp_err){
        g_propagate_error(error, tmp_err);
        return -1;
    }
    return 0;
}

