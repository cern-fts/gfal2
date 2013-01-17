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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <common/gfal_common_plugin_interface.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_common_internal.h>
#include <transfer/gfal_transfer_types_internal.h>
#include <fdesc/gfal_file_handle.h>
#include <exceptions/gerror_to_cpp.h>


const Glib::Quark scope_copy("FileCopy::start_copy");
const Glib::Quark scope_local_copy("FileCopy::local_copy");

const long default_buffer_size= 200000;


namespace Gfal{

namespace Transfer{


FileCopy::FileCopy(gfal2_context_t context){
    this->context = context;
}

FileCopy::~FileCopy() {

}

void FileCopy::start_copy(gfalt_params_t p, const std::string & src, const std::string & dst){
    gfal_log(GFAL_VERBOSE_TRACE, " -> Gfal::Transfer::FileCopy ");
    GError * tmp_err=NULL;
    int res = -1;
    void * plugin_data=NULL;
    plugin_filecopy_call p_copy = find_copy_plugin(src, dst, &plugin_data); // get the filecopy call of the plugin
    if(p_copy == NULL){
        if(gfalt_get_local_transfer_perm(p,NULL) ){
            start_local_copy(p, src, dst);
            res = 0;
        }else{
            throw Gfal::CoreException(scope_copy, "no plugin is able to support a transfer from %s to %s", EPROTONOSUPPORT);
        }
    }else{
        res = p_copy(plugin_data, context, p, src.c_str(), dst.c_str(), &tmp_err);
    }
    //p->unlock();
    if(res <0 )
        throw Gfal::CoreException(scope_copy, std::string(tmp_err->message), tmp_err->code);
    gfal_log(GFAL_VERBOSE_TRACE, " <- Gfal::Transfer::FileCopy ");
}


void FileCopy::start_local_copy(gfalt_params_t p, const std::string & src, const std::string & dst){
    gfal_log(GFAL_VERBOSE_TRACE, " -> Gfal::Transfer::start_local_copy ");
    GError * tmp_err_src=NULL;
    GError * tmp_err_dst=NULL;
    GError * tmp_err_out=NULL;

    gfal_file_handle f_src =NULL;
    gfal_file_handle f_dst = NULL;

    gfal_log(GFAL_VERBOSE_TRACE, " open src file : %s ",src.c_str());
    f_src = gfal_plugin_openG(this->context,src.c_str(), O_RDONLY,0, &tmp_err_src);
    if(!tmp_err_src){
        gfal_log(GFAL_VERBOSE_TRACE, "  open dst file : %s ",dst.c_str());
        f_dst = gfal_plugin_openG(this->context,dst.c_str(), O_WRONLY | O_CREAT,0755, &tmp_err_dst);
        ssize_t s_file=1;
        if(!tmp_err_dst){
            const unsigned long mbuffer = default_buffer_size;
            char* buff = (char*) g_malloc(mbuffer);

            gfal_log(GFAL_VERBOSE_TRACE, "  begin local transfer %s ->  %s with buffer size %ld",src.c_str(), dst.c_str(), mbuffer);
            while(s_file > 0 && !tmp_err_src && !tmp_err_dst){
                    s_file = gfal_plugin_readG(this->context,f_src, buff, mbuffer, &tmp_err_src);
                    if(s_file > 0 && !tmp_err_src)
                        gfal_plugin_writeG(this->context,f_dst, buff, s_file, &tmp_err_dst);
            }
            g_free(buff);
            gfal_plugin_closeG(this->context, f_src, (tmp_err_src)?NULL:(&tmp_err_src));
            gfal_plugin_closeG(this->context, f_dst, (tmp_err_dst)?NULL:(&tmp_err_dst));
        }
    }


    if(tmp_err_src){
        g_set_error(&tmp_err_out,scope_local_copy.id(),tmp_err_src->code, "Local transfer error on SRC %s : %s",src.c_str(),tmp_err_src->message);
        g_clear_error(&tmp_err_src);
    }else if (tmp_err_dst){
        g_set_error(&tmp_err_out,scope_local_copy.id(),tmp_err_dst->code, "Local transfer error on DST %s : %s",dst.c_str(),tmp_err_dst->message);
        g_clear_error(&tmp_err_dst);
    }
    if(tmp_err_out)
        throw Glib::Error(tmp_err_out);

    gfal_log(GFAL_VERBOSE_TRACE, " <- Gfal::Transfer::start_local_copy ");
}

const plugin_filecopy_call FileCopy::find_copy_plugin(const std::string & src, const std::string & dst, void** plugin_data){


    GError * tmp_err=NULL;
    plugin_pointer_handle start_list, p_list;
    plugin_filecopy_call resu = NULL;
    start_list = p_list = gfal_plugins_list_handler(context, &tmp_err);
    gerror_to_cpp(&tmp_err);

    while(p_list->dlhandle != NULL){
            plugin_url_check2_call check_call = p_list->plugin_api->check_plugin_url_transfer;
            if(check_call != NULL){
                gboolean compatible;
                if( (compatible = check_call(p_list->plugin_data, src.c_str(), dst.c_str(), GFAL_FILE_COPY) ) == TRUE){
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




} //end Transfer


} // end Gfal






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
        Gfal::Transfer::FileCopy f(handle);
		if(params == NULL){
			p = gfalt_params_handle_new(NULL);
            f.start_copy(p, src, dst);
		}
		else{
            f.start_copy(params, src, dst);
		}
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfalt_params_handle_delete(p, NULL);

    GFAL2_END_SCOPE_CANCEL(handle);

	G_RETURN_ERR(ret, tmp_err, err);
}


}

