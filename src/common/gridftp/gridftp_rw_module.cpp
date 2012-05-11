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


#include "gridftp_plugin_loader.h"
#include "gridftp_rw_module.h"


const Glib::Quark scope_open("GridftpModule::open");

const size_t readdir_len = 65000;

struct GridFTP_File_desc{
	std::auto_ptr<GridFTP_stream_state> stream;
	int open_flags;
	off_t current_offset;
	GridFTP_File_desc(GridFTP_stream_state * s, int open_flags) : stream(s){
		this->open_flags = open_flags;
		current_offset = 0;
	}
	
};


// gridFTP open is restricted by the protocol : READ or Write but not both
//
gfal_file_handle GridftpModule::open(const char* url, int flag, mode_t mode)
{
	GridFTP_Request_state status;
	
	std::auto_ptr<GridFTP_File_desc> desc(new GridFTP_File_desc(
							new GridFTP_stream_state(_handle_factory->gfal_globus_ftp_take_handle()),
							flag)
						);
	gfal_print_verbose(GFAL_VERBOSE_TRACE," -> [GridftpModule::open] ");	
	globus_result_t res	;
	
	if( desc->open_flags & O_RDONLY){	// bet on a full read
		 gfal_print_verbose(GFAL_VERBOSE_TRACE," -> initialize FTP GET operations... ");	
		 res = globus_ftp_client_get( // start req
					&(desc->stream->sess->handle),
					url,
					NULL,
					NULL,
					globus_basic_client_callback,
					&status);
		gfal_globus_check_result(scope_open, res);						
	}else if(desc->open_flags & ( O_WRONLY | O_CREAT ) ){
		 gfal_print_verbose(GFAL_VERBOSE_TRACE," -> initialize FTP PUT operations... ");		
		res = globus_ftp_client_put( // bet on a full write
					&(desc->stream->sess->handle),
					url,
					NULL,
					NULL,
					globus_basic_client_callback,
					&status);	
		gfal_globus_check_result(scope_open, res);							
	}else{
		 gfal_print_verbose(GFAL_VERBOSE_TRACE," -> no operation initialization ...");
		 desc->stream.reset();	
	}

	gfal_print_verbose(GFAL_VERBOSE_TRACE," <- [GridftpModule::open] ");	
	return gfal_file_handle_ext_new(plugin_name(), (gpointer) desc.release(), NULL);
}

// open C bind
extern "C" gfal_file_handle gfal_gridftp_openG(plugin_handle handle, const char* url, int flag, mode_t mode, GError** err){
	g_return_val_err_if_fail( handle != NULL && url != NULL
			, NULL, err, "[gfal_gridftp_openG][gridftp] einval params");

	GError * tmp_err=NULL;
	gfal_file_handle ret = NULL;
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_openG]");
	CPP_GERROR_TRY
		ret = ((static_cast<GridftpModule*>(handle))->open(url,flag, mode));
	CPP_GERROR_CATCH(&tmp_err);
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_openG]<-");
	G_RETURN_ERR(ret, tmp_err, err);	
}
