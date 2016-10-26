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

struct gfal_sftp_handle_s {
    int sock;
    LIBSSH2_SESSION *ssh_session;
    LIBSSH2_SFTP *sftp_session;
    const char *path;
};
typedef struct gfal_sftp_handle_s gfal_sftp_handle_t;

void gfal_plugin_sftp_translate_error(const char *func, gfal_sftp_handle_t *handle, GError **err);

gfal_sftp_handle_t *gfal_sftp_connect(gfal_sftp_data_t *data, const char *path, GError **err);
void gfal_sftp_release(gfal_sftp_data_t *data, gfal_sftp_handle_t *sftp_handle);

#endif // GFAL_SFTP_CONNECTION_H
