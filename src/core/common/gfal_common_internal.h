#pragma once
#ifndef _GFAL_COMMON_INTERNAL_H_
#define _GFAL_COMMON_INTERNAL_H_
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

/*
 * gfal_common_internal.h
 * internal declaration for gfal 2.0
 * 			do not use this in external programs
 * author Devresse Adrien
 * */

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define GFAL2_BEGIN_SCOPE_CANCEL(context, ret_err_value, err) \
    do{                                                       \
    if(gfal2_start_scope_cancel(context, err) < 0){           \
        return ret_err_value;                                 \
    }                                                         \
    }while(0)

#define GFAL2_END_SCOPE_CANCEL(context) \
    gfal2_end_scope_cancel(context)

/// init function
gfal2_context_t gfal_initG(GError** err);

/// free gfal handle
void gfal_handle_freeG(gfal2_context_t handle);

GQuark gfal_cancel_quark();


int gfal2_start_scope_cancel(gfal2_context_t context, GError** err);

int gfal2_end_scope_cancel(gfal2_context_t context);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _GFAL_COMMON_INTERNAL_H_
