#pragma once
#ifndef _GFAL_COMMON_INTERNAL_H_
#define _GFAL_COMMON_INTERNAL_H_
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

gfal_handle gfal_initG(GError** err);
void gfal_handle_freeG(gfal_handle handle);



// conversion funcs
char** gfal_GList_to_tab(GList* surls);
int* gfal_GList_to_tab_int(GList* int_list);


// convenience internal funcs
void gfal_new_request_state(gfal_handle handle);
int resolve_dlsym_listG(void* handle, void*** flist, const char** sym_list, int num, GError** err);
gboolean is_valid_access_result(int status);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _GFAL_COMMON_INTERNAL_H_
