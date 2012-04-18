#pragma once
#ifndef GRIFTP_IFCE_FILECOPY_H
#define GRIFTP_IFCE_FILECOPY_H
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


#include <exceptions/gfalcoreexception.hpp>
#include <transfer/params_plugin_interface.hpp>
#include <transfer/gfal_transfer.h>
#include <common/gfal_types.h>
#include <common/gfal_constants.h>

#include "gridftpmodule.h"

class GridFTPFileCopyModule : public GridFTPDecorator {
	public:
		GridFTPFileCopyModule(GridFTPInterface* wrap);
		virtual ~GridFTPFileCopyModule(){};
		virtual int filecopy(gfalt_params_handle params, const char* src, const char* dst);
};


#endif /* GRIFTP_IFCE_FILECOPY_H */ 
