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

#include <glib.h>
#include <config/gfal_config_internal.h>
#include <logger/gfal_logger.h>
#include <stdio.h>
#include <string.h>
#include <common/gfal_common_plugin.h>
#include <gfal_api.h>
#include <common/gfal_common_dir_handle.h>

// initialization
__attribute__((constructor))
void core_init()
{
#if  (!GLIB_CHECK_VERSION (2, 32, 0))
    if (!g_thread_supported())
    g_thread_init(NULL);
#endif
}

GQuark gfal2_get_core_quark()
{
    return g_quark_from_static_string(GFAL2_QUARK_CORE);
}


GQuark gfal2_get_config_quark()
{
    return g_quark_from_static_string(GFAL2_QUARK_CONFIG);
}


GQuark gfal2_get_plugins_quark()
{
    return g_quark_from_static_string(GFAL2_QUARK_PLUGINS);
}

static void gfal_setCredentialLocation(const char *where, gfal2_context_t handle, const char *cert, const char *key)
{
    GError *error = NULL;
    gfal2_set_opt_string(handle, "X509", "CERT", cert, &error);
    g_clear_error(&error);
    gfal2_set_opt_string(handle, "X509", "KEY", key, &error);
    g_clear_error(&error);

    gfal2_log(G_LOG_LEVEL_DEBUG, "Using credentials from %s", where);
    gfal2_log(G_LOG_LEVEL_DEBUG, "Certificate: %s", cert);
    gfal2_log(G_LOG_LEVEL_DEBUG, "Private key: %s", key);
}

// Setup default credentials depending on the environment
static void gfal_initCredentialLocation(gfal2_context_t handle)
{
    // X509_USER_PROXY
    const char *proxy = getenv("X509_USER_PROXY");
    if (proxy != NULL) {
        gfal_setCredentialLocation("X509_USER_PROXY", handle, proxy, proxy);
        return;
    }
    // /tmp/x509up_u<uid>
    char default_proxy[64];
    snprintf(default_proxy, sizeof(default_proxy), "/tmp/x509up_u%d", getuid());
    if (access(default_proxy, F_OK) == 0) {
        gfal_setCredentialLocation("default proxy location", handle, default_proxy, default_proxy);
        return;
    }
    // X509_USER_CERT and X509_USER_KEY
    const char *cert, *key;
    cert = getenv("X509_USER_CERT");
    key = getenv("X509_USER_KEY");
    if (cert != NULL && key != NULL) {
        gfal_setCredentialLocation("X590_USER_CERT and X509_USER_KEY", handle, cert, key);
        return;
    }
    // Default certificate location
    const char *home = getenv("HOME");
    if (home != NULL) {
        size_t cert_path_length = strlen(home) + 22;
        char *default_cert = (char *) malloc(cert_path_length);
        char *default_key = (char *) malloc(cert_path_length);
        snprintf(default_cert, sizeof(default_cert), "%s/.globus/usercert.pem", home);
        snprintf(default_key, sizeof(default_key), "%s/.globus/userkey.pem", home);
        if (access(default_cert, F_OK) == 0 && access(default_key, F_OK) == 0) {
            gfal_setCredentialLocation("default certificate location", handle, default_cert, default_key);
            free(default_cert);
            free(default_key);
            return;
        }
        free(default_cert);
        free(default_key);
    }
    // No idea!
    gfal2_log(G_LOG_LEVEL_WARNING, "Could not find the credentials in any of the known locations");
}

gfal2_context_t gfal2_context_new(GError **err)
{
    GError *tmp_err = NULL;
    gfal2_context_t handle = g_new0(struct gfal_handle_, 1);// clear allocation of the struct and set defautl options
    if (handle == NULL) {
        errno = ENOMEM;
        g_set_error(err, gfal2_get_plugins_quark(), ENOMEM,
            "[%s] bad allocation, no more memory free", __func__);
        return NULL;
    }
    handle->plugin_opt.plugin_number = 0;

    if ((handle->conf = gfal_conf_new(&tmp_err)) && !tmp_err) {
        // credential location
        gfal_initCredentialLocation(handle);
        // load and instantiate all the plugins
        gfal_plugins_instance(handle, &tmp_err);
        // cancel logic init
        handle->cancel = FALSE;
        handle->running_ops = 0;
        handle->mux_cancel = g_mutex_new();
        g_hook_list_init(&handle->cancel_hooks, sizeof(GHook));
    }


    if (tmp_err) {
        if (handle && handle->conf) {
            gfal_conf_delete(handle->conf);
        }
        g_free(handle);
        handle = NULL;
    }
    else {
        handle->client_info = g_ptr_array_new();
    }

    G_RETURN_ERR(handle, tmp_err, err);
}


void gfal2_context_free(gfal2_context_t context)
{
    if (context == NULL) {
        return;
    }
    gfal_plugins_delete(context, NULL);
    gfal_dir_handle_container_delete(&(context->fdescs));
    gfal_conf_delete(context->conf);
    g_list_free(context->plugin_opt.sorted_plugin);
    g_mutex_free(context->mux_cancel);
    g_hook_list_clear(&context->cancel_hooks);
    g_free(context->agent_name);
    g_free(context->agent_version);
    g_ptr_array_foreach(context->client_info, gfal_free_keyvalue, NULL);
    g_free(context);
    context = NULL;
}


gchar **gfal2_get_plugin_names(gfal2_context_t context)
{
    gchar **array = g_new0(gchar*, context->plugin_opt.plugin_number + 1);
    int i;

    for (i = 0; i < context->plugin_opt.plugin_number; ++i) {
        array[i] = g_strdup(context->plugin_opt.plugin_list[i].getName());
    }
    array[i] = NULL;

    return array;
}

// return a string of the current gfal version
const char *gfal2_version()
{
    return VERSION;
}