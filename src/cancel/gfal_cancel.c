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

#include <cancel/gfal_cancel.h>

#include <common/gfal_constants.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_errverbose.h>



//
// @author : Devresse Adrien
//
// cancel logic of gfal 2
//


int gfal2_cancel(gfal2_context_t context){
    g_assert(context);
    if(context->cancel == TRUE) // avoid recursive calls
        return 0;
    g_mutex_lock(context->mux_cancel);
    const int n_cancel = g_atomic_int_get(&(context->running_ops));
    context->cancel = TRUE;
    g_hook_list_invoke(&context->cancel_hooks, TRUE);
    g_mutex_unlock(context->mux_cancel);
    while( (g_atomic_int_get(&(context->running_ops))) > 0){
        usleep(50);
    }
    context->cancel = FALSE;
    return n_cancel;
}

gboolean gfal2_is_canceled(gfal2_context_t context){
    return context->cancel;
}


//  increase number of the running task for the cancel logic
// return negative value if task is canceled
int gfal2_start_scope_cancel(gfal2_context_t context, GError** err){
    if(context->cancel){
        g_set_error(err, gfal_cancel_quark(), ECANCELED, "[gfal2_cancel] operation canceled by user");
        return -1;
    }
    g_atomic_int_inc(&(context->running_ops));
    return 0;
}

int gfal2_end_scope_cancel(gfal2_context_t context){
    g_atomic_int_dec_and_test(&(context->running_ops));
    return 0;
}


struct gfal_hook_data_s{
    void* userdata;
    gfal2_context_t context;
    gfal_cancel_hook_cb cb;
};


static void gfal_ghook_cancel_wrapper(gpointer data){
  struct gfal_hook_data_s* d  = data;
  d->cb(d->context, d->userdata);
}

gfal_cancel_token_t gfal2_register_cancel_callback(gfal2_context_t context, gfal_cancel_hook_cb cb, void* userdata){
    g_assert(context && cb);
    g_mutex_lock(context->mux_cancel);
    GHook* h = g_hook_alloc(&context->cancel_hooks);
    struct gfal_hook_data_s* d = g_new(struct gfal_hook_data_s,1);
    d->context = context;
    d->userdata = userdata;
    d->cb = cb;
    h->data = d;
    h->destroy = &g_free;
    h->func = &gfal_ghook_cancel_wrapper;
    g_hook_append(&context->cancel_hooks, h);
    g_mutex_unlock(context->mux_cancel);
    return (gfal_cancel_token_t) h;
}


void gfal2_remove_cancel_callback(gfal2_context_t context, gfal_cancel_token_t token){
    g_assert(context && token);
    g_mutex_lock(context->mux_cancel);
    GHook* cb = (GHook*) token;
    g_hook_destroy_link(&context->cancel_hooks, cb);
    g_mutex_unlock(context->mux_cancel);
}

///
///
GQuark gfal_cancel_quark(){
    return g_quark_from_string("[gfal2_cancel]");
}
