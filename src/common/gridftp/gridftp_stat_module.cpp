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


#include "gridftp_stat_module.h"

const Glib::Quark scope_stat("Gridftp_stat_module::stat");
const Glib::Quark scope_access("Gridftp_stat_module::access");



void GridftpModule::stat(const char* path, struct stat * st){
	if(path== NULL || st == NULL)
		throw Glib::Error(scope_stat, EINVAL, "Invalid arguments path or stat ");
	gfal_log(GFAL_VERBOSE_TRACE," -> [GridftpModule::stat] ");
	gfal_globus_stat_t gl_stat;
	memset(&gl_stat,0, sizeof(gfal_globus_stat_t));
	internal_globus_gass_stat(path, &gl_stat);
	
	memset(st,0, sizeof(struct stat));
	st->st_mode = (mode_t) ((gl_stat.mode != -1)?gl_stat.mode:0);
//	st->st_mode |= (gl_stat.symlink_target != NULL)?(S_IFLNK):0;
	st->st_mode |= (gl_stat.type == GLOBUS_GASS_COPY_GLOB_ENTRY_DIR)?(S_IFDIR):(S_IFREG);
	st->st_size = (off_t) gl_stat.size;
	st->st_mtime = (time_t) (gl_stat.mdtm != -1)?(gl_stat.mdtm):0;

	gfal_log(GFAL_VERBOSE_TRACE," <- [GridftpModule::stat] ");	
}

void GridftpModule::access(const char*  path, int mode){
	if(path== NULL)
		throw Gfal::CoreException(scope_stat, "Invalid arguments path or stat ", EINVAL);
		
	gfal_log(GFAL_VERBOSE_TRACE," -> [Gridftp_stat_module::access] ");
	gfal_globus_stat_t gl_stat;
	memset(&gl_stat,0, sizeof(gfal_globus_stat_t));
	internal_globus_gass_stat(path, &gl_stat);
	
	if(gl_stat.mode == -1){ // mode not managed by server
		gfal_log(GFAL_VERBOSE_VERBOSE, "access request is not managed by this server %s , return access authorized by default", path);
		return;
	}
	
	const mode_t file_mode = (mode_t) gl_stat.mode;
	if( ((file_mode & ( S_IRUSR | S_IRGRP | S_IROTH)) == FALSE )
		&& ( mode & R_OK) )
		throw Gfal::CoreException(scope_access, "No read access ", EACCES);
		
	if( ((file_mode &  ( S_IWUSR | S_IWGRP | S_IWOTH) ) == FALSE)
		&& ( mode & W_OK) )
		throw Gfal::CoreException(scope_access, "No write access ", EACCES);	
		
	if( ((file_mode & ( S_IXUSR | S_IXGRP | S_IXOTH)  ) == FALSE)
		&& ( mode & W_OK) )
		throw Gfal::CoreException(scope_access, "No execute access ", EACCES);				

	gfal_log(GFAL_VERBOSE_TRACE," <- [Gridftp_stat_module::access] ");	
}

void GridftpModule::internal_globus_gass_stat(const char* path,  gfal_globus_stat_t * gl_stat){

	gfal_log(GFAL_VERBOSE_TRACE," -> [Gridftp_stat_module::globus_gass_stat] ");	
	std::auto_ptr<GridFTP_session> sess(_handle_factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(path)));
		
	globus_result_t res= globus_gass_copy_stat(sess->get_gass_handle(), (char*)path, sess->get_gass_attr(), gl_stat);
	gfal_globus_check_result("GridFTPFileCopyModule::internal_globus_gass_stat", res);
		
	errno =0; // clean bad errno number
	gfal_log(GFAL_VERBOSE_TRACE," <- [Gridftp_stat_module::globus_gass_stat] ");		
}


extern "C" int gfal_gridftp_statG(plugin_handle handle, const char* name, struct stat* buff, GError ** err){
	g_return_val_err_if_fail( handle != NULL && name != NULL
			&& buff != NULL , -1, err, "[gfal_gridftp_statG][gridftp] einval params");

	GError * tmp_err=NULL;
	int ret = -1;
	gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_statG]");
	CPP_GERROR_TRY
		(static_cast<GridftpModule*>(handle))->stat(name, buff);
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_statG]<-");
	G_RETURN_ERR(ret, tmp_err, err);	
}


extern "C" int gfal_gridftp_accessG(plugin_handle handle, const char* name, int mode, GError** err){
	g_return_val_err_if_fail( handle != NULL && name != NULL
						, -1, err, "[gfal_gridftp_statG][gridftp] einval params");	

	GError * tmp_err=NULL;
	int ret = -1;
	gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_accessG]");
	CPP_GERROR_TRY
		(static_cast<GridftpModule*>(handle))->access(name, mode);
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_accessG]<-");
	G_RETURN_ERR(ret, tmp_err, err);	
	
}

