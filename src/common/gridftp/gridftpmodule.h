#pragma once
#ifndef GRIDFTOMODULE_H
#define GRIDFTOMODULE_H

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

#include <common/gfal_types.h>
#include <common/gfal_common_errverbose.h>
#include <libcpp/cpp_to_gerror.hpp>
#include <transfer/gfal_transfer_plugins.h>
#include <transfer/params_plugin_interface.hpp>

#include "gridftpinterface.h"
#include "gridftpdecorator.h"
#include "gridftpwrapper.h"

#include "gridftp_ifce_filecopy.h"

#include "grid_ftp_ifce_include.h"

class GridftpModule : public GridFTPDecorator
{
	public:
		GridftpModule(GridFTPInterface * wrap);
		virtual ~GridftpModule();


};

#endif /* GRIDFTOMODULE_H */ 
