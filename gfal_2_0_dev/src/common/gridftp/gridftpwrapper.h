#ifndef GRIDFTPWRAPPER_H
#define GRIDFTPWRAPPER_H
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

#include "gridftpmodule.h"

class GridFTPWrapper : public GridFTPInterface
{
	public:
		GridFTPWrapper(gfal_handle handle );
		virtual ~GridFTPWrapper();

		virtual gfal_globus_copy_handle_t take_globus_handle() ;
		virtual void release_globus_handle(gfal_globus_copy_handle_t*) ;
		virtual void globus_check_result(const std::string & nmspace, gfal_globus_result_t res);
	
	protected:
		gfal_handle _handle;
		virtual gfal_handle get_handle();
};

#endif /* GRIDFTPWRAPPER_H */ 
