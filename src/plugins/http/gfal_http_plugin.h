#ifndef _GFAL_HTTP_PLUGIN_H
#define _GFAL_HTTP_PLUGIN_H

#include <fdesc/gfal_file_handle.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_types.h>


#include <davix.hpp>


struct GfalHttpInternal {
    GfalHttpInternal(gfal_handle handle);

    Davix::Context       context;
    Davix::DavPosix      posix;
    Davix::RequestParams params;
};


struct GfalHttpPluginData{
    GfalHttpPluginData(gfal_handle);
    ~GfalHttpPluginData();

    GfalHttpInternal* davix;
    GMutex* _init_mux;
    gfal_handle handle;
};


GfalHttpInternal* gfal_http_get_plugin_context(gpointer plugin_data);

void gfal_http_context_delete(gpointer plugin_data);

extern const char* http_module_name;
extern GQuark http_plugin_domain;

// Initializes a GError from a DavixError
void davix2gliberr(const Davix::DavixError* daverr, GError** err);

// X509 callback
void gfal_http_get_ucert(std::string& ucert, std::string& ukey);
int gfal_http_authn_cert_X509(void* userdata, const Davix::SessionInfo & info, Davix::X509Credential * cert, Davix::DavixError** err);

// METADATA OPERATIONS
void gfal_http_delete(plugin_handle plugin_data);

int gfal_http_stat(plugin_handle plugin_data, const char* url, struct stat* buf, GError** err);

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


int gfal_http_copy(plugin_handle plugin_data, gfal2_context_t context, gfalt_params_t params,
        const char* src, const char* dst, GError** err);

int gfal_http_copy_check(plugin_handle plugin_data, gfal_context_t context,
        const char* src, const char* dst, gfal_url2_check check);

#endif //_GFAL_HTTP_PLUGIN_H
