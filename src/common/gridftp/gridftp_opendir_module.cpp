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
#include "gridftp_opendir_module.h"


const Glib::Quark scope_opendir("GridftpModule::opendir");

const size_t readdir_len = 65000;

struct GridFTP_Dir_desc{
	struct dirent dir;
	char buff[readdir_len+1]; // buffer used for 
	std::string list; // concated list of dirs
	std::auto_ptr<GridFTP_stream_state> stream;
	GridFTP_Dir_desc(GridFTP_stream_state * s) : stream(s){
			*(buff+readdir_len)='\0';
			memset(&dir, 0, sizeof(struct dirent));
	}
	
};



gfal_file_handle GridftpModule::opendir(const char* path)
{
	GridFTP_Request_state status;
	ssize_t r_size;
	std::auto_ptr<GridFTP_Dir_desc> desc(new GridFTP_Dir_desc(
						new GridFTP_stream_state(_handle_factory->gfal_globus_ftp_take_handle())
						));
	gfal_print_verbose(GFAL_VERBOSE_TRACE," -> [GridftpModule::opendir] ");	
	
	globus_result_t res = globus_ftp_client_list( // start req
				&(desc->stream->sess->handle),
				path,
				NULL,
				globus_basic_client_callback,
    			&status);
	gfal_globus_check_result(scope_opendir, res);
	
	r_size= gridftp_read_stream(scope_opendir, (desc->stream.get()),
				desc->buff, readdir_len); // initiate reading stream
	*(desc->buff + r_size) = '\0';
	desc->list = std::string(desc->buff);
	
	gfal_print_verbose(GFAL_VERBOSE_TRACE," <- [GridftpModule::opendir] ");	
	return gfal_file_handle_ext_new(plugin_name(), (gpointer) desc.release(), NULL);
}

// try to extract dir information
int gridftp_readdir_desc_parser(GridFTP_Dir_desc* desc){
	const char * c_list = desc->list.c_str();
	char* p,*p1;
	if( (p = strchr((char*) c_list, '\n')) == NULL)
		return 0; // no new entry, c'est la fin des haricots
	p1 = (char*)mempcpy(desc->dir.d_name, c_list, std::min((long int)NAME_MAX-1, p-c_list));
	*p1 = '\0';
    while( *(--p1) == '\r' || *p1 == '\n') // clear new line madness
		*p1 = '\0';
	desc->list = std::string(p+1);
	return 1;	
}

struct dirent * GridftpModule::readdir(gfal_file_handle  fh){
	GridFTP_Dir_desc* desc = static_cast<GridFTP_Dir_desc*>(fh->fdesc);	
	ssize_t r_size;
	gfal_print_verbose(GFAL_VERBOSE_TRACE," -> [GridftpModule::readdir] ");	
	
	while(gridftp_readdir_desc_parser(desc) == 0){
		if( (r_size = gridftp_read_stream(scope_opendir, (desc->stream.get()),
				desc->buff, readdir_len)) == 0) // end of stream
				return NULL;
		*(desc->buff + r_size) = '\0';			
		desc->list+= std::string(desc->buff);	
	}
	
	gfal_print_verbose(GFAL_VERBOSE_VERBOSE,"  list file %s ", desc->dir.d_name);		
	gfal_print_verbose(GFAL_VERBOSE_TRACE,"  [GridftpModule::readdir] <- ");		
	return &(desc->dir);
}

int GridftpModule::closedir(gfal_file_handle  fh){
	gfal_print_verbose(GFAL_VERBOSE_TRACE,"  -> [GridftpModule::closedir]");	
	GridFTP_Dir_desc* desc = static_cast<GridFTP_Dir_desc*>(fh->fdesc);	
	if(desc){
		delete desc;
		gfal_file_handle_delete(fh);
	}
	gfal_print_verbose(GFAL_VERBOSE_TRACE,"  [GridftpModule::closedir]  <- ");	
	return 0;
}

extern "C" gfal_file_handle gfal_gridftp_opendirG(plugin_handle handle , const char* path,  GError** err){
	g_return_val_err_if_fail( handle != NULL && path != NULL
			, NULL, err, "[gfal_gridftp_opendirG][gridftp] einval params");

	GError * tmp_err=NULL;
	gfal_file_handle ret = NULL;
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_opendirG]");
	CPP_GERROR_TRY
		ret = ((static_cast<GridftpModule*>(handle))->opendir(path));
	CPP_GERROR_CATCH(&tmp_err);
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_opendirG]<-");
	G_RETURN_ERR(ret, tmp_err, err);	
}


extern "C" struct dirent* gfal_gridftp_readdirG(plugin_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail( handle != NULL && fh != NULL
			, NULL, err, "[gfal_gridftp_readdirG][gridftp] einval params");

	GError * tmp_err=NULL;
	struct dirent* ret = NULL;
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_readdirG]");
	CPP_GERROR_TRY
		ret = ((static_cast<GridftpModule*>(handle))->readdir(fh));
	CPP_GERROR_CATCH(&tmp_err);
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_readdirG] <-");
	G_RETURN_ERR(ret, tmp_err, err);	
}

extern "C" int gfal_gridftp_closedirG(plugin_handle handle, gfal_file_handle fh, GError** err){
	g_return_val_err_if_fail( handle != NULL 
			, -1, err, "[gfal_gridftp_readdirG][gridftp] einval params");
	GError * tmp_err=NULL;
	int ret = -1;
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_closedirG]");
	CPP_GERROR_TRY
		((static_cast<GridftpModule*>(handle))->closedir(fh));
		ret =0;
	CPP_GERROR_CATCH(&tmp_err);
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_closedirG] <-");
	G_RETURN_ERR(ret, tmp_err, err);	
}

