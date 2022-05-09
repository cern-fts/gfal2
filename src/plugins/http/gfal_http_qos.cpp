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
#include <json.h>
#include "gfal_http_plugin.h"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

using namespace Davix;

ssize_t gfal_http_check_classes(plugin_handle plugin_data, const char* url, const char* type,
                                char* buff, size_t s_buff, GError** err)
{
  ssize_t ret = -1;

  if (type != NULL && (strcmp(type, "dataobject") == 0 || strcmp(type, "container") == 0 )) {
		GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
		DavixError* dav_err = NULL;
		Context c;

		std::string uri(url);
		uri += "/cdmi_capabilities/";
		uri += type;

		HttpRequest r(c, uri, &dav_err);
		Davix::RequestParams req_params;
		davix->get_params(&req_params, Davix::Uri(url));
		r.setParameters(req_params);

		if (!dav_err) {
      r.executeRequest(&dav_err);
    }

		if (dav_err) {
			std::cerr << " error in request of getting available QoS classes: " << dav_err->getErrMsg() << std::endl;
      davix2gliberr(dav_err, err, __func__);
      Davix::DavixError::clearError(&dav_err);
		} else {
			std::vector<char> body = r.getAnswerContentVec();
			std::string response(body.begin(), body.end());
			json_object* info = json_tokener_parse(response.c_str());
			std::string classes = json_object_get_string(json_object_object_get(info, "children"));

			// Remove all extra chars and create a comma separated string to return
			classes.erase(std::remove(classes.begin(), classes.end(), '['), classes.end());
			classes.erase(std::remove(classes.begin(), classes.end(), ']'), classes.end());
			classes.erase(std::remove(classes.begin(), classes.end(), ' '), classes.end());
			classes.erase(std::remove(classes.begin(), classes.end(), '"'), classes.end());

			// Adding prefix of cdmi capabilitiesURI
			std::string stype(type);
			std::string prefix = "/cdmi_capabilities/" + stype + "/";
			std::istringstream iss(classes);
			std::string classToken;
			classes = "";
			while (std::getline(iss, classToken, ','))
			{
				classes += prefix + classToken + ",";
			}

			// Remove final comma
			classes.erase(classes.size() - 1);

		  if (classes.size() < s_buff) {
        std::strcpy(buff, classes.c_str());
        ret = classes.size() + 1;
		  }	else {
        gfal2_set_error(err, http_plugin_domain, ENOMEM, __func__, "response larger than allocated buffer size [%ld]", s_buff);
		  }
		}
	} else {
		gfal2_set_error(err, http_plugin_domain, EINVAL, __func__, "type argument should be either dataobject or container");
	}

	return ret;
}

ssize_t gfal_http_check_file_qos(plugin_handle plugin_data, const char* url, char* buff, size_t s_buff, GError** err)
{
	GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
	DavixError* dav_err = NULL;
  ssize_t ret = -1;
	Context c;

	std::string uri(url);
	HttpRequest r(c, uri, &dav_err);
	Davix::RequestParams req_params;
	davix->get_params(&req_params, Davix::Uri(url));
	r.setParameters(req_params);

	if (!dav_err) {
    r.executeRequest(&dav_err);
  }

	if (dav_err) {
		std::cerr << " error in request of checking file QoS: " << dav_err->getErrMsg() << std::endl;
    davix2gliberr(dav_err, err, __func__);
    Davix::DavixError::clearError(&dav_err);
	} else {
		std::vector<char> body = r.getAnswerContentVec();
		std::string response(body.begin(), body.end());

		json_object *info = json_tokener_parse(response.c_str());
		std::string qos_class = json_object_get_string(json_object_object_get(info, "capabilitiesURI"));
		qos_class.erase(std::remove(qos_class.begin(), qos_class.end(), '"'), qos_class.end());

    if (qos_class.size() < s_buff) {
      std::strcpy(buff, qos_class.c_str());
      ret = qos_class.size() + 1;
    }	else {
      gfal2_set_error(err, http_plugin_domain, ENOMEM, __func__, "response larger than allocated buffer size [%ld]", s_buff);
    }
	}

	return ret;
}

ssize_t gfal_http_check_qos_available_transitions(plugin_handle plugin_data, const char* qos_class_url,
                                                  char* buff, size_t s_buff, GError** err)
{
	GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
  DavixError* dav_err = NULL;
  ssize_t ret = -1;
  Context c;

  std::string uri(qos_class_url);
  HttpRequest r(c, uri, &dav_err);
  Davix::RequestParams req_params;
  davix->get_params(&req_params, Davix::Uri(qos_class_url));
  r.setParameters(req_params);

  if (!dav_err) {
    r.executeRequest(&dav_err);
  }

  if (dav_err) {
    std::cerr << " error in request of checking file QoS: " << dav_err->getErrMsg() << std::endl;
    davix2gliberr(dav_err, err, __func__);
    Davix::DavixError::clearError(&dav_err);
  } else {
    std::vector<char> body = r.getAnswerContentVec();
    std::string response(body.begin(), body.end());

    json_object *info = json_tokener_parse(response.c_str());
    json_object *metadata = json_object_object_get(info, "metadata");
    std::string transitions = json_object_get_string(json_object_object_get(metadata, "cdmi_capabilities_allowed"));

    // Remove all extra chars and create a comma separated string to return
    transitions.erase(std::remove(transitions.begin(), transitions.end(), '['), transitions.end());
    transitions.erase(std::remove(transitions.begin(), transitions.end(), ']'), transitions.end());
    transitions.erase(std::remove(transitions.begin(), transitions.end(), ' '), transitions.end());
    transitions.erase(std::remove(transitions.begin(), transitions.end(), '"'), transitions.end());
    transitions.erase(std::remove(transitions.begin(), transitions.end(), '\\'), transitions.end());

    if (transitions.size() < s_buff) {
      std::strcpy(buff, transitions.c_str());
      ret = transitions.size() + 1;
    }	else {
      gfal2_set_error(err, http_plugin_domain, ENOMEM, __func__, "response larger than allocated buffer size [%ld]", s_buff);
    }
  }

  return ret;
}


/*
 TODO: Have all get requests being done in a separate method, at the moment there is an issue with the second level json reading of gfal_http_check_qos_available_transitions
 	   when all tests are running

std::string response = execute_get_request_to_cdmi(plugin_data, qosClassUrl);

std::string execute_get_request_to_cdmi(plugin_handle plugin_data, const char *url)
{
	GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
	Context c;
	DavixError* tmp_err=NULL;

	std::string uri(url);
	HttpRequest r(c, uri, &tmp_err);
	Davix::RequestParams req_params;
	davix->get_params(&req_params, Davix::Uri(url), false);
	r.setParameters(req_params);

	if(!tmp_err)
		r.executeRequest(&tmp_err);
	if(tmp_err){
			std::cerr << " ERROR on get Request to the CDMI server: " << tmp_err->getErrMsg() << std::endl;
	} else {
		std::vector<char> body = r.getAnswerContentVec();
		std::string response(body.begin(), body.end());
		return response;
	}
	return "";
}*/

ssize_t gfal_http_check_target_qos(plugin_handle plugin_data, const char* url, char* buff, size_t s_buff, GError** err)
{
	GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
	DavixError* dav_err = NULL;
	ssize_t ret = -1;
	Context c;

	std::string uri(url);
	HttpRequest r(c, uri, &dav_err);

	Davix::RequestParams req_params;
	davix->get_params(&req_params, Davix::Uri(url));
	r.setParameters(req_params);

	if (!dav_err) {
    r.executeRequest(&dav_err);
  }

	if (dav_err) {
		std::cerr << " error in request of checking file QoS: " << dav_err->getErrMsg() << std::endl;
    davix2gliberr(dav_err, err, __func__);
    Davix::DavixError::clearError(&dav_err);
	} else {
		std::vector<char> body = r.getAnswerContentVec();
		std::string response(body.begin(), body.end());

		json_object *info = json_tokener_parse(response.c_str());
		json_object *metadata = json_object_object_get(info, "metadata");
		json_object *target = json_object_object_get(metadata, "cdmi_capabilities_target");
    std::string target_qos = "";

		if (target != NULL) {
      target_qos = json_object_get_string(target);

      // Remove all extra chars
      target_qos.erase(std::remove(target_qos.begin(), target_qos.end(), '['), target_qos.end());
      target_qos.erase(std::remove(target_qos.begin(), target_qos.end(), ']'), target_qos.end());
      target_qos.erase(std::remove(target_qos.begin(), target_qos.end(), ' '), target_qos.end());
      target_qos.erase(std::remove(target_qos.begin(), target_qos.end(), '"'), target_qos.end());
      target_qos.erase(std::remove(target_qos.begin(), target_qos.end(), '\\'), target_qos.end());
    }

    if (target_qos.size() < s_buff) {
      std::strcpy(buff, target_qos.c_str());
      ret = target_qos.size() + 1;
    }	else {
      gfal2_set_error(err, http_plugin_domain, ENOMEM, __func__, "response larger than allocated buffer size [%ld]", s_buff);
    }
	}

	return ret;
}

int gfal_http_change_object_qos(plugin_handle plugin_data, const char* url, const char* target_qos, GError** err)
{
	GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
	DavixError* dav_err = NULL;
	Context c;

	std::string uri(url);
	std::stringstream body;
	body << "{\"capabilitiesURI\":\"" << target_qos << "\"}";
	PutRequest pr(c, uri, &dav_err);
	Davix::RequestParams req_params;
	davix->get_params(&req_params, Davix::Uri(url));
  req_params.addHeader("Content-Type", "application/cdmi-object");
	pr.setParameters(req_params);
	pr.setRequestBody(body.str());

	if (!dav_err) {
		pr.executeRequest(&dav_err);
	}

	if (dav_err || !http_cdmi_code_is_valid(pr.getRequestCode())) {
		if (dav_err) {
      std::cerr << " error in request of changing file QoS: " << dav_err->getErrMsg() << std::endl;
      davix2gliberr(dav_err, err, __func__);
      Davix::DavixError::clearError(&dav_err);
    } else {
			std::cerr << " error in request of changing file QoS " << std::endl;
		}

		return -1;
	}

	return 0;
}

bool http_cdmi_code_is_valid(int code)
{	/* Should expect 204 as per CDMI document page 8 for a PUT request */
  switch (code) {
    case 200:           /* OK */
    case 201:           /* OK */
    case 202:           /* Accepted */
    case 204:           /* No Content */
      return true;
    default:
      return false;
  }
}
