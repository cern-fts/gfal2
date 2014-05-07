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
#include <glibmm.h>

#include <common/gfal_common_internal.h>
#include <common/gfal_common_plugin.h>
#include <exceptions/gerror_to_cpp.h>
#include <transfer/gfal_transfer_internal.h>
#include <transfer/gfal_transfer_types_internal.h>


static Glib::Quark scope_copy("FileCopy::start_copy");

using namespace Gfal;

static const plugin_filecopy_call find_copy_plugin(gfal2_context_t context,
        const std::string & src, const std::string & dst, void** plugin_data)
{
    GError * tmp_err = NULL;
    plugin_pointer_handle start_list, p_list;
    plugin_filecopy_call resu = NULL;
    start_list = p_list = gfal_plugins_list_handler(context, &tmp_err);
    gerror_to_cpp(&tmp_err);

    while (p_list->dlhandle != NULL) {
        plugin_url_check2_call check_call =
                p_list->plugin_api->check_plugin_url_transfer;
        if (check_call != NULL) {
            gboolean compatible;
            if ((compatible = check_call(p_list->plugin_data, context,
                    src.c_str(), dst.c_str(), GFAL_FILE_COPY)) == TRUE) {
                *plugin_data = p_list->plugin_data;
                resu = p_list->plugin_api->copy_file;
                break;
            }

        }
        p_list++;
    }
    g_free(start_list);
    return resu;
}


static void perform_copy(gfal2_context_t context, gfalt_params_t params,
        const std::string & src, const std::string & dst)
{
    gfal_log(GFAL_VERBOSE_TRACE, " -> Gfal::Transfer::FileCopy ");
    GError * tmp_err=NULL;
    int res = -1;
    void * plugin_data=NULL;
    plugin_filecopy_call p_copy = find_copy_plugin(context, src, dst, &plugin_data);
    if(p_copy == NULL){
        if(gfalt_get_local_transfer_perm(params, NULL)) {
            perform_local_copy(context, params, src, dst);
            res = 0;
        }else{
            throw Gfal::CoreException(scope_copy, "no plugin is able to support a transfer from %s to %s", EPROTONOSUPPORT);
        }
    }else{
        res = p_copy(plugin_data, context, params, src.c_str(), dst.c_str(), &tmp_err);
    }
    //p->unlock();
    if(res <0 )
        throw Gfal::CoreException(scope_copy, std::string(tmp_err->message), tmp_err->code);
    gfal_log(GFAL_VERBOSE_TRACE, " <- Gfal::Transfer::FileCopy ");
}


extern "C" {

gfalt_transfer_status_t gfalt_transfer_status_create(const gfalt_hook_transfer_plugin_t * hook){
    gfalt_transfer_status_t state = g_new0(struct _gfalt_transfer_status,1);
    state->hook = hook;
    return state;
}


void gfalt_transfer_status_delete(gfalt_transfer_status_t state){
    if(state)
        g_free(state);
}


	
int gfalt_copy_file(gfal2_context_t handle, gfalt_params_t params, 
			const char* src, const char* dst,  GError** err){
	
	g_return_val_err_if_fail( handle && src && dst, -1, err, "invalid source or/and destination values");
	GError * tmp_err=NULL;
	int ret = -1;
	gfalt_params_t p = NULL;

    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
	
	CPP_GERROR_TRY
		if(params == NULL){
			p = gfalt_params_handle_new(NULL);
            perform_copy(handle, p, src, dst);
		}
		else{
            perform_copy(handle, params, src, dst);
		}
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfalt_params_handle_delete(p, NULL);

    GFAL2_END_SCOPE_CANCEL(handle);

	G_RETURN_ERR(ret, tmp_err, err);
}


}

