#pragma once
#ifndef GRIDFTPDECORATOR_H
#define GRIDFTPDECORATOR_H
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



#include "gridftpinterface.h"

class GridFTPDecorator: public GridFTPInterface
{
	public:
		GridFTPDecorator(GridFTPInterface * i);
		virtual ~GridFTPDecorator();

		virtual gfal_handle get_handle();	

		virtual gfal_globus_copy_handle_t take_globus_handle() ;
		virtual void release_globus_handle(gfal_globus_copy_handle_t*) ;
		virtual void globus_check_result(const std::string & nmspace, gfal_globus_result_t res);		
	protected:
		/* add your private declarations */
		GridFTPInterface* _wrap;

};

#endif /* GRIDFTPDECORATOR_H */ 
