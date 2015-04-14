/*
* Copyright @ CERN, 2015.
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

/**
 * This implements a GridFTP client plugin that listen to PASV events, so it can
 * trigger gfal2 events and pass outside the information before the connection happens
 */

#ifndef GRIDFTPPASVPLUGIN_H
#define GRIDFTPPASVPLUGIN_H

#include <gfal_api.h>
#include <globus_ftp_client_plugin.h>

class GridFTPSession;

/**
 * Initialize the PASV plugin
 */
globus_result_t gfal2_ftp_client_pasv_plugin_init(globus_ftp_client_plugin_t* plugin,
        GridFTPSession* session);

#endif // GRIDFTPPASVPLUGIN_H
