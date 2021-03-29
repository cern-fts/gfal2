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

#ifndef _GFAL_HTTP_PLUGIN_TOKEN_H
#define _GFAL_HTTP_PLUGIN_TOKEN_H

#include <davix.hpp>

#include "gfal_http_plugin.h"

/**
 * Gfal2 HTTP plugin abstraction for a bearer token.
 * Holds the token itself, validity and write access.
 *
 * The tokens are associated to a resource (URL) in the token internal map.
 * When the same resource is requested again, given the validity
 * and permissions match, the token retrieved from the map can be reused.
 */
struct gfal_http_token_s {
    std::string token;
    unsigned validity;
    bool write_access;
};

typedef struct gfal_http_token_s gfal_http_token_t;

class TokenRetriever {
public:
    TokenRetriever(std::string label, std::string issuer);

    virtual ~TokenRetriever() = default;

    TokenRetriever* add(TokenRetriever* elem);

    TokenRetriever* next();

    /**
     * Performs necessary steps to retrieve a SE-issued token for a given URL.
     * Token-specific steps are defined by sub-classes.
     *
     * @param _url path to the resource
     * @param _params Davix request parameters
     * @param write_access token needs write permissions
     * @params validity token validity expressed in minutes
     * @return Gfal2 HTTP token structure
     */
    gfal_http_token_t retrieve_token(const Davix::Uri& _url,
                                     const Davix::RequestParams& _params,
                                     bool write_access,
                                     unsigned validity);



    /// Short class description
    const std::string label;
    /// Token issuer endpoint
    const std::string issuer;

protected:
    /**
     * Template method to allow validating the token issuer endpoint.
     * @param endpoint the token issuer endpoint
     * @params uri the resource URL
     * @return true if valid endpoint, false otherwise
     */
    virtual bool validate_endpoint(std::string& endpoint, const Davix::Uri& uri) = 0;

    /**
     * Template method to prepare the token request.
     * @param request Davix HTTP request
     * @param path the resource to access
     * @param write_access flag to signal write access needed
     * @params validity token validity expressed in minutes
     */
    virtual void prepare_request(Davix::HttpRequest& request, const std::string& path,
                                 bool write_access, unsigned validity) = 0;

    /**
     * Perform an HTTP request.
     * @param request Davix Request object
     * @param description the request description
     * @return the response body
     */
    virtual std::string perform_request(Davix::HttpRequest& request, std::string description = "");

    /**
     * Parse the JSON response and extract the given key.
     * @param response the server response
     * @param key the key to extract
     * @return extracted key value
     */
    std::string parse_json_response(const std::string& response, const std::string& key);

    /// Separate Davix context object
    Davix::Context context;
    /// Endpoint discovery fallback flag
    bool discovery_fallback;
    /// Key used to extract the token from JSON response
    std::string token_key;
    /// Pointer to allow chaining token retrievers
    std::unique_ptr<TokenRetriever> _next;

private:
    /**
     * Checks that the protocol allows token retrieval (https:// or davs://).
     * @param _url input URL
     * @return HTTPS schema URL
     */
    Davix::Uri format_protocol(const Davix::Uri& _url);

    /**
     * Identify the token issuer endpoint.
     * @param params Davix request parameters
     * @return the token issuer endpoint
     */
    std::string get_token_endpoint(Davix::RequestParams& params);

    // Convenience function to build OAuth authorization endpoint
    std::string _metadata_endpoint(const Davix::Uri& url);

    // Query the metadata URL for the token issuer endpoint
    std::string _endpoint_discovery(const std::string& metadata_url, Davix::RequestParams& params);
};

class MacaroonRetriever: public TokenRetriever {
public:
    MacaroonRetriever();
    MacaroonRetriever(std::string issuer);

    /// Maximum size in bytes accepted for token response
    static constexpr dav_ssize_t RESPONSE_MAX_SIZE {1024 * 1024};

protected:
    /**
     * Returns true in all cases.
     * For an empty endpoint, query the resource host directly.
     */
    bool validate_endpoint(std::string& endpoint, const Davix::Uri& uri) override;

    /**
     * Macaroon specific request headers and content.
     * @param request Davix HTTP request
     * @param path the resource to access
     * @param write_access flag to signal write access needed
     * @param validity token validity expressed in minutes
     */
    void prepare_request(Davix::HttpRequest& request, const std::string& path,
                         bool write_access, unsigned validity) override;

    /**
     * Override for the base class perform HTTP request.
     * For macaroons, a special request is needed to accommodate a bug encountered in STORM.
     * @param request Davix Request object
     * @param description the request description
     * @return the response body
     */
    std::string perform_request(Davix::HttpRequest& request, std::string description = "") override;


private:
    // Create macaroon request content for the given write access and validity
    std::string macaroon_request_content(bool write_access, unsigned validity);

    // Create oauth-type request content for the given path, write access and validity
    std::string oauth_request_content(const std::string& path, bool write_access,
                                      unsigned validity);

    // Utility function to generate activities needed for read/write access
    std::vector<std::string> _activities(bool write_access);

    /// OAuth macaroon issuer
    bool is_oauth;
};

class SciTokensRetriever: public TokenRetriever {
public:
    SciTokensRetriever(std::string issuer);

protected:
    /**
     * Returns true if endpoint is not empty.
     */
    bool validate_endpoint(std::string& endpoint, const Davix::Uri& uri) override;

    /**
     * SciTokens specific request headers and content.
     * @param request Davix HTTP request
     * @param path the resource to access
     * @param write_access flag to signal write access needed
     * @param validity token validity expressed in minutes
     */
    void prepare_request(Davix::HttpRequest& request, const std::string& path,
                         bool write_access, unsigned validity) override;
};

#endif //_GFAL_HTTP_PLUGIN_TOKEN_H
