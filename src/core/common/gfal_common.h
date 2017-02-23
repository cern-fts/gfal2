/*
 * Copyright (c) CERN 2013-2017
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

#pragma once
#ifndef GFAL_GLOBAL_H_
#define GFAL_GLOBAL_H_

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif

#include <glib.h>
#include "gfal_deprecated.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * A gfal2 context is a separated instance of the gfal2 library
 * Each context owns his parameters, file descriptors
 * Context allows to have separated instance of GFAL with different parameters
 * providing an advanced interface to  GFAL
 */
typedef struct gfal_handle_* gfal2_context_t;

/**
 * @brief Create a gfal2 context
 *
 * Each context contain its own set of parameters and configurations ( \ref config_group )
 * A context can be used in multiple threads at the same time ( Thread-safe ).
 *
 * @param err : GError error report system
 * @return a context if success, NULL if error
 */
gfal2_context_t gfal2_context_new(GError ** err);

/**
 *  Free a gfal2 context
 *  It is safe to delete a NULL context
 */
void gfal2_context_free(gfal2_context_t context);

/**
 * Get list of loaded plugins
 * The returned list must be freed using g_strfreev
 */
gchar** gfal2_get_plugin_names(gfal2_context_t context);

/** For errors, gfal2 core quark */
#define GFAL2_QUARK_CORE "GFAL2::CORE"
/** For errors, gfal2 configuration quark */
#define GFAL2_QUARK_CONFIG "GFAL2::CONFIG"
/** For errors, gfal2 plugins quark */
#define GFAL2_QUARK_PLUGINS "GFAL2::PLUGINS"

/**
 * GQuark for the gfal2 core namespace
 * GQuark are used by the GError gfal2 error system in order to determine the scope of one error
 * GQuark String : "GFAL2::CORE"
 */
GQuark gfal2_get_core_quark();


/**
 * GQuark for the gfal2 config namespace
 * GQuark are used by the GError gfal2 error system in order to determine the scope of one error
 * GQuark String : "GFAL2::CONFIG"
 */
GQuark gfal2_get_config_quark();

/**
 * GQuark for the gfal2 plugin namespace
 * GQuark String : "GFAL2::PLUGINS"
 * Any plugin specific GQuark follows this pattern GFAL2::PLUGINS::NAME
 * Example srm plugin : GFAL2::PLUGINS::SRM
 */
GQuark gfal2_get_plugins_quark();


#ifdef __cplusplus
}
#endif

#endif /* GFAL_GLOBAL_H_ */
