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

#include <config/gfal_config.h>
#include <globus_ftp_client.h>
#include "gridftp_stat_module.h"
#include "gridftpwrapper.h"

static Glib::Quark gfal_gridftp_scope_stat(){
    return Glib::Quark("Gridftp_stat_module::stat");
}

static Glib::Quark gfal_gridftp_scope_access(){
    return Glib::Quark("Gridftp_stat_module::access");
}



void GridftpModule::stat(const char* path, struct stat * st){
	if(path== NULL || st == NULL)
        throw Glib::Error(gfal_gridftp_scope_stat(), EINVAL, "Invalid arguments path or stat ");
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
        throw Gfal::CoreException(gfal_gridftp_scope_stat(), "Invalid arguments path or stat ", EINVAL);
		
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
        throw Gfal::CoreException(gfal_gridftp_scope_access(), "No read access ", EACCES);
		
	if( ((file_mode &  ( S_IWUSR | S_IWGRP | S_IWOTH) ) == FALSE)
		&& ( mode & W_OK) )
        throw Gfal::CoreException(gfal_gridftp_scope_access(), "No write access ", EACCES);
		
	if( ((file_mode & ( S_IXUSR | S_IXGRP | S_IXOTH)  ) == FALSE)
		&& ( mode & X_OK) )
        throw Gfal::CoreException(gfal_gridftp_scope_access(), "No execute access ", EACCES);

	gfal_log(GFAL_VERBOSE_TRACE," <- [Gridftp_stat_module::access] ");	
}

static mode_t _str2mode_triad(const char* buffer)
{
    mode_t mode = 0;
    if (buffer[0] == 'r')
        mode |= S_IREAD;
    if (buffer[1] == 'w')
        mode |= S_IWRITE;
    switch (buffer[2]) {
        case 'x':
            mode |= S_IEXEC;
            break;
        case 't': case 's':
            mode |= S_IEXEC;
        case 'T': case 'S':
            mode |= S_ISUID;
            break;
    }
    return mode;
}

static mode_t _str2mode(const char* buffer)
{
    return (_str2mode_triad(buffer + 1)) |
           (_str2mode_triad(buffer + 4) >> 3) |
           (_str2mode_triad(buffer + 7) >> 6);
}

static void _setStatTimeout(gfal2_context_t context, GridFTP_Request_state* req){
    struct timespec t;
    t.tv_sec = gfal2_get_opt_integer_with_default(context, GRIDFTP_CONFIG_GROUP, "STAT_TIMEOUT", 600);
    t.tv_nsec =0;
    gfal_log(GFAL_VERBOSE_TRACE, "Set stat timeout to %d", t.tv_sec);
    req->init_timeout(&t);
}

void GridftpModule::internal_globus_gass_stat(const char* path,  gfal_globus_stat_t * gl_stat){

	gfal_log(GFAL_VERBOSE_TRACE," -> [Gridftp_stat_module::globus_gass_stat] ");	
    std::auto_ptr<GridFTP_session> sess(_handle_factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(path)));

    std::auto_ptr<Gass_attr_handler>  gass_attr_src( sess->generate_gass_copy_attr());

    globus_byte_t *buffer = NULL;
    globus_size_t buflen = 0;
    std::auto_ptr<GridFTP_Request_state> req(new GridFTP_Request_state(sess.get(), false));

    _setStatTimeout(_handle_factory->get_handle(), req.get());

    globus_result_t res = globus_ftp_client_stat(sess->get_ftp_handle(),
                                                 path, sess->get_op_attr_ftp(),
                                                 &buffer, &buflen,
                                                 globus_basic_client_callback,
                                                 req.get());
    gfal_globus_check_result(gfal_gridftp_scope_stat(), res);
    req->poll_callback(gfal_gridftp_scope_stat());

    char     mode[12], trash[64];
    unsigned count;

    errno = req->get_error_code();
    switch(errno){
        case 0:
            gfal_log(GFAL_VERBOSE_TRACE,"   <- [internal_globus_gass_stat] Got '%s'", buffer);
            // Expected: -rw-r--r--   1     root     root         1604 Nov 11 19:13 passwd
            if (sscanf((char*)buffer, "%s %u %s %s %lu",
                       mode, &count, trash, trash, &(gl_stat->size)) != 5) {
                errno = EIO;
            }
            else {
                // Modification time
                gl_stat->mdtm = -1;
                // File type
                switch (mode[0]) {
                    case '-':
                        gl_stat->type = GLOBUS_GASS_COPY_GLOB_ENTRY_FILE;
                        break;
                    case 'd':
                        gl_stat->type = GLOBUS_GASS_COPY_GLOB_ENTRY_DIR;
                        break;
                    default:
                        gl_stat->type = GLOBUS_GASS_COPY_GLOB_ENTRY_OTHER;
                }
                // Mode
                gl_stat->mode = _str2mode(mode);
            }
            break;
        default:
            req->err_report(gfal_gridftp_scope_stat());

    }

    globus_free(buffer);
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

