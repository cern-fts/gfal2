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
#include "gridftpmodule.h"
#include "gridftpwrapper.h"

#include <globus_gass_copy.h>
#include <globus_ftp_client.h>
#include <globus_ftp_client_debug_plugin.h>
#include <globus_ftp_client_throughput_plugin.h>


//GOnce my_once = G_ONCE_INIT;

static const GQuark GFAL_GRIDFTP_SCOPE_GLOBUS_INIT = g_quark_from_static_string("GridFTPModule::init_globus");

// initialization
__attribute__((constructor))
void gridftp_plugin_init()
{
#if  (!GLIB_CHECK_VERSION (2, 32, 0))
    if (!g_thread_supported())
    g_thread_init(NULL);
#endif

    if (!getenv("GLOBUS_THREAD_MODEL"))
        globus_thread_set_model("pthread");

    globus_module_activate(GLOBUS_GSI_GSS_ASSIST_MODULE);
    globus_module_activate(GLOBUS_GSI_GSSAPI_MODULE);
}


GridFTPModule::GridFTPModule(GridFTPFactory* factory)
{
    _handle_factory = factory;

    globus_module_activate(GLOBUS_GASS_COPY_MODULE);
    globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
    globus_module_activate(GLOBUS_FTP_CLIENT_DEBUG_PLUGIN_MODULE);
    globus_module_activate(GLOBUS_FTP_CLIENT_THROUGHPUT_PLUGIN_MODULE);
}


GridFTPModule::~GridFTPModule()
{
    delete _handle_factory;

    globus_module_deactivate(GLOBUS_GASS_COPY_MODULE);
    globus_module_deactivate(GLOBUS_FTP_CLIENT_MODULE);
    globus_module_deactivate(GLOBUS_FTP_CLIENT_DEBUG_PLUGIN_MODULE);
    globus_module_deactivate(GLOBUS_FTP_CLIENT_THROUGHPUT_PLUGIN_MODULE);
}



