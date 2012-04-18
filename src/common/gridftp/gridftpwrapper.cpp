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


#include "gridftpwrapper.h"


GridFTPWrapper::GridFTPWrapper(gfal_handle handle) : _handle(handle)
{
	
}




GridFTPWrapper::~GridFTPWrapper()
{
	
}

gfal_handle GridFTPWrapper::get_handle(){
	return _handle;
}



gfal_globus_copy_handle_t GridFTPWrapper::take_globus_handle(){
	gfal_globus_copy_handle_t globus_handle;
	globus_result_t res = globus_gass_copy_handle_init 	(&globus_handle, GLOBUS_NULL);
	globus_check_result("GridFTPWrapper::release_globus_handle", res);	
	return globus_handle;
		
}
void GridFTPWrapper::release_globus_handle(gfal_globus_copy_handle_t *globus_handle){
	globus_result_t res = globus_gass_copy_handle_destroy(globus_handle);
	globus_check_result("GridFTPWrapper::release_globus_handle", res);	
}

/*
 *  dirty function to convert error code from globus
 *  In the current state, globus provides no way to convert gridftp error code :'(
 * */
static int scan_errstring(const char *p) {

    int ret = ECOMM;
    if (p == NULL) return ret;

    if (strstr(p, "o such file"))
        ret = ENOENT;
    else if (strstr(p, "ermission denied") || strstr(p, "credential"))
        ret = EACCES;
    else if (strstr(p, "exists"))
        ret = EEXIST;
    return ret;
}

void GridFTPWrapper::globus_check_result(const std::string & nmspace, gfal_globus_result_t res){
	if(res != GLOBUS_SUCCESS){
		char errbuff[GFAL_URL_MAX_LEN];
		char * glob_str;
		*errbuff='\0';
		globus_object_t * error=globus_error_get(res); // get error from result code 
		if(error == NULL)
			throw Gfal::CoreException(nmspace, "Unknow error  unable to map result code to globus error", ENOENT);
		
		if((glob_str = globus_error_print_friendly(error)) != NULL) // convert err string
			g_strlcpy( errbuff, glob_str, GFAL_URL_MAX_LEN);
		const int globus_errno = scan_errstring(errbuff); // try to get errno
		globus_object_free(error);
		throw Gfal::CoreException(nmspace, errbuff, globus_errno );
	}
}



