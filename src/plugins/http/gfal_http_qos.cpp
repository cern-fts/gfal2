/*
 * Copyright (c) CERN 2013-2015
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
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

#include <davix.hpp>
#include <copy/davixcopy.hpp>
#include <unistd.h>
#include <checksums/checksums.h>
#include <cstdio>
#include <cstring>
#include "gfal_http_plugin.h"

using namespace Davix;

void gfal_http_check_classes(plugin_handle plugin_data, const char *url, GError** err)
{
	DavixError* tmp_err=NULL;
	Context c;
	HttpRequest r(c, url, &tmp_err);

	if(!tmp_err)
		r.executeRequest(&tmp_err);
	if(tmp_err){
		std::cerr << " error in request : " << tmp_err->getErrMsg() << std::endl;
	}else{
		std::vector<char> body = r.getAnswerContentVec();
		std::string v(body.begin(), body.end());
		std::cout << "content "<< v << std::endl;
	}
}

