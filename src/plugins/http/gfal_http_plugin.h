/*
 * Copyright (c) CERN 2013-2017
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

#ifndef _GFAL_HTTP_PLUGIN_H
#define _GFAL_HTTP_PLUGIN_H

#include <map>

#include <gfal_plugins_api.h>
#include <davix.hpp>

#include "gfal_http_plugin_token.h"

#define HTTP_CONFIG_OP_TIMEOUT     "OPERATION_TIMEOUT"

class GfalHttpPluginData {
public:
    GfalHttpPluginData(gfal2_context_t);

    Davix::Context  context;
    Davix::DavPosix posix;
    gfal2_context_t handle;

    enum class OP {
        READ,
        HEAD,
        WRITE,
        MKCOL,
        TAPE,
        READ_PASV,
        WRITE_PASV
    };

    // Set up the Davix request parameters for a given URL
    // @param operation the HTTP operation to be performed
    void get_params(Davix::RequestParams*, const Davix::Uri& uri,
                    const OP& operation = OP::READ);

    // Put together parameters for the TPC, which may depend on both URLs in the transfer
    // Further, the request headers depend on the transfer mode that will be used.
    void get_tpc_params(Davix::RequestParams*,
                        const Davix::Uri& src_uri, const Davix::Uri& dst_uri,
                        gfalt_params_t transfer_params,
                        bool push_mode);

    friend ssize_t gfal_http_token_retrieve(plugin_handle plugin_data, const char* url, const char* issuer,
                                            gboolean write_access, unsigned validity, const char* const* activities,
                                            char* buff, size_t s_buff, GError** err);

    friend int gfal_http_mkdirpG(plugin_handle plugin_data, const char *url, mode_t mode,
                                 gboolean rec_flag, GError **err);

    friend int gfal_http_rename(plugin_handle plugin_data, const char* oldurl,
                                const char* newurl, GError** err);;

    friend class TokenMapTest;

    friend std::string gfal_http_discover_tape_endpoint(GfalHttpPluginData* davix, const char* url, const char* method,
                                                        GError** err);

    friend void gfal_http_get_tape_api_version(plugin_handle plugin_data, const char* url, const char *key,
                                               char* buff, size_t s_buff, GError** err);

private:
    typedef std::map<std::string, bool> TokenAccessMap;
    typedef std::map<std::string, std::pair<std::string, std::string>> TapeEndpointMap;
    /// baseline Davix Request Parameters
    Davix::RequestParams reference_params;
    /// map a token with read/write access flag
    TokenAccessMap token_map;
    /// token retriever object (can be chained)
    std::unique_ptr<TokenRetriever> token_retriever_chain;
    /// map a url with a tape endpoint and the corresponding version
    TapeEndpointMap tape_endpoint_map;

    // Set up general request parameters
    void get_params_internal(Davix::RequestParams& params, const Davix::Uri& uri);

    // Obtain credentials for a given Uri and set those credentials in the Davix request parameters.
    // @param operation the HTTP operation to be performed
    // @param token_validity requested lifetime of the token in minutes
    void get_credentials(Davix::RequestParams& params, const Davix::Uri& uri,
                         const OP& operation, unsigned token_validity = 180);

    // Obtain token credentials
    // @param operation the HTTP operation to be performed
    // @param validity requested lifetime of the token in seconds
    // @return true if a bearer for the provided Uri was set in the request params
    bool get_token(Davix::RequestParams& params, const Davix::Uri& uri,
                   const OP& operation, unsigned validity);

    // Find SE-issued token in the Gfal2 credential map based on the path.
    // The found token provides either an exact path match or a parent directory path.
    // When the "extended_search" is requested, the found token may be a subpath of the search path.
    // @param operation the HTTP operation to be performed. Read/write access and extended search is inferred
    // @return the SE-issued token or null
    char* find_se_token(const Davix::Uri& uri, const OP& operation);

    // Attempt to obtain a SE-issued token (by exchanging x509 certificate)
    // @param operation the HTTP operation to be performed. Read/write access is inferred
    // @param validity lifetime of the token in seconds
    // @return the SE-issued token or null
    char* retrieve_and_store_se_token(const Davix::Uri& uri, const OP& operation, unsigned validity);

    // Discover tape endpoint and cache it
    // @param config_endpoint the well-known endpoint of the Tape REST API
    // @param err error handle
    // @return the tape endpoint
    std::string retrieve_and_store_tape_endpoint(const std::string& config_endpoint, GError** err);

    // Obtain request parameters + credentials for an AWS endpoint
    void get_aws_params(Davix::RequestParams& params, const Davix::Uri& uri);

    // Obtain GCloud endpoint credentials
    void get_gcloud_credentials(Davix::RequestParams& params, const Davix::Uri& uri);

    // Obtain Reva endpoint credentials
    void get_reva_credentials(Davix::RequestParams &params, const Davix::Uri &uri, const OP& operation);

    // Obtain certificate credentials
    void get_certificate(Davix::RequestParams& params, const Davix::Uri& uri);

    // Obtain request parameters + credentials for a Swift endpoint
    void get_swift_params(Davix::RequestParams &params, const Davix::Uri &uri);

    bool writeFlagFromOperation(const OP& operation);
    bool searchFlagFromOperation(const OP& operation);
    bool needsTransferHeader(const OP& operation);
};

const char* gfal_http_get_name(void);

GfalHttpPluginData* gfal_http_get_plugin_context(gpointer plugin_data);

void gfal_http_context_delete(gpointer plugin_data);

extern GQuark http_plugin_domain;

// Initializes a GError from a DavixError
void davix2gliberr(const Davix::DavixError* daverr, GError** err, const gchar* function);

// Initializes a GError from an HTTP code
void http2gliberr(GError** err, int http, const char* func, const char* msg);

// Returns errno from Davix StatusCode
int davix2errno(Davix::StatusCode::Code code);

// Returns whether HTTP streaming is enabled for the involved Storage Endpoints
bool is_http_streaming_enabled(gfal2_context_t context, const char* src, const char* dst);

// Removes +3rd from the url, if there
void strip_3rd_from_url(const char* url_full, char* url, size_t url_size);

// Find tape endpoint for a given method
std::string gfal_http_discover_tape_endpoint(GfalHttpPluginData* davix, const char* url, const char* method,
                                             GError** err);

// Get user.status extended attribute
void gfal_http_status_getxattr(plugin_handle plugin_data, const char* url, char* buff, size_t s_buff, GError** err);

// Get tape REST API version
void gfal_http_get_tape_api_version(plugin_handle plugin_data, const char* url, const char *key,
                                    char* buff, size_t s_buff, GError** err);

// METADATA OPERATIONS
void gfal_http_delete(plugin_handle plugin_data);

int gfal_http_stat(plugin_handle plugin_data, const char* url, struct stat* buf, GError** err);

int gfal_http_rename(plugin_handle plugin_data, const char* oldurl, const char* newurl, GError** err);

int gfal_http_access(plugin_handle plugin_data, const char* url, int mode, GError** err);

int gfal_http_mkdirpG(plugin_handle plugin_data, const char* url, mode_t mode, gboolean rec_flag, GError** err);

int gfal_http_rmdirG(plugin_handle plugin_data, const char* url, GError** err);

int gfal_http_unlinkG(plugin_handle plugin_data, const char* url, GError** err);

gfal_file_handle gfal_http_opendir(plugin_handle plugin_data, const char* url, GError** err);

struct dirent* gfal_http_readdir(plugin_handle plugin_data, gfal_file_handle dir_desc, GError** err);

struct dirent* gfal_http_readdirpp(plugin_handle plugin_data, gfal_file_handle dir_desc, struct stat* st, GError** err);

int gfal_http_closedir(plugin_handle plugin_data, gfal_file_handle dir_desc, GError** err);

// IO
gfal_file_handle gfal_http_fopen(plugin_handle plugin_data, const char* url, int flag, mode_t mode, GError** err);

ssize_t gfal_http_fread(plugin_handle, gfal_file_handle fd, void* buff, size_t count, GError** err);

ssize_t gfal_http_fwrite(plugin_handle, gfal_file_handle fd, const void* buff, size_t count, GError** err);

int gfal_http_fclose(plugin_handle, gfal_file_handle fd, GError ** err);

off_t gfal_http_fseek(plugin_handle, gfal_file_handle fd, off_t offset, int whence, GError** err);

// Checksum
int gfal_http_checksum(plugin_handle data, const char* url, const char* check_type,
                       char * checksum_buffer, size_t buffer_length,
                       off_t start_offset, size_t data_length,
                       GError ** err);

// Extended attributes
ssize_t gfal_http_getxattrG(plugin_handle plugin_data, const char* url, const char* key,
                            void* buff, size_t s_buff, GError** err);

ssize_t gfal_http_listxattrG(plugin_handle plugin_data, const char* url,
                             char* list, size_t s_list, GError** err);

int gfal_http_setxattrG(plugin_handle plugin_data, const char* url, const char* key,
                        const void* buff , size_t s_buff, int flags, GError** err);

int gfal_http_copy(plugin_handle plugin_data, gfal2_context_t context, gfalt_params_t params,
        const char* src, const char* dst, GError** err);

int gfal_http_copy_check(plugin_handle plugin_data, gfal2_context_t context,
        const char* src, const char* dst, gfal_url2_check check);

// QoS
ssize_t gfal_http_check_classes(plugin_handle plugin_data, const char* url, const char* type,
                                char* buff, size_t s_buff, GError** err);
ssize_t gfal_http_check_file_qos(plugin_handle plugin_data, const char* url, char* buff, size_t s_buff, GError** err);
ssize_t gfal_http_check_qos_available_transitions(plugin_handle plugin_data, const char* qos_class_url,
                                                  char* buff, size_t s_buff, GError** err);
ssize_t gfal_http_check_target_qos(plugin_handle plugin_data, const char* url, char* buff, size_t s_buff, GError** err);
int gfal_http_change_object_qos(plugin_handle plugin_data, const char* url, const char* target_qos, GError** err);
bool http_cdmi_code_is_valid(int code);

// Token
ssize_t gfal_http_token_retrieve(plugin_handle plugin_data, const char* url, const char* issuer,
                                 gboolean write_access, unsigned validity, const char* const* activities,
                                 char* buff, size_t s_buff, GError** err);

// Tape Operations
int gfal_http_release_file(plugin_handle plugin_data, const char* url,
                           const char* request_id, GError** err);

int gfal_http_release_file_list(plugin_handle plugin_data, int nbfiles, const char* const* urls,
                                const char* request_id, GError** errors);

int gfal_http_archive_poll(plugin_handle plugin_data, const char* url, GError** err);

int gfal_http_archive_poll_list(plugin_handle plugin_data, int nbfiles, const char* const* urls,
                                GError** errors);

int gfal_http_bring_online_poll(plugin_handle plugin_data, const char* url, const char* token, GError** err);

int gfal_http_bring_online_poll_list(plugin_handle plugin_data, int nbfiles, const char* const* urls,
                                     const char* token, GError** errors);

int gfal_http_abort_files(plugin_handle handle, int nbfiles, const char* const* uris, const char* token, GError** errors);

int gfal_http_bring_online(plugin_handle plugin_data, const char* url, time_t pintime, time_t timeout, char* token,
                           size_t tsize, int async, GError** err);

int gfal_http_bring_online_v2(plugin_handle plugin_data, const char* url, const char* metadata, time_t pintime, time_t timeout,
                              char* token, size_t tsize, int async, GError** err);

int gfal_http_bring_online_list(plugin_handle plugin_data, int nbfiles, const char* const* urls, time_t pintime, time_t timeout,
                                char* token, size_t tsize, int async, GError** errors);

int gfal_http_bring_online_list_v2(plugin_handle plugin_data, int nbfiles, const char* const* urls, const char* const* metadata,
                                   time_t pintime, time_t timeout, char* token, size_t tsize, int async, GError** errors);

#endif //_GFAL_HTTP_PLUGIN_H
