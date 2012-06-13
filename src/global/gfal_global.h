#pragma once
#ifndef GFAL_GLOBAL_HPP
#define GFAL_GLOBAL_HPP
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
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

#include <glib.h>
#include <common/gfal_prototypes.h>

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
typedef gfal_handle gfal2_context_t;
// backward compatibility
typedef gfal2_context_t gfal_context_t;

/**
  @brief create a gfal 2 context
  contexts are used for all gfal 2 operations
  @param err : GError error report system
  @return gfal2 context
*/
gfal2_context_t gfal2_context_new(GError ** err);

/**
  free a gfal 2 context
*/
void gfal2_context_free(gfal2_context_t context);

//  same than gfal2_context_new but with old compatibility
gfal_context_t gfal_context_new(GError ** err);
// same than gfal2_context_free for compatibility reasons
void gfal_context_free(gfal_context_t context);

#ifdef __cplusplus
}
#endif  // __cplusplus


#endif /* GFAL_GLOBAL_HPP */ 

