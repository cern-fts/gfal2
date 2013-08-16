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
#include <ctime>
#include <csignal>

#include "gridftp_namespace.h"
#include "gridftp_filecopy.h"

#include <checksums/checksums.h>
#include <uri/uri_util.h>
#include <transfer/gfal_transfer_types_internal.h>
#include <file/gfal_file_api.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

static Glib::Quark gfal_gridftp_scope_filecopy(){
    return Glib::Quark("GridFTP::Filecopy");
}

static Glib::Quark gfal_gsiftp_domain(){
    return Glib::Quark("GSIFTP");
}

/*IPv6 compatible lookup*/
std::string lookup_host (const char *host)
{
  struct addrinfo hints, *res=NULL;
  int errcode;
  char addrstr[100]={0};
  void *ptr = NULL;
  
  if(!host){
  	return std::string("cant.be.resolved");  
  }

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags |= AI_CANONNAME;

  errcode = getaddrinfo (host, NULL, &hints, &res);
  if (errcode != 0){
  	return std::string("cant.be.resolved");
  }

  while (res)
    {
      inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);

      switch (res->ai_family)
        {
        case AF_INET:
          ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
          break;
        case AF_INET6:
          ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
          break;
        }
      if(ptr){	
      	inet_ntop (res->ai_family, ptr, addrstr, 100);      
	}

      res = res->ai_next;
    }
    
  if(res)
  	freeaddrinfo(res);
  
  if(strlen(addrstr) < 7)
  	return std::string("cant.be.resolved");
  else
  	return std::string(addrstr);
}



static std::string returnHostname(const std::string &uri){
	Uri u0 = Uri::Parse(uri);
	return  lookup_host(u0.Host.c_str()) + ":" + u0.Port;	
}

const char * gridftp_checksum_transfer_config= "COPY_CHECKSUM_TYPE";
const char * gridftp_perf_marker_timeout_config= "PERF_MARKER_TIMEOUT";

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
            throw Gfal::CoreException(gfal_gridftp_scope_filecopy(), err_buff, EEXIST);
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
                throw Gfal::CoreException(gfal_gridftp_scope_filecopy(),
                                          "The parent of the destination file exists, but it is not a directory",
                                          ENOTDIR);
            else
                return;

            tmp_err = NULL;
            gfal_log(GFAL_VERBOSE_TRACE, "try to create directory %s", current_uri);
            (void) gfal2_mkdir_rec(handle, current_uri, 0755, &tmp_err);
            Gfal::gerror_to_cpp(&tmp_err);

        }else{
            throw Gfal::CoreException(gfal_gridftp_scope_filecopy(), "impossible to create directory " + std::string(current_uri) + " : invalid path", EINVAL);
        }
        gfal_log(GFAL_VERBOSE_TRACE, " [gridftp_create_parent_copy] <-");
    }
}

void gsiftp_rd3p_callback(void* user_args, globus_gass_copy_handle_t* handle, globus_off_t total_bytes, float throughput, float avg_throughput);

//
// Performance callback object
// contain the performance callback parameter
// an the auto cancel logic on performance callback inaticity
struct Callback_handler{

    Callback_handler(gfal2_context_t context,
                     gfalt_params_t params, GridFTP_Request_state* req,
                     const char* src, const char* dst) :
        args(NULL){
        GError * tmp_err=NULL;
        gfalt_monitor_func callback = gfalt_get_monitor_callback(params, &tmp_err);
        Gfal::gerror_to_cpp(&tmp_err);
        gpointer user_args = gfalt_get_user_data(params, &tmp_err);
        Gfal::gerror_to_cpp(&tmp_err);

        if(callback){
            args = new callback_args(context, callback, user_args, src, dst, req, &tmp_err);
        }


    }

    static void* func_timer(void* v){
        callback_args* args = (callback_args*) v;
        while( time(NULL) < args->timeout_time){
            if( pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0){
                gfal_log(GFAL_VERBOSE_TRACE, " thread setcancelstate error, interrupt perf marker timer");
                return NULL;
            }
            usleep(500000);
            if( pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) != 0){
                gfal_log(GFAL_VERBOSE_TRACE, " thread setcancelstate error, interrupt perf marker timer");
                return NULL;
            }
        }

        std::stringstream msg;
        msg << "Transfer canceled because the gsiftp performance marker timeout of "
            << args->timeout_value
            << "seconds has been exceeded.";
        args->req->cancel_operation_async(gfal_gridftp_scope_filecopy(),
                                          msg.str());

        return NULL;
    }

    virtual ~Callback_handler(){
        if(args){
            delete args;
        }
    }
    struct callback_args{
        callback_args(gfal2_context_t context, const gfalt_monitor_func mcallback, gpointer muser_args,
                      const char* msrc, const char* mdst, GridFTP_Request_state* mreq,
                      GError** err) :
            callback(mcallback),
            user_args(muser_args),
            req(mreq),
            src(msrc),
            dst(mdst),
            start_time(time(NULL)),
            timeout_value(gfal2_get_opt_integer_with_default(context, GRIDFTP_CONFIG_GROUP,
                                                             gridftp_perf_marker_timeout_config, 180)),
            timeout_time(time(NULL) + timeout_value),
            timer_pthread()
        {
            Glib::RWLock::ReaderLock l (req->mux_req_state);
            globus_gass_copy_register_performance_cb(req->sess->get_gass_handle(), gsiftp_rd3p_callback, (gpointer) this);

            if(timeout_value > 0){
                pthread_create(&timer_pthread, NULL, Callback_handler::func_timer, this);
            }
        }

        virtual ~callback_args(){
            if(timeout_value > 0){
                void * res;
                pthread_cancel(timer_pthread);
                pthread_join(timer_pthread, &res);
            }
            Glib::RWLock::ReaderLock l (req->mux_req_state);
            globus_gass_copy_register_performance_cb(req->sess->get_gass_handle(), NULL, NULL);
        }

        gfalt_monitor_func callback;
        gpointer user_args;
        GridFTP_Request_state* req;
        const char* src;
        const char* dst;
        time_t start_time;
        int timeout_value;
        time_t timeout_time;
        pthread_t timer_pthread;
    } *args;
};

void gsiftp_rd3p_callback(void* user_args, globus_gass_copy_handle_t* handle, globus_off_t total_bytes, float throughput, float avg_throughput){
    Callback_handler::callback_args * args = (Callback_handler::callback_args *) user_args;

    GridFTP_Request_state* req = args->req;
    Glib::RWLock::ReaderLock l (req->mux_req_state);
    if(args->timeout_value > 0){
        gfal_log(GFAL_VERBOSE_TRACE, "Performance marker received, re-arm timer");
        args->timeout_time = time(NULL) + args->timeout_value;
    }

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

    const time_t timeout = gfalt_get_timeout(params, &tmp_err);
    Gfal::gerror_to_cpp(&tmp_err);

    const unsigned int nbstream = gfalt_get_nbstreams(params, &tmp_err); Gfal::gerror_to_cpp(&tmp_err);
    const guint64 tcp_buffer_size = gfalt_get_tcp_buffer_size(params, &tmp_err); Gfal::gerror_to_cpp(&tmp_err);

    std::auto_ptr<GridFTP_Request_state> req(  new GridFTP_Request_state(factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(src)),
                                                                         true,
                                                                         GRIDFTP_REQUEST_GASS)
                                            );
    GridFTP_session* sess = req->sess.get();

    sess->set_nb_stream( nbstream);
    gfal_log(GFAL_VERBOSE_TRACE, "   [GridFTPFileCopyModule::filecopy] setup gsiftp number of streams to %d", nbstream);
    sess->set_tcp_buffer_size(tcp_buffer_size);
    gfal_log(GFAL_VERBOSE_TRACE, "   [GridFTPFileCopyModule::filecopy] setup gsiftp buffer size to %d", tcp_buffer_size);

    if( gfalt_get_strict_copy_mode(params, NULL) == false){
        gridftp_filecopy_delete_existing(factory->get_handle(), sess, params, dst);
        gridftp_create_parent_copy(factory->get_handle(), params, dst);
    }

    std::auto_ptr<Gass_attr_handler>  gass_attr_src(sess->generate_gass_copy_attr());
    std::auto_ptr<Gass_attr_handler>  gass_attr_dst(sess->generate_gass_copy_attr());
    Callback_handler callback_handler(factory->get_handle(), params, req.get(), src, dst);

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
    req->wait_callback(gfal_gridftp_scope_filecopy(), timeout);
    return 0;

}

void gridftp_checksum_transfer_verify(const char * src_chk, const char* dst_chk, const char* user_defined_chk){
    if(*user_defined_chk == '\0'){
        if(gfal_compare_checksums(src_chk, dst_chk, GFAL_URL_MAX_LEN) != 0){
            std::ostringstream ss;
            ss << "SRC and DST checksum are different. Source: " << src_chk << " Destination: " << dst_chk ;
            throw Gfal::CoreException(gfal_gridftp_scope_filecopy(), ss.str(), EIO);
        }
    }else{
        if(gfal_compare_checksums(src_chk, user_defined_chk, GFAL_URL_MAX_LEN) != 0
                || gfal_compare_checksums(dst_chk, user_defined_chk, GFAL_URL_MAX_LEN) != 0){
            std::ostringstream ss;
            ss << "USER_DEFINE, SRC and DST checksum are different. User defined: "
               << user_defined_chk << " Source: " << src_chk
               << " Destination: " << dst_chk;
            throw Gfal::CoreException(gfal_gridftp_scope_filecopy(), ss.str(),EIO);
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


void GridftpModule::filecopy(gfalt_params_t params, const char* src, const char* dst)
{
    char checksum_type[GFAL_URL_MAX_LEN]={0};
    char checksum_user_defined[GFAL_URL_MAX_LEN];
    char checksum_src[GFAL_URL_MAX_LEN] = { 0 };
    char checksum_dst[GFAL_URL_MAX_LEN] = { 0 };

    gboolean checksum_check = gfalt_get_checksum_check(params, NULL);

    if (checksum_check) {
        gfalt_get_user_defined_checksum(params,
                                        checksum_type, sizeof(checksum_type),
                                        checksum_user_defined, sizeof(checksum_user_defined),
                                        NULL);

        if (checksum_user_defined[0] == '\0' && checksum_type[0] == '\0') {
            GError *get_default_error = NULL;
            char *default_checksum_type;

            default_checksum_type = gfal2_get_opt_string(_handle_factory->get_handle(),
                                                         GRIDFTP_CONFIG_GROUP,
                                                         gridftp_checksum_transfer_config,
                                                         &get_default_error);
            Gfal::gerror_to_cpp(&get_default_error);

            strncpy(checksum_type, default_checksum_type, sizeof(checksum_type));
	    checksum_type[GFAL_URL_MAX_LEN-1] = '\0';
            g_free(default_checksum_type);

            gfal_log(GFAL_VERBOSE_TRACE,
                     "\t\tNo user defined checksum, fetch the default one from configuration");
        }

        gfal_log(GFAL_VERBOSE_DEBUG,
                "\t\tChecksum Algorithm for transfer verification %s",
                checksum_type);
    }

    // Retrieving the source checksum and doing the transfer can be, potentially,
    // done in parallel. But not for now.
    // (That's why the brackets: they are marking potential parallelizable sections)
    // But remember to modify the catches when you make them parallel!

    // Source checksum
    {
        try {
            if (checksum_check) {
                plugin_trigger_event(params, gfal_gsiftp_domain(),
                                     GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_ENTER,
                                     "%s", checksum_type);

                checksum(src, checksum_type, checksum_src, sizeof(checksum_src),
                         0, 0);

                plugin_trigger_event(params, gfal_gsiftp_domain(),
                                     GFAL_EVENT_SOURCE, GFAL_EVENT_CHECKSUM_EXIT,
                                     "%s=%s", checksum_type, checksum_src);
            }
        }
        catch (const Glib::Error &e) {
            throw;
        }
        catch (...) {
            throw Glib::Error(gfal_gsiftp_domain(), EIO,
                              "Undefined Exception catched while getting the source checksum!!");
        }
    }

    // Transfer
    GError* transfer_error = NULL;
    {
        plugin_trigger_event(params, gfal_gsiftp_domain(), GFAL_EVENT_NONE,
                             GFAL_EVENT_TRANSFER_ENTER,
                             "(%s) %s => (%s) %s",
                             returnHostname(src).c_str(), src,
                             returnHostname(dst).c_str(), dst);
        CPP_GERROR_TRY

            gridftp_filecopy_copy_file_internal(_handle_factory, params,
                                                src, dst);
        CPP_GERROR_CATCH(&transfer_error);

        plugin_trigger_event(params, gfal_gsiftp_domain(), GFAL_EVENT_NONE,
                             GFAL_EVENT_TRANSFER_EXIT,
                             "(%s) %s => (%s) %s",
                             returnHostname(src).c_str(), src,
                             returnHostname(dst).c_str(), dst);
    }

    // If we got an error, clean the destination and throw
    if (transfer_error != NULL) {
        autoCleanFileCopy(params, transfer_error, dst);
        Gfal::gerror_to_cpp(&transfer_error);
    }

    // Validate destination checksum
    if (checksum_check) {
        plugin_trigger_event(params, gfal_gsiftp_domain(),
                GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_ENTER, "%s",
                checksum_type);

        checksum(dst, checksum_type, checksum_dst, sizeof(checksum_dst), 0, 0);
        gridftp_checksum_transfer_verify(checksum_src, checksum_dst,
                                         checksum_user_defined);

        plugin_trigger_event(params, gfal_gsiftp_domain(),
                GFAL_EVENT_DESTINATION, GFAL_EVENT_CHECKSUM_EXIT, "%s",
                checksum_type);
    }
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
