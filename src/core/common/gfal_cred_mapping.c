/*
 * Copyright (c) CERN 2017
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

#include <gfal_api.h>
#include <string.h>
#include "gfal_handle.h"


typedef struct {
    size_t prefix_len;
    char *url_prefix;
    gfal2_cred_t *cred;
} gfal2_cred_node_t ;


static gint node_compare(gconstpointer a, gconstpointer b)
{
    const gfal2_cred_node_t *node_a, *node_b;
    node_a = a;
    node_b = b;
    int comp = -strcmp(node_a->url_prefix, node_b->url_prefix);
    if (comp == 0) {
        return strcmp(node_a->cred->type, node_b->cred->type);
    }
    return comp;
}


static void node_copy(gpointer src, gpointer user_data)
{
    gfal2_context_t dest = user_data;
    const gfal2_cred_node_t *node = src;
    gfal2_cred_set(dest, node->url_prefix, node->cred, NULL);
}


static void node_free(gpointer ptr)
{
    gfal2_cred_node_t *node = ptr;
    g_free(node->url_prefix);
    gfal2_cred_free(node->cred);
    g_free(node);
}


gfal2_cred_t *gfal2_cred_new(const char* type, const char *value)
{
    gfal2_cred_t *cred = g_malloc0(sizeof(gfal2_cred_t));
    cred->type = g_strdup(type);
    cred->value = g_strdup(value);
    return cred;
}


void gfal2_cred_free(gfal2_cred_t *cred)
{
    if (cred) {
        g_free(cred->type);
        g_free(cred->value);
        g_free(cred);
    }
}


gfal2_cred_t* gfal2_cred_dup(const gfal2_cred_t *cred)
{
    return gfal2_cred_new(cred->type, cred->value);
}


int gfal2_cred_set(gfal2_context_t handle, const char *url_prefix, const gfal2_cred_t *cred, GError **error)
{
    gfal2_cred_node_t *node = g_malloc0(sizeof(gfal2_cred_node_t));
    node->prefix_len = strlen(url_prefix);
    node->url_prefix = g_strdup(url_prefix);
    node->cred = gfal2_cred_dup(cred);

    // Remove existing value
    GList *item = g_list_find_custom(handle->cred_mapping, node, node_compare);
    if (item) {
        gfal2_cred_node_t *match = item->data;
        node_free(match);
        handle->cred_mapping = g_list_delete_link(handle->cred_mapping, item);
    }

    // If cred is NULL, done
    if (cred == NULL) {
        node_free(node);
        return 0;
    }

    handle->cred_mapping = g_list_insert_sorted(handle->cred_mapping, node, node_compare);
    return 0;
}


char *gfal2_cred_get(gfal2_context_t handle, const char *type, const char *url, char const** baseurl, GError **error)
{
    size_t type_len = strlen(type);
    // Pick the first, which is the longest match, since the list is sorted
    GList *item;
    for (item = g_list_first(handle->cred_mapping); item != NULL; item = g_list_next(item)) {
        gfal2_cred_node_t *node = item->data;
        if (strncmp(node->cred->type, type, type_len) == 0 && strncmp(node->url_prefix, url, node->prefix_len) == 0) {
            if (baseurl) {
                *baseurl = (char const*)(node->url_prefix);
            }
            return g_strdup(node->cred->value);
        }
    }
    if (baseurl) {
        *baseurl = "";
    }
    // If there is no match, use the config
    if (strncmp(type, GFAL_CRED_X509_CERT, type_len) == 0) {
        return gfal2_get_opt_string_with_default(handle, "X509", "CERT", NULL);
    }
    if (strncmp(type, GFAL_CRED_X509_KEY, type_len) == 0) {
        return gfal2_get_opt_string_with_default(handle, "X509", "KEY", NULL);
    }
    return NULL;
}


int gfal2_cred_clean(gfal2_context_t handle, GError **error)
{
    g_list_free_full(handle->cred_mapping, node_free);
    handle->cred_mapping = NULL;
    return 0;
}


int gfal2_cred_copy(gfal2_context_t dest, const gfal2_context_t src, GError **error)
{
    if (gfal2_cred_clean(dest, error) != 0) {
        return -1;
    }
    g_list_foreach(src->cred_mapping, node_copy, dest);
    return 0;
}


typedef struct {
    gfal_cred_func_t callback;
    void *user_data;
} callback_data;


static void foreach_callback_wrapper(gpointer item, gpointer user_data)
{
    callback_data *data = user_data;
    gfal2_cred_node_t *node = item;
    data->callback(node->url_prefix, node->cred, data->user_data);
}


void gfal2_cred_foreach(gfal2_context_t handle, gfal_cred_func_t callback, void *user_data)
{
    callback_data data = {callback, user_data};
    g_list_foreach(handle->cred_mapping, foreach_callback_wrapper, &data);
}
