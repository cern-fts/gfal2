/*
 * Copyright (c) CERN 2016
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

#ifndef GFAL_SFTP_CONNECTION_H
#define GFAL_SFTP_CONNECTION_H

#include "gfal_sftp_plugin.h"
#include <libssh2.h>
#include <libssh2_sftp.h>

/// Wraps a connection plus a session to a remote SSH server
struct gfal_sftp_handle_s {
    int sock;
    LIBSSH2_SESSION *ssh_session;
    LIBSSH2_SFTP *sftp_session;
    const char *host;
    int port;
    const char *path;
};
typedef struct gfal_sftp_handle_s gfal_sftp_handle_t;

/// SSH session cache
struct gfal_sftp_handle_cache_s {
    GHashTable *caches;
};
typedef struct gfal_sftp_handle_cache_s gfal_sftp_handle_cache_t;

/// Plugin internal data
struct gfal_sftp_context_s {
    gfal2_context_t gfal2_context;
    GHashTable *cache;
};
typedef struct gfal_sftp_context_s gfal_sftp_context_t;

/// Translates an SSH error to a GError
/// @param func     The function that caused the error
/// @param handle   The handle used when the error happened. The error will be extracted from here.
/// @param[out] err This GError will be filled up with the error message and code
void gfal_plugin_sftp_translate_error(const char *func, gfal_sftp_handle_t *handle, GError **err);

/// Returns a new handle wrapping a connection to the remote endpoint
/// @param context  The SFTP context
/// @param url      Full URL (sftp://host:port/path) to which to connect
/// @param[out] err Any error will be put here
/// @return         NULL on error
gfal_sftp_handle_t *gfal_sftp_connect(gfal_sftp_context_t *context, const char *url, GError **err);

/// Releases a handle
/// @param context      The SFTP context
/// @param handle       The handle we are done with
void gfal_sftp_release(gfal_sftp_context_t *context, gfal_sftp_handle_t *handle);

/// Creates a new GHashTable that models a connection cache
GHashTable *gfal_sftp_cache_new();

/// Gets a handle from the cache
/// @param cache    An initialized cache
/// @param host     The remote host
/// @param port     The remote port
/// @return         NULL if there is no cached entry
gfal_sftp_handle_t *gfal_sftp_cache_pop(GHashTable *cache, const char *host, int port);

/// Puts a handle back to the cache
/// @param handle   The handle to release
void gfal_sftp_cache_push(GHashTable *cache, gfal_sftp_handle_t *handle);

/// Frees memory and closes connections
void gfal_sftp_cache_destroy(GHashTable *cache);


#endif // GFAL_SFTP_CONNECTION_H
