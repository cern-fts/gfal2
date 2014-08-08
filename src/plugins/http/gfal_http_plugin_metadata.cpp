#include <errno.h>
#include <glib.h>
#include <unistd.h>
#include <common/gfal_common_err_helpers.h>
#include "gfal_http_plugin.h"



int gfal_http_stat(plugin_handle plugin_data, const char* url,
                   struct stat* buf, GError** err)
{
    Davix::StatInfo info;
    char stripped_url[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(url, stripped_url, sizeof(stripped_url));
    if(buf==NULL){
        gfal2_set_error(err, http_plugin_domain, EINVAL, __func__, "Invalid stat argument");
        return -1;
    }

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    if (davix->posix.stat64(&davix->params, stripped_url, &info, &daverr) != 0) {
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
        return -1;
    }
    info.toPosixStat(*buf);
    return 0;
}



int gfal_http_mkdirpG(plugin_handle plugin_data, const char* url, mode_t mode, gboolean rec_flag, GError** err)
{
    char stripped_url[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(url, stripped_url, sizeof(stripped_url));

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    if (davix->posix.mkdir(&davix->params, stripped_url, mode, &daverr) != 0) {
        davix2gliberr(daverr, err);
        // Beware!
        // Davix maps 405 to PermissionRefused, even though it is FileExist for MKCOL
        // And maps 409 to FileExists, when it actually is ENOENT for MKCOL
        // Have a look here http://www.webdav.org/specs/rfc4918.html#rfc.section.9.3.1
        if (daverr->getStatus() == Davix::StatusCode::PermissionRefused)
            (*err)->code = EEXIST;
        if (daverr->getStatus() == Davix::StatusCode::FileExist)
            (*err)->code = ENOENT;
        Davix::DavixError::clearError(&daverr);
        return -1;
    }
    return 0;
}



int gfal_http_unlinkG(plugin_handle plugin_data, const char* url, GError** err)
{
    char stripped_url[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(url, stripped_url, sizeof(stripped_url));

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;

    // Note: in WebDAV DELETE works for directories and files, so check ourselves
    // we are not unlinking a folder
    struct stat st;
    if (davix->posix.stat(&davix->params, stripped_url, &st, &daverr) != 0) {
      davix2gliberr(daverr, err);
      Davix::DavixError::clearError(&daverr);
      return -1;
    }

    if (S_ISDIR(st.st_mode)) {
        gfal2_set_error(err, http_plugin_domain, EISDIR, __func__,
                  "Can not unlink a directory");
        return -1;
    }

    if (davix->posix.unlink(&davix->params, stripped_url, &daverr) != 0) {
      davix2gliberr(daverr, err);
      Davix::DavixError::clearError(&daverr);
      return -1;
    }
    return 0;
}



int gfal_http_rmdirG(plugin_handle plugin_data, const char* url, GError** err)
{
    char stripped_url[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(url, stripped_url, sizeof(stripped_url));

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;

    // Note: in WebDAV DELETE is recursive for directories, so check first if there is anything
    // inside and fail in that case
    struct stat st;
    if (davix->posix.stat(&davix->params, stripped_url, &st, &daverr) != 0) {
      davix2gliberr(daverr, err);
      Davix::DavixError::clearError(&daverr);
      return -1;
    }

    if (!S_ISDIR(st.st_mode)) {
        gfal2_set_error(err, http_plugin_domain, ENOTDIR, __func__,
                  "Can not rmdir a file");
        return -1;
    }

    if (davix->posix.rmdir(&davix->params, stripped_url, &daverr) != 0) {
      davix2gliberr(daverr, err);
      Davix::DavixError::clearError(&daverr);
      return -1;
    }
    return 0;
}



int gfal_http_access(plugin_handle plugin_data, const char* url, int mode, GError** err)
{
    struct stat buf;
    GError* tmp_err = NULL;

    if (gfal_http_stat(plugin_data, url, &buf, &tmp_err) != 0) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
        return -1;
    }

    uid_t real_uid = getuid();
    gid_t real_gid = getgid();

    int ngroups = getgroups(0, NULL);
    if (ngroups < 0) {
        gfal2_set_error(err, http_plugin_domain, errno, __func__,
                "Could not get the groups of the current user");
        return -1;
    }

    gid_t additional_gids[ngroups];
    getgroups(ngroups, additional_gids);

    if (real_uid == buf.st_uid)
        mode <<= 6;
    else if (real_gid == buf.st_gid)
        mode <<= 3;
    else {
        for (int i = 0; i < ngroups; ++i) {
            if (additional_gids[i] == buf.st_gid) {
                mode <<= 3;
                break;
            }
        }
    }

    if ((mode & buf.st_mode) != static_cast<mode_t>(mode)) {
        gfal2_set_error(err, http_plugin_domain, EACCES, __func__,
                "Does not have enough permissions on '%s'", url);
        return -1;
    }
    else {
        return 0;
    }
}



gfal_file_handle gfal_http_opendir(plugin_handle plugin_data, const char* url,
                                   GError** err)
{
    char stripped_url[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(url, stripped_url, sizeof(stripped_url));

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;

    DAVIX_DIR* dir = davix->posix.opendirpp(&davix->params, stripped_url, &daverr);
    if (dir == NULL) {
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
        return NULL;
    }
    return gfal_file_handle_new2(http_module_name, dir, NULL, url);
}



struct dirent* gfal_http_readdir(plugin_handle plugin_data,
        gfal_file_handle dir_desc, GError** err)
{
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;

    daverr = NULL;
    struct stat _;
    struct dirent* de = davix->posix.readdirpp((DAVIX_DIR*)gfal_file_handle_get_fdesc(dir_desc),
                                             &_, &daverr);
    if (de == NULL && daverr != NULL) {
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
    }
    return de;
}



struct dirent* gfal_http_readdirpp(plugin_handle plugin_data,
        gfal_file_handle dir_desc, struct stat* st, GError** err)
{
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;

    daverr = NULL;
    struct dirent* de = davix->posix.readdirpp((DAVIX_DIR*)gfal_file_handle_get_fdesc(dir_desc),
                                               st, &daverr);
    if (de == NULL && daverr != NULL) {
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
    }
    return de;
}


int gfal_http_closedir(plugin_handle plugin_data, gfal_file_handle dir_desc, GError** err)
{
    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    int ret = 0;

    if (davix->posix.closedir((DAVIX_DIR*) gfal_file_handle_get_fdesc(dir_desc), &daverr) != 0) {
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
        ret = -1;
    }
    gfal_file_handle_delete(dir_desc);
    return ret;
}



int gfal_http_checksum(plugin_handle plugin_data, const char* url, const char* check_type,
                       char * checksum_buffer, size_t buffer_length,
                       off_t start_offset, size_t data_length,
                       GError ** err)
{
    char stripped_url[GFAL_URL_MAX_LEN];
    strip_3rd_from_url(url, stripped_url, sizeof(stripped_url));

    GfalHttpPluginData* davix = gfal_http_get_plugin_context(plugin_data);
    Davix::DavixError* daverr = NULL;
    std::string buffer_chk, algo_chk(check_type);

    if (start_offset != 0 || data_length != 0) {
        gfal2_set_error(err, http_plugin_domain, ENOTSUP, __func__,
                    "HTTP does not support partial checksums");
        return -1;
    }

    Davix::File f(davix->context, Davix::Uri(stripped_url));
    if(f.checksum(&davix->params, buffer_chk, check_type, &daverr) <0 ){
        davix2gliberr(daverr, err);
        Davix::DavixError::clearError(&daverr);
    }

    g_strlcpy(checksum_buffer, buffer_chk.c_str(), buffer_length);
    return 0;
}
