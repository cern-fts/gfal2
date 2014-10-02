/*
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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
 * @file gfal_global.h
 * @brief gfal2 context management functions
 * @author Devresse Adrien
 **/

#pragma once
#ifndef GFAL_GLOBAL_HPP
#define GFAL_GLOBAL_HPP

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif

#include <glib.h>
#include <common/gfal_prototypes.h>
#include <common/gfal_deprecated.h>

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

/**
 * a gfal context is a separated instance of the gfal library
 * Each context owns his parameters, file descriptors
 * Context allows to have separated instance of GFAL with differents parameters
 *  providing an advanced interface to  GFAL
 */
typedef struct gfal_handle_* gfal2_context_t;
// backward compatibility
typedef GFAL2_DEPRECATED(gfal2_context_t) gfal2_context_t gfal_context_t;

///
///   @brief create a gfal2 context
///
///
///   Each context contain its own set of parameters and configurations ( \ref config_group )
///
///   A context can be used in multiple threads at the same time ( Thread-safe ).
///
///   @param err : GError error report system
///   @return a context if success, NULL if error
///
gfal2_context_t gfal2_context_new(GError ** err);

///
///  free a gfal2 context
///  It is safe to delete a NULL context
///  Thread-safe
///
void gfal2_context_free(gfal2_context_t context);


///   @deprecated
///    same than gfal2_context_new but with old compatibility
///
GFAL2_DEPRECATED(gfal2_context_new) gfal2_context_t gfal_context_new(GError ** err);

///  @deprecated
///
GFAL2_DEPRECATED(gfal2_context_free) void gfal_context_free(gfal2_context_t context);


#define GFAL2_QUARK_CORE "GFAL2::CORE"
#define GFAL2_QUARK_CONFIG "GFAL2::CONFIG"
#define GFAL2_QUARK_PLUGINS "GFAL2::PLUGINS"

///
///  GQuark for the gfal2 core namespace
///  GQuark are used by the GError gfal2 error system in order to determine the scope of one error
///  GQuark String : "GFAL2::CORE"
GQuark gfal2_get_core_quark();


///
///  GQuark for the gfal2 config namespace
///  GQuark are used by the GError gfal2 error system in order to determine the scope of one error
///  GQuark String : "GFAL2::CONFIG"
GQuark gfal2_get_config_quark();

///
/// GQuark for the gfal2 plugin namespace
/// GQuark String : "GFAL2::PLUGINS"
/// Any Plugin specific GQuark follows this pattern GFAL2::PLUGINS::NAME
/// example srm plugin : GFAL2::PLUGINS::SRM
GQuark gfal2_get_plugins_quark();


#ifdef __cplusplus
}
#endif  // __cplusplus


#endif /* GFAL_GLOBAL_HPP */
