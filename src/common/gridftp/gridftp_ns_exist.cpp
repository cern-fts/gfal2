/*
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gridftp_namespace.h"


static const Glib::Quark gfal_gridftp_scope_exist("GridftpModule::file_exist");


bool gridftp_module_file_exist(plugin_handle handle, const char* name)
{
	gfal_log(GFAL_VERBOSE_TRACE,"   -> [gridftp_module_file_exist]");
	
	struct stat buff;
	GError *stat_error = NULL;
	
	bool exists = false;

	if (gfal_gridftp_statG(handle, name, &buff, &stat_error) == 0) {
	    exists = true;
	}
	else if (stat_error->code == ENOENT) {
	    g_error_free(stat_error);
	}
	else {
        throw Gfal::CoreException(gfal_gridftp_scope_exist, stat_error->message,
                                  stat_error->code);
	}

    gfal_log(GFAL_VERBOSE_TRACE,"   <- [gridftp_module_file_exist]");
    return exists;
}
