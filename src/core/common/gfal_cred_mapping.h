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

#ifndef GFAL2_GFAL_CRED_MAPPING_H
#define GFAL2_GFAL_CRED_MAPPING_H

#include "gfal_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Predefined credential types
 * Some plugins may define their own
 */

/// No credential set
#define GFAL_CRED_NONE NULL
/// X509 user certificate, or proxy certificate
#define GFAL_CRED_X509_CERT "X509_CERT"
/// X509 user private key
#define GFAL_CRED_X509_KEY "X509_KEY"
/// User, usually combined with GFAL_CRED_PASSWD
#define GFAL_CRED_USER "USER"
/// Password, usually combined with GFAL_CRED_USER
#define GFAL_CRED_PASSWD "PASSWORD"
/// Bearer token-type credential
#define GFAL_CRED_BEARER "BEARER"

/**
 * Stores a credential value together with its type
 * The specific value depends on the type.
 * @note Use gfal2_cred_new to create a new instance
 */
typedef struct {
    char *type;
    char *value;
} gfal2_cred_t;

/**
 * Callback type for gfal2_cred_foreach
 */
typedef void (*gfal_cred_func_t)(const char *url_prefix, const gfal2_cred_t *cred, void *user_data);

/**
 * Create a new gfal2_cred_t
 * @return An initialized gfal2_cred_t
 */
gfal2_cred_t *gfal2_cred_new(const char* type, const char *value);

/**
 * Release a gfal2_cred_t instance
  */
void gfal2_cred_free(gfal2_cred_t *cred);

/**
 * Duplicate a gfal2_cred_t instance
 */
gfal2_cred_t *gfal2_cred_dup(const gfal2_cred_t *cred);

/**
 * Set a credential for a given url prefix
 * @param handle        The gfal2 context
 * @param url_prefix    The URL prefix
 * @param cred          The credential to use for this prefix
 * @param error         In case of error
 * @return              0 on success, -1 on error
 * @note                The empty prefix is initialized by default with the environment X509_USER_* variables
 *                      or the [X509] configuration
 * @note                It will store its own copy of url_prefix and cred
 */
int gfal2_cred_set(gfal2_context_t handle, const char *url_prefix, const gfal2_cred_t *cred, GError **error);

/**
 * Get a credential for a given url
 * @param handle        The gfal2 context
 * @param type          Credential type
 * @param url           Full URL. Best matching prefix will be picked.
 * @param baseurl       If not NULL, the chosen base url will be put here.
 * @param error         In case of error
 * @return              A credential suitable for the given url. NULL if nothing has been found. Remember to g_free it.
 */
char *gfal2_cred_get(gfal2_context_t handle, const char *type, const char *url, char const** baseurl, GError **error);

/**
 * Get a credential for a given url.
 * This search acts in reserve, returning the first credential whose path fully includes the search url.
 * @param handle        The gfal2 context
 * @param type          Credential type
 * @param url           Full URL. First including prefix will be picked.
 * @param baseurl       If not NULL, the chose base url will be put here.
 * @param error         In case of error
 * @return              A credential suitable for the given url. NULL if nothing has been found. Remember to g_free it.
 */
char *gfal2_cred_get_reverse(gfal2_context_t handle, const char *type, const char *url,
                             char const** baseurl, GError **error);

/**
 * Remove the credential for a given type and url
 * @param handle        The gfal2 context
 * @param type          Credential type
 * @param url           Full URL. Only exact matching URL will be deleted
 * @param error         In case of error
 * @return              0 on success, -1 on error
 */
int gfal2_cred_del(gfal2_context_t handle, const char *type, const char *url, GError **error);

/**
 * Remove all loaded credentials
 * @param handle        The gfal2 context
 * @param error         In case of error
 * @return              0 on success, -1 on error
 */
int gfal2_cred_clean(gfal2_context_t handle, GError **error);

/**
 * Copy the credential list from one context to another
 * @param dest          Destination gfal2 context
 * @param src           Source gfal2 context
 * @param error         In case of error
 * @return              0 on success, -1 on error
 */
int gfal2_cred_copy(gfal2_context_t dest, const gfal2_context_t src, GError **error);

/**
 * Iterate over all registered credentials
 * @param handle        The gfal2 context
 * @param callback      Callback for each item
 * @param user_data     To be passed to the callback
 */
void gfal2_cred_foreach(gfal2_context_t handle, gfal_cred_func_t callback, void *user_data);

#ifdef __cplusplus
}
#endif

#endif //GFAL2_GFAL_CRED_MAPPING_H
