#ifndef _GFAL_HTTP_PLUGIN_H
#define _GFAL_HTTP_PLUGIN_H

#include "../gfal_common_errverbose.h"
#include "../fdesc/gfal_file_handle.h"
#include "../gfal_common_internal.h"
#include "../gfal_common_plugin.h"
#include "../gfal_types.h"


#include <davix.hpp>


struct GfalHttpInternal {
    Davix::Context*       context;
    Davix::DavPosix*      posix;
    Davix::RequestParams* params;
};

extern const char* http_module_name;
extern GQuark http_plugin_domain;

// Initializes a GError from a DavixError
void davix2gliberr(const Davix::DavixError* daverr, GError** err);

// Converts a cert/key pair (X509) into a P12
int convert_x509_to_p12(const char *privkey, const char *clicert, const char *p12cert, davix_error_t*);


// METADATA OPERATIONS
void gfal_http_delete(plugin_handle plugin_data);

int gfal_http_stat(plugin_handle plugin_data, const char* url, struct stat* buf, GError** err);

int gfal_http_access(plugin_handle plugin_data, const char* url, int mode, GError** err);

int gfal_http_mkdirpG(plugin_handle plugin_data, const char* url, mode_t mode, gboolean rec_flag, GError** err);

int gfal_http_rmdirG(plugin_handle plugin_data, const char* url, GError** err);

int gfal_http_unlinkG(plugin_handle plugin_data, const char* url, GError** err);

gfal_file_handle gfal_http_opendir(plugin_handle plugin_data, const char* url, GError** err);

struct dirent* gfal_http_readdir(plugin_handle plugin_data, gfal_file_handle dir_desc, GError** err);

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

// 3RD PARTY COPY
int gfal_http_3rdcopy(plugin_handle plugin_data, gfal2_context_t context, gfalt_params_t params,
                      const char* src, const char* dst, GError** err);

int gfal_http_3rdcopy_check(plugin_handle plugin_data,  const char* src, const char* dst, gfal_url2_check check);

#endif //_GFAL_HTTP_PLUGIN_H
