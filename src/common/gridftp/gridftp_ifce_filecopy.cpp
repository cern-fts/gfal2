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

#include <string>
#include <sstream>

#include "gridftp_exist.h"
#include "gridftp_ifce_filecopy.h"

#include <externals/utils/uri_util.h>
#include <transfer/gfal_transfer_types_internal.h>
#include <file/gfal_file_api.h>

const Glib::Quark scope_filecopy("GridFTP::Filecopy");
const Glib::Quark gsiftp_domain("GSIFTP");
const char * gridftp_checksum_transfer_config= "COPY_CHECKSUM_TYPE";

void gridftp_filecopy_delete_existing(gfal2_context_t context, GridFTP_session * sess, gfalt_params_t params, const char * url){
	const bool replace = gfalt_get_replace_existing_file(params,NULL);
    bool exist = gridftp_module_file_exist(context, sess, url);
    if(exist){

        if(replace){
            gfal_log(GFAL_VERBOSE_TRACE, " File %s already exist, delete it for override ....",url);
            gridftp_unlink_internal(context, sess, url, false);
            gfal_log(GFAL_VERBOSE_TRACE, " File %s deleted with success, proceed to copy ....",url);
        }else{
            char err_buff[GFAL_ERRMSG_LEN];
            snprintf(err_buff, GFAL_ERRMSG_LEN, " Destination already exist %s, Cancel", url);
            throw Gfal::CoreException(scope_filecopy, err_buff, EEXIST);
        }
    }
	
}

// create the parent directory
void gridftp_create_parent_copy(gfal2_context_t handle, gfalt_params_t params,
                                    const char * gridftp_url){
    const gboolean create_parent = gfalt_get_create_parent_dir(params, NULL);
    if(create_parent){
        gfal_log(GFAL_VERBOSE_TRACE, " -> [gridftp_create_parent_copy]");
        GError * tmp_err=NULL;
        char current_uri[GFAL_URL_MAX_LEN];
        g_strlcpy(current_uri, gridftp_url, GFAL_URL_MAX_LEN);
        const size_t s_uri = strlen(current_uri);
        char* p_uri = current_uri + s_uri -1;
        while( p_uri > current_uri && *p_uri == '/' ){ // remove trailing '/'
            *p_uri = '\0';
             p_uri--;
        }
        while( p_uri > current_uri && *p_uri != '/'){ // find the parent directory
            p_uri--;
        }
        if(p_uri > current_uri){
            struct stat st;
            *p_uri = '\0';

            gfal2_stat(handle, current_uri, &st, &tmp_err);

            if (tmp_err && tmp_err->code != ENOENT)
                Gfal::gerror_to_cpp(&tmp_err);
            else if (tmp_err)
                g_error_free(tmp_err);
            else if (!S_ISDIR(st.st_mode))
                throw Gfal::CoreException(scope_filecopy,
                                          "The parent of the destination file exists, but it is not a directory",
                                          ENOTDIR);
            else
                return;

            tmp_err = NULL;
            gfal_log(GFAL_VERBOSE_TRACE, "try to create directory %s", current_uri);
            (void) gfal2_mkdir_rec(handle, current_uri, 0755, &tmp_err);
            Gfal::gerror_to_cpp(&tmp_err);

        }else{
            throw Gfal::CoreException(scope_filecopy, "impossible to create directory " + std::string(current_uri) + " : invalid path", EINVAL);
        }
        gfal_log(GFAL_VERBOSE_TRACE, " [gridftp_create_parent_copy] <-");
    }
}

void gsiftp_rd3p_callback(void* user_args, globus_gass_copy_handle_t* handle, globus_off_t total_bytes, float throughput, float avg_throughput);

struct Callback_handler{

    Callback_handler(gfalt_params_t params, GridFTP_Request_state* req, const char* src, const char* dst){
        GError * tmp_err=NULL;
        args.callback = gfalt_get_monitor_callback(params, &tmp_err);
        Gfal::gerror_to_cpp(&tmp_err);

        args.req = req;
        args.user_args = gfalt_get_user_data(params, &tmp_err);
        args.src = src;
        args.dst = dst;
        args.start_time = time(NULL);
        Gfal::gerror_to_cpp(&tmp_err);
        if(args.callback){
            Glib::RWLock::ReaderLock l (req->mux_req_state);
            Glib::Mutex::Lock l_call (req->mux_callback_lock);
            globus_gass_copy_register_performance_cb(args.req->sess->get_gass_handle(), gsiftp_rd3p_callback, (gpointer) &args);
        }
    }

    virtual ~Callback_handler(){
        Glib::RWLock::ReaderLock l (args.req->mux_req_state);
        Glib::Mutex::Lock l_call (args.req->mux_callback_lock);
        globus_gass_copy_register_performance_cb(args.req->sess->get_gass_handle(), NULL, NULL);
    }
    struct callback_args{
        gfalt_monitor_func callback;
        gpointer user_args;
        GridFTP_Request_state* req;
        const char* src;
        const char* dst;
        time_t start_time;
    } args;
};

void gsiftp_rd3p_callback(void* user_args, globus_gass_copy_handle_t* handle, globus_off_t total_bytes, float throughput, float avg_throughput){
    Callback_handler::callback_args * args = (Callback_handler::callback_args *) user_args;

    GridFTP_Request_state* req = args->req;
    Glib::RWLock::ReaderLock l (req->mux_req_state);
//    Glib::Mutex::Lock l_call (req->mux_callback_lock); --> disable the security lock, globus seems to stuck internally with the pthread threading model

    gfalt_hook_transfer_plugin_t hook;
    hook.bytes_transfered = total_bytes;
    hook.average_baudrate= (size_t) avg_throughput;
    hook.instant_baudrate = (size_t) throughput;
    hook.transfer_time = (time(NULL) - args->start_time);

    gfalt_transfer_status_t state = gfalt_transfer_status_create(&hook);
    args->callback(state, args->src, args->dst, args->user_args);
    gfalt_transfer_status_delete(state);
}


int gridftp_filecopy_copy_file_internal(GridFTPFactoryInterface * factory, gfalt_params_t params,
                                        const char* src, const char* dst){
    using namespace Gfal::Transfer;
    GError * tmp_err=NULL;
    struct timespec ops_timeout;

    ops_timeout.tv_nsec =0;
    ops_timeout.tv_sec = gfalt_get_timeout(params, &tmp_err); Gfal::gerror_to_cpp(&tmp_err);
    const unsigned int nbstream = gfalt_get_nbstreams(params, &tmp_err); Gfal::gerror_to_cpp(&tmp_err);
    const guint64 tcp_buffer_size = gfalt_get_tcp_buffer_size(params, &tmp_err); Gfal::gerror_to_cpp(&tmp_err);

    std::auto_ptr<GridFTP_Request_state> req(  new GridFTP_Request_state(factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(src)),
                                                                         true,
                                                                         GRIDFTP_REQUEST_GASS)
                                            );
    GridFTP_session* sess = req->sess.get();

    sess->set_nb_stream( nbstream);
    gfal_log(GFAL_VERBOSE_TRACE, "   [GridFTPFileCopyModule::filecopy] setup gsiftp number of streams to %d", nbstream);
    req->init_timeout(&ops_timeout);
    gfal_log(GFAL_VERBOSE_TRACE, "   [GridFTPFileCopyModule::filecopy] setup gsiftp timeout to %ld s and %ld ns", ops_timeout.tv_sec, ops_timeout.tv_nsec);
    sess->set_tcp_buffer_size(tcp_buffer_size);
    gfal_log(GFAL_VERBOSE_TRACE, "   [GridFTPFileCopyModule::filecopy] setup gsiftp buffer size to %d", tcp_buffer_size);

    Callback_handler callback_handler(params, req.get(), src, dst);

    if( gfalt_get_strict_copy_mode(params, NULL) == false){
        gridftp_filecopy_delete_existing(factory->get_handle(), sess, params, dst);
        gridftp_create_parent_copy(factory->get_handle(), params, dst);
    }

    std::auto_ptr<Gass_attr_handler>  gass_attr_src(sess->generate_gass_copy_attr());
    std::auto_ptr<Gass_attr_handler>  gass_attr_dst(sess->generate_gass_copy_attr());

    req->start();
    gfal_log(GFAL_VERBOSE_TRACE, "   [GridFTPFileCopyModule::filecopy] start gridftp transfer %s -> %s", src, dst);
    GridFTPOperationCanceler canceler(factory->get_handle(), req.get());
    gfal_globus_result_t res = globus_gass_copy_register_url_to_url(
        sess->get_gass_handle(),
        (char*)src,
        &(gass_attr_src->attr_gass),
        (char*)dst,
        &(gass_attr_dst->attr_gass),
        globus_gass_basic_client_callback,
        req.get()
    );

    gfal_globus_check_result("GridFTPFileCopyModule::filecopy", res);
    req->wait_callback(scope_filecopy);
    return 0;

}

void gridftp_checksum_transfer_verify(const char * src_chk, const char* dst_chk, const char* user_defined_chk){
    if(*user_defined_chk == '\0'){
        if(strncasecmp(src_chk, dst_chk,GFAL_URL_MAX_LEN) != 0){
            std::ostringstream ss;
            ss << "SRC and DST checksum are different: " << src_chk << " " << dst_chk ;
            throw Gfal::CoreException(scope_filecopy, ss.str(), EIO);
        }
    }else{
        if(strncasecmp(src_chk, user_defined_chk, GFAL_URL_MAX_LEN) != 0
                || strncasecmp(dst_chk, user_defined_chk, GFAL_URL_MAX_LEN) != 0){
            std::ostringstream ss;
            ss << "USER_DEFINE, SRC and DST checksum are different " << user_defined_chk << " " << src_chk << " " << dst_chk;
            throw Gfal::CoreException(scope_filecopy, ss.str(),EIO);
        }

    }
}

// clear dest if error occures in transfer, does not clean if dest file if set as already exist before any transfer
void GridftpModule::autoCleanFileCopy(gfalt_params_t params, GError* checked_error, const char* dst){
    if(checked_error && checked_error->code != EEXIST){
        gfal_log(GFAL_VERBOSE_TRACE, "\t\tError in transfer, clean destination file %s ", dst);
        try{
            this->unlink(dst);
        }catch(...){
           gfal_log(GFAL_VERBOSE_TRACE, "\t\tFailure in cleaning ...");
        }
    }
}


int GridftpModule::filecopy(gfalt_params_t params, const char* src, const char* dst){
    GError * tmp_err, *tmp_err_chk_src, *tmp_err_chk_copy, *tmp_err_chk_dst;
    tmp_err=tmp_err_chk_src=tmp_err_chk_dst=tmp_err_chk_copy=NULL;


    char checksum_src[GFAL_URL_MAX_LEN]={0};
    char checksum_dst[GFAL_URL_MAX_LEN]={0};
    char checksum_user_defined[GFAL_URL_MAX_LEN];
    char checksum_type_user_define[GFAL_URL_MAX_LEN];

    gboolean checksum_check = gfalt_get_checksum_check(params, &tmp_err);
    Gfal::gerror_to_cpp(&tmp_err);
    struct scoped_free{
        scoped_free() {checksum_algo=NULL;}
        ~scoped_free(){
            g_free(checksum_algo);
        }
        char *checksum_algo;
    }chk_algo; // automated destruction

    if(checksum_check){
        gfalt_get_user_defined_checksum(params, checksum_type_user_define, GFAL_URL_MAX_LEN,
                                        checksum_user_defined, GFAL_URL_MAX_LEN, &tmp_err); // fetch the user defined chk
        Gfal::gerror_to_cpp(&tmp_err);
        if(*checksum_user_defined == '\0' || *checksum_type_user_define == '\0'){ // check if user defined checksum exist
             chk_algo.checksum_algo = gfal2_get_opt_string(_handle_factory->get_handle(), GRIDFTP_CONFIG_GROUP, gridftp_checksum_transfer_config, &tmp_err);
            Gfal::gerror_to_cpp(&tmp_err);
            gfal_log(GFAL_VERBOSE_TRACE, "\t\tNo user defined checksum, fetch the default one from configuration ");
        }else{
            chk_algo.checksum_algo = g_strdup(checksum_type_user_define);
        }
        gfal_log(GFAL_VERBOSE_DEBUG, "\t\tChecksum Algorithm for transfer verification %s", chk_algo.checksum_algo);
    }

    const int n_sess = 1;
    #pragma omp parallel num_threads(n_sess)
    {
        #pragma omp sections
        {
            #pragma omp section  // calc src checksum
            {
               CPP_GERROR_TRY
               if(checksum_check) {
                    plugin_trigger_event(params, gsiftp_domain,
                                         GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_ENTER,
                                         "%s", chk_algo.checksum_algo);

                    checksum(src, chk_algo.checksum_algo, checksum_src, GFAL_URL_MAX_LEN, 0,0);

                    plugin_trigger_event(params, gsiftp_domain,
                                                        GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_EXIT,
                                                        "%s", chk_algo.checksum_algo);
               }
               CPP_GERROR_CATCH(&tmp_err_chk_src);
            }
            #pragma omp section // start transfert and replace logic
            {
                  plugin_trigger_event(params, gsiftp_domain,
                                       GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_ENTER,
                                       "");
                  CPP_GERROR_TRY
                  gridftp_filecopy_copy_file_internal(_handle_factory, params, src, dst);
                  CPP_GERROR_CATCH(&tmp_err_chk_copy);
                  plugin_trigger_event(params, gsiftp_domain,
                                       GFAL_EVENT_NONE, GFAL_EVENT_TRANSFER_EXIT,
                                       "");
            }
        }
    }


    if(gfal_error_keep_first_err(&tmp_err,&tmp_err_chk_copy , &tmp_err_chk_src, &tmp_err_chk_dst, NULL)){
        autoCleanFileCopy( params, tmp_err, dst);
        Gfal::gerror_to_cpp(&tmp_err);
    }

    // calc checksum for dst
    if(checksum_check){
        plugin_trigger_event(params, gsiftp_domain,
                             GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_ENTER,
                             "%s", chk_algo.checksum_algo);

        checksum(dst, chk_algo.checksum_algo, checksum_dst, GFAL_URL_MAX_LEN, 0,0);
        gridftp_checksum_transfer_verify(checksum_src, checksum_dst, checksum_user_defined);

        plugin_trigger_event(params, gsiftp_domain,
                             GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_EXIT,
                             "%s", chk_algo.checksum_algo);
    }

    return 0;
}

extern "C"{

/**
 * initiaize a file copy from the given source to the given dest with the parameters params
 */
int plugin_filecopy(plugin_handle handle, gfal2_context_t context, gfalt_params_t params, const char* src, const char* dst, GError ** err){
	g_return_val_err_if_fail( handle != NULL && src != NULL
			&& dst != NULL , -1, err, "[plugin_filecopy][gridftp] einval params");

	GError * tmp_err=NULL;
	int ret = -1;
	gfal_log(GFAL_VERBOSE_TRACE, "  -> [gridftp_plugin_filecopy]");
	CPP_GERROR_TRY
		( static_cast<GridftpModule*>(handle))->filecopy(params, src, dst);
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfal_log(GFAL_VERBOSE_TRACE, "  [gridftp_plugin_filecopy]<-");
	G_RETURN_ERR(ret, tmp_err, err);
}

int gridftp_plugin_filecopy(plugin_handle handle, gfal2_context_t context, gfalt_params_t params, const char* src, const char* dst, GError ** err){
    return plugin_filecopy(handle, context, params, src, dst, err);
}

}
