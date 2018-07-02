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
#include <sstream>
#include <json/json.h>
#include "gfal_http_plugin.h"

using namespace Davix;

void gfal_http_check_classes(plugin_handle plugin_data, const char *url, const char *type, GError** err)
{
	if (type != NULL && (strcmp(type, "dataobject") == 0 || strcmp(type, "container") == 0 )) {
		DavixError* tmp_err=NULL;
		Context c;


		std::string uri(url);
		uri += "/cdmi_capabilities/";
		uri += type;
		HttpRequest r(c, uri, &tmp_err);
		Davix::RequestParams req_params = new RequestParams();
		//get_params(req_params, const Davix::Uri& uri, false)

		std::stringstream ss;
		ss << "Bearer " << "eyJraWQiOiJyc2ExIiwiYWxnIjoiUlMyNTYifQ.eyJzdWIiOiJmZWE1ZTZlMi0wYjlmLTQwZjUtYjE5OC00YmI3YWU0YjIzNGEiLCJpc3MiOiJodHRwczpcL1wvaWFtLmV4dHJlbWUtZGF0YWNsb3VkLmV1XC8iLCJleHAiOjE1MzA1NDY1OTksImlhdCI6MTUzMDU0Mjk5OSwianRpIjoiNDQ4YzgzZGMtNTRhYi00NmU5LWIwZDEtN2FmYTY4NThmZGFkIn0.HUNcbt5fF9368XdjNxP1eltAMu3auFUOabTo8ZoDOcpL7E7b1d_Z-Uz-x6vkRUAG80koPtkeGXwN34vkujE512vMF-FaGJkmlw-eo65U4OxnkT9w_UJesk4ZbDVlP7-fNepg0AJeEzkzp8UE5dE81B2K40K9s9uclwBQuhukV-s";
		req_params.addHeader("Authorization", ss.str());
		r.setParameters(req_params);

		if(!tmp_err)
			r.executeRequest(&tmp_err);
		if(tmp_err){
			std::cerr << " error in request : " << tmp_err->getErrMsg() << std::endl;
		}else{
			std::vector<char> body = r.getAnswerContentVec();
			std::string response(body.begin(), body.end());
			json_object *info = json_tokener_parse(response.c_str());

			std::string classes = json_object_get_string(json_object_object_get(info, "children"));
			std::cout << "QoS classes: "<< classes << std::endl;

			std::cout << "content "<< response << std::endl;
		}
	} else {
		gfal2_set_error(err, http_plugin_domain, EINVAL, __func__, "type argument should be either dataobject or container");
	}
}

