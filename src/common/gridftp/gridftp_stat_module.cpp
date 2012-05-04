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


void GridftpModule::stat(const char* path, struct stat * st){
	if(path== NULL || st == NULL)
		throw Glib::Error(scope_stat, EINVAL, "Invalid arguments path or stat ");
	gfal_print_verbose(GFAL_VERBOSE_TRACE," -> [GridftpModule::stat] ");
	gfal_globus_stat_t gl_stat;
	memset(&gl_stat,0, sizeof(gfal_globus_stat_t));
	internal_globus_gass_stat(path, &gl_stat);
	
	memset(st,0, sizeof(struct stat));
	st->st_mode = (mode_t) ((gl_stat.mode != -1)?gl_stat.mode:0);
//	st->st_mode |= (gl_stat.symlink_target != NULL)?(S_IFLNK):0;
	st->st_mode |= (gl_stat.type == GLOBUS_GASS_COPY_GLOB_ENTRY_DIR)?(S_IFDIR):(S_IFREG);
	st->st_size = (off_t) gl_stat.size;
	st->st_mtime = (time_t) (gl_stat.mdtm != -1)?(gl_stat.mdtm):0;

	gfal_print_verbose(GFAL_VERBOSE_TRACE," <- [GridftpModule::stat] ");	
}

void GridftpModule::internal_globus_gass_stat(const char* path,  gfal_globus_stat_t * gl_stat){

	gfal_print_verbose(GFAL_VERBOSE_TRACE," -> [Gridftp_stat_module::globus_gass_stat] ");	
	gfal_globus_copy_handle_t h = _handle_factory->take_globus_gass_handle();
	gfal_globus_copy_attr_t * attr = _handle_factory->take_globus_gass_attr();
	
	globus_result_t res= globus_gass_copy_stat(&h, (char*)path, attr, gl_stat);
	
	_handle_factory->release_globus_gass_attr(attr);
	_handle_factory->release_globus_gass_handle(&h);
	gfal_globus_check_result("Gridftp_stat_module::globus_gass_stat", res);	
	errno =0; // clean bad errno number
	gfal_print_verbose(GFAL_VERBOSE_TRACE," <- [Gridftp_stat_module::globus_gass_stat] ");		
}


extern "C" int gfal_gridftp_statG(plugin_handle handle, const char* name, struct stat* buff, GError ** err){
	g_return_val_err_if_fail( handle != NULL && name != NULL
			&& buff != NULL , -1, err, "[gfal_gridftp_statG][gridftp] einval params");

	GError * tmp_err=NULL;
	int ret = -1;
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_statG]");
	CPP_GERROR_TRY
		(static_cast<GridftpModule*>(handle))->stat(name, buff);
		ret = 0;
	CPP_GERROR_CATCH(&tmp_err);
	gfal_print_verbose(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_statG]<-");
	G_RETURN_ERR(ret, tmp_err, err);	
}



