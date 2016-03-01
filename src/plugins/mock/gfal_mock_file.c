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

#include "gfal_mock_plugin.h"


gfal_file_handle gfal_plugin_mock_open(plugin_handle plugin_data, const char *url, int flag, mode_t mode, GError **err)
{

}


ssize_t gfal_plugin_mock_read(plugin_handle plugin_data, gfal_file_handle fd, void *buff, size_t count, GError **err)
{

}

ssize_t gfal_plugin_mock_write(plugin_handle plugin_data, gfal_file_handle fd, const void *buff, size_t count,
    GError **err)
{

}


int gfal_plugin_mock_close(plugin_handle plugin_data, gfal_file_handle fd, GError **err)
{

}


off_t gfal_plugin_mock_seek(plugin_handle plugin_data, gfal_file_handle fd, off_t offset, int whence, GError **err)
{

}
