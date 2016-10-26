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

#ifndef GFAL_XROOTD_PLUGIN_MAIN_H_
#define GFAL_XROOTD_PLUGIN_MAIN_H_

#include <gfal_plugins_api.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Entry point of the plugin
gfal_plugin_interface gfal_plugin_init(gfal_handle context, GError** err);


#ifdef __cplusplus
}
#endif

#endif /* GFAL_XROOTD_PLUGIN_MAIN_H_ */
