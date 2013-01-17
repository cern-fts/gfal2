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

#include <unistd.h>

#include <file/gfal_file_api.h>

#include <common/gfal_constants.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_filedescriptor.h>
#include <common/gfal_common_dir_handle.h>



//
// @author : Devresse Adrien
//
// cancel logic of gfal 2
//


int gfal2_cancel(gfal2_context_t context){
    const int n_cancel = g_atomic_int_get(&(context->running_ops));
    context->cancel = TRUE;
    while( (g_atomic_int_get(&(context->running_ops))) > 0){
        usleep(50);
    }
    context->cancel = FALSE;
    return n_cancel;
}

gboolean gfal2_is_canceled(gfal2_context_t context){
    return context->cancel;
}
