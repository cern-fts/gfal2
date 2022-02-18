/*
 * Copyright (c) CERN 2013-2021
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

#include <sstream>
#include <cstring>
#include "json.h"

#include "gfal_http_plugin.h"
#include "exceptions/gfalcoreexception.hpp"

/*
 * NOTE:
 * The token interaction in this unit file is largely based
 * on the x509-scitokens-issuer project (https://github.com/scitokens/x509-scitokens-issuer).
 *
 * As SE-issued tokens are to become an integral part of the Gfal2 HTTP plugin,
 * the goal was to move the token-related client interaction here.
 */

using namespace Davix;

ssize_t gfal_http_token_retrieve(plugin_handle plugin_data, const char* url, const char* issuer,
                                 gboolean write_access, unsigned validity, const char* const* activities,
                                 char* buff, size_t s_buff, GError** err)
{
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    TokenRetriever* retriever_chain = NULL;
    ssize_t ret = -1;

    // Emulate GfalHttpPluginData::get_params(..) without the get_credentials(..) part
    Davix::RequestParams params = davix->reference_params;
    davix->get_params_internal(params, Davix::Uri(url));

    if (issuer && *issuer) {
        retriever_chain = new SciTokensRetriever(issuer);
        retriever_chain->add(new MacaroonRetriever(issuer));
    } else {
        retriever_chain = new MacaroonRetriever();
    }

    std::string token;
    TokenRetriever* retriever = retriever_chain;

    while (retriever != NULL) {
        try {
            gfal_http_token_t http_token = retriever->retrieve_token(Davix::Uri(url), params,
                                                                     write_access, validity, activities);
            token = http_token.token;
            break;
        } catch (const Gfal::CoreException& e) {
            gfal2_log(G_LOG_LEVEL_INFO, "(SEToken) Error during token retrieval: %s", e.what());
            retriever = retriever->next();
        }
    }

    if (token.empty()) {
        gfal2_set_error(err, http_plugin_domain, ENODATA, __func__,
                        "Could not retrieve token for %s", url);
    } else if (token.size() >= s_buff) {
        gfal2_set_error(err, http_plugin_domain, ENOMEM, __func__,
                        "response larger than allocated buffer size [%ld]", s_buff);
    } else {
        std::strcpy(buff, token.c_str());
        ret = token.size() + 1;
    }

    delete retriever_chain;
    return ret;
}

// --------------------------------------------------------
// General Token Retrieval implementation
// --------------------------------------------------------

TokenRetriever::TokenRetriever(std::string label, std::string issuer):
        label(std::move(label)), issuer(std::move(issuer)), context(),
        discovery_fallback(false), token_key("access_token")
{
    context.loadModule("grid");
}

TokenRetriever* TokenRetriever::add(TokenRetriever* elem)
{
    _next.reset(elem);
    return _next.get();
}

TokenRetriever* TokenRetriever::next()
{
    return _next.get();
}

Uri TokenRetriever::format_protocol(const Uri& _url)
{
    Uri url(_url);

    if (url.getStatus() != Davix::StatusCode::OK) {
        std::stringstream errmsg;
        errmsg << "Failed to parse url '" << url.getString() << "'";
        throw Gfal::CoreException(http_plugin_domain, EINVAL, errmsg.str());
    }

    if (url.getProtocol() == "davs") {
        url.setProtocol("https");
    }

    if (url.getProtocol() != "https") {
        throw Gfal::CoreException(http_plugin_domain, EINVAL, "Token request must be done over HTTPs");
    }

    return url;
}

std::string TokenRetriever::get_token_endpoint(RequestParams& params)
{
    Uri url = format_protocol(Uri(issuer));
    std::string oauth_endpoint = _metadata_endpoint(url);

    std::string endpoint = _endpoint_discovery(oauth_endpoint, params);

    if (!endpoint.empty() || !discovery_fallback) {
        return endpoint;
    }

    std::string config_url = issuer;

    if (config_url[config_url.size() -1 ] != '/') {
        config_url += "/";
    }
    config_url += ".well-known/openid-configuration";

    return _endpoint_discovery(config_url, params);
}

std::string TokenRetriever::_metadata_endpoint(const Uri& url)
{
    std::stringstream endpoint;
    endpoint << url.getProtocol() << "://" << url.getHost();

    if (url.getPort()) {
        endpoint << ":" << url.getPort();
    }
    endpoint << "/.well-known/oauth-authorization-server";

    if (url.getPath() != "/") {
        endpoint << url.getPath();
    }
    return endpoint.str();
}

std::string TokenRetriever::_endpoint_discovery(const std::string& metadata_url,
                                                RequestParams& params)
{
    DavixError* err = NULL;
    GetRequest request(context, metadata_url, &err);
    request.setParameters(params);

    std::string response = perform_request(request, "Token endpoint discovery");
    return parse_json_response(response, "token_endpoint");
}

std::string TokenRetriever::parse_json_response(const std::string& response, const std::string& key)
{
    json_object* json_response;
    json_object* json_item;

    if (response.empty()) {
        throw Gfal::CoreException(http_plugin_domain, EINVAL, "Response with no data");
    }

    json_response = json_tokener_parse(response.c_str());

    if (!json_response) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "(SEToken) Unparsable JSON: %s", response.c_str());
        throw Gfal::CoreException(http_plugin_domain, EINVAL, "Response was not valid JSON");
    }

    if (!json_object_object_get_ex(json_response, key.c_str(), &json_item)) {
        std::stringstream errmsg;
        errmsg << "Response did not include '" << key << "' key";
        json_object_put(json_response);
        throw Gfal::CoreException(http_plugin_domain, EINVAL, errmsg.str());
    }

    const char* value = json_object_get_string(json_item);

    if (!value) {
        std::stringstream errmsg;
        errmsg << "Key '" << key << "' was not a string";
        json_object_put(json_response);
        throw Gfal::CoreException(http_plugin_domain, EINVAL, errmsg.str());
    }

    std::string result(value);
    json_object_put(json_response);

    if (result.empty()) {
        std::stringstream errmsg;
        errmsg << "Extracted value for key '" << key << "' is empty";
        throw Gfal::CoreException(http_plugin_domain, EINVAL, errmsg.str());
    }

    return result;
}

std::string TokenRetriever::perform_request(HttpRequest& request, std::string description)
{
    DavixError* err = NULL;

    if (description.empty()) {
        description = label;
    }

    if (request.executeRequest(&err))
    {
        std::stringstream errmsg;
        errmsg << description << " request failed: " << err->getErrMsg();
        throw Gfal::CoreException(http_plugin_domain, davix2errno(err->getStatus()), errmsg.str());
    }

    if (request.getRequestCode() != 200) {
        std::stringstream errmsg;
        errmsg << description << " request failed with status code: " << request.getRequestCode();
        throw Gfal::CoreException(http_plugin_domain, davix2errno(err->getStatus()), errmsg.str());
    }

    return std::string(request.getAnswerContent());
}

gfal_http_token_t TokenRetriever::retrieve_token(const Uri& _url,
                                                 const RequestParams& _params,
                                                 bool write_access,
                                                 unsigned validity,
                                                 const char* const* activities)
{
    Uri url = format_protocol(_url);
    RequestParams params(_params);
    params.setProtocol(RequestProtocol::Http);
    std::string path = url.getPath();
    std::string endpoint;

    try {
        endpoint = (!issuer.empty()) ? get_token_endpoint(params) : "";
    } catch (const Gfal::CoreException& e) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "(SEToken) Error during issuer endpoint discovery: %s", e.what());
    }

    // Let sub-classes validate token endpoint
    if (!validate_endpoint(endpoint, url)) {
        throw Gfal::CoreException(http_plugin_domain, EINVAL, "Invalid or empty token issuer endpoint");
    }

    DavixError* err = NULL;
    PostRequest request(context, endpoint, &err);
    request.setParameters(params);

    // Let sub-classes prepare the request
    prepare_request(request, path, write_access, validity, activities);

    std::string response = perform_request(request);
    std::string stoken = parse_json_response(response, token_key);

    gfal_http_token_t token = {stoken, validity, write_access};
    return token;
}

// --------------------------------------------------------
// Macaroon-specific Retrieval implementation
// --------------------------------------------------------

MacaroonRetriever::MacaroonRetriever():
    MacaroonRetriever("")
{
}

MacaroonRetriever::MacaroonRetriever(std::string issuer):
    TokenRetriever("Macaroon", std::move(issuer)),
    is_oauth(false)
{
    discovery_fallback = true;
}

bool MacaroonRetriever::validate_endpoint(std::string& endpoint, const Uri& url)
{
    is_oauth = !endpoint.empty();

    if (endpoint.empty()) {
        endpoint = url.getString();
    }

    return true;
}

void MacaroonRetriever::prepare_request(HttpRequest& request, const std::string& path,
                                        bool write_access, unsigned validity, const char* const* activities)
{
    std::vector<std::string> v_activities = _activities(write_access, activities);

    if (is_oauth) {
        request.addHeaderField("Content-Type", "application/x-www-form-urlencoded");
        request.addHeaderField("Accept", "application/json");
        request.setRequestBody(oauth_request_content(path, validity, v_activities));
    } else {
        request.addHeaderField("Content-Type", "application/macaroon-request");
        request.setRequestBody(macaroon_request_content(validity, v_activities));
    }

    token_key = (is_oauth) ? "access_token" : "macaroon";
}

std::string MacaroonRetriever::perform_request(HttpRequest& request, std::string description)
{
    char buffer[MacaroonRetriever::RESPONSE_MAX_SIZE];
    DavixError* err = NULL;

    description = (is_oauth) ? "Token" : "Macaroon";

    if (request.beginRequest(&err)) {
        std::stringstream errmsg;
        errmsg << description << " request failed: " << err->getErrMsg();
        throw Gfal::CoreException(http_plugin_domain, davix2errno(err->getStatus()), errmsg.str());
    }

    dav_ssize_t response_size = request.getAnswerSize();

    if (response_size >= MacaroonRetriever::RESPONSE_MAX_SIZE) {
        std::stringstream errmsg;
        errmsg << description << " response exceeds maximum size: " << response_size
               << " bytes (max size = " << MacaroonRetriever::RESPONSE_MAX_SIZE << ")";
        throw Gfal::CoreException(http_plugin_domain, EINVAL, errmsg.str());
    }

    // StoRM has an interesting bug where an unknown/unhandled POST is treated like a corresponding GET,
    // meaning it would respond to the macaroon request with the entire file itself.
    // To protect against this, we read out at most 1MB.
    dav_ssize_t segment_size = request.readSegment(buffer, MacaroonRetriever::RESPONSE_MAX_SIZE, &err);

    if (segment_size < 0) {
        std::stringstream errmsg;
        errmsg << "Reading body of " << description << " request failed: " << err->getErrMsg();
        throw Gfal::CoreException(http_plugin_domain, davix2errno(err->getStatus()), errmsg.str());
    }

    if (segment_size >= MacaroonRetriever::RESPONSE_MAX_SIZE) {
        std::stringstream errmsg;
        errmsg << description << " response exceeds maximum size: " << segment_size
               << " bytes (max size = " << MacaroonRetriever::RESPONSE_MAX_SIZE << ")";
        throw Gfal::CoreException(http_plugin_domain, EINVAL, errmsg.str());
    }

    if (request.getRequestCode() != 200)
    {
        std::stringstream errmsg;
        errmsg << description << " request failed with status code " << request.getRequestCode();
        throw Gfal::CoreException(http_plugin_domain, davix2errno(err->getStatus()), errmsg.str());
    }

    return std::string(buffer);
}

std::string MacaroonRetriever::macaroon_request_content(unsigned validity,
                                                        const std::vector<std::string>& activities)

{
    std::stringstream content;
    content << "{\"caveats\": [\"activity:";

    std::vector<std::string>::const_iterator it;
    for (it = activities.begin(); it != activities.end(); it++) {
        if (it != activities.begin()) {
            content << ",";
        }
        content << (*it);
    }

    content << "\"], \"validity\": \"PT" << validity << "M\"}";
    return content.str();
}

std::string MacaroonRetriever::oauth_request_content(const std::string& path, unsigned validity,
                                                     const std::vector<std::string>& activities)
{
    std::stringstream scopes;

    std::vector<std::string>::const_iterator it;
    for (it = activities.begin(); it != activities.end(); it++) {
        if (it != activities.begin()) {
            scopes << " ";
        }
        scopes << (*it) << ":" << path;
    }

    std::stringstream content;
    content << "grant_type=client_credentials&expire_in=" << (validity * 60);
    content << "&scopes=" << Uri::queryParamEscape(scopes.str());

    return content.str();
}

std::vector<std::string> MacaroonRetriever::_activities(bool write_access, const char* const* activities)
{
    std::vector<std::string> v_activities;

    // User-provided activities
    if (activities && activities[0]) {
        for (int idx = 0; activities[idx]; idx++) {
            v_activities.emplace_back(activities[idx]);
        }

        return v_activities;
    }

    // Construct activities based on read/write flag
    v_activities.emplace_back("LIST");
    v_activities.emplace_back("DOWNLOAD");

    if (write_access) {
        v_activities.emplace_back("MANAGE");
        v_activities.emplace_back("UPLOAD");
        v_activities.emplace_back("DELETE");
    }

    return v_activities;
}

// --------------------------------------------------------
// SciTokens-specific Retrieval implementation
// --------------------------------------------------------

SciTokensRetriever::SciTokensRetriever(std::string issuer):
    TokenRetriever("SciTokens", std::move(issuer))
{
}

bool SciTokensRetriever::validate_endpoint(std::string& endpoint, const Davix::Uri& uri)
{
    return !endpoint.empty();
}

void SciTokensRetriever::prepare_request(HttpRequest& request, const std::string& path,
                                         bool write_access, unsigned validity, const char* const* activities)
{
    request.addHeaderField("Accept", "application/json");
    request.addHeaderField("Content-Type", "application/x-www-form-urlencoded");
    request.setRequestBody("grant_type=client_credentials");
}
