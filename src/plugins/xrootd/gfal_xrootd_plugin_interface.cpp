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

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <sys/stat.h>

// This header provides all the required functions except chmod
#include <XrdPosix/XrdPosixXrootd.hh>

// This header is required for chmod
#include <XrdCl/XrdClFileSystem.hh>

// For directory listing
#include <XrdCl/XrdClFileSystem.hh>
#include <XrdCl/XrdClXRootDResponses.hh>

// For setting the log level
#include <XrdCl/XrdClDefaultEnv.hh>

#include <XrdVersion.hh>

// TRUE and FALSE are defined in Glib and xrootd headers
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif

#include <gfal_plugins_api.h>
#include "gfal_xrootd_plugin_interface.h"
#include "gfal_xrootd_plugin_utils.h"

void set_xrootd_log_level()
{
    // Note: xrootd lib logs to stderr
    if (gfal2_log_get_level() & G_LOG_LEVEL_DEBUG)
      XrdCl::DefaultEnv::SetLogLevel( "Debug" );
    else if (gfal2_log_get_level() & G_LOG_LEVEL_INFO)
      XrdCl::DefaultEnv::SetLogLevel( "Info" );
    else if (gfal2_log_get_level() & G_LOG_LEVEL_MESSAGE)
      XrdCl::DefaultEnv::SetLogLevel( "Info" );
    else if (gfal2_log_get_level() & G_LOG_LEVEL_WARNING)
      XrdCl::DefaultEnv::SetLogLevel( "Warning" );
    else
      XrdCl::DefaultEnv::SetLogLevel( "Error" );
}


int gfal_xrootd_statG(plugin_handle handle, const char* path, struct stat* buff,
        GError ** err)
{
    std::string sanitizedUrl = prepare_url((gfal2_context_t) handle, path);

    // reset stat fields
    reset_stat(*buff);

    if (XrdPosixXrootd::Stat(sanitizedUrl.c_str(), buff) != 0) {
        gfal2_xrootd_set_error(err, errno, __func__, "Failed to stat file");
        return -1;
    }
    return 0;
}


gfal_file_handle gfal_xrootd_openG(plugin_handle handle, const char *path,
        int flag, mode_t mode, GError ** err)
{
    std::string sanitizedUrl = prepare_url((gfal2_context_t) handle, path);

    int *fd = new int;
    *fd = XrdPosixXrootd::Open(sanitizedUrl.c_str(), flag, mode);
    if (*fd == -1) {
        gfal2_xrootd_set_error(err, errno, __func__, "Failed to open file");
        delete fd;
        return NULL;
    }
    return gfal_file_handle_new(gfal_xrootd_getName(), (gpointer) fd);
}


ssize_t gfal_xrootd_readG(plugin_handle handle, gfal_file_handle fd, void *buff,
        size_t count, GError ** err)
{
    int * fdesc = (int*) (gfal_file_handle_get_fdesc(fd));
    if (!fdesc) {
        gfal2_xrootd_set_error(err, errno, __func__, "Bad file handle");
        return -1;
    }
    ssize_t l = XrdPosixXrootd::Read(*fdesc, buff, count);
    if (l < 0) {
        gfal2_xrootd_set_error(err, errno, __func__, "Failed while reading from file");
        return -1;
    }
    return l;
}


ssize_t gfal_xrootd_writeG(plugin_handle handle, gfal_file_handle fd,
        const void *buff, size_t count, GError ** err)
{
    int * fdesc = (int*) (gfal_file_handle_get_fdesc(fd));
    if (!fdesc) {
        gfal2_xrootd_set_error(err, errno, __func__, "Bad file handle");
        return -1;
    }
    ssize_t l = XrdPosixXrootd::Write(*fdesc, buff, count);
    if (l < 0) {
        gfal2_xrootd_set_error(err, errno, __func__, "Failed while writing to file");
        return -1;
    }
    return l;
}


off_t gfal_xrootd_lseekG(plugin_handle handle, gfal_file_handle fd,
        off_t offset, int whence, GError **err)
{
    int * fdesc = (int*) (gfal_file_handle_get_fdesc(fd));
    if (!fdesc) {
        gfal2_xrootd_set_error(err, errno, __func__, "Bad file handle");
        return -1;
    }
    off_t l = XrdPosixXrootd::Lseek(*fdesc, offset, whence);
    if (l < 0) {
        gfal2_xrootd_set_error(err, errno, __func__, "Failed to seek within file");
        return -1;
    }
    return l;
}


int gfal_xrootd_closeG(plugin_handle handle, gfal_file_handle fd, GError ** err)
{
    int r = 0;
    int * fdesc = (int*) (gfal_file_handle_get_fdesc(fd));
    if (fdesc) {
        r = XrdPosixXrootd::Close(*fdesc);
        if (r != 0) {
            gfal2_xrootd_set_error(err, errno, __func__, "Failed to close file");
        }
        delete (int*) (gfal_file_handle_get_fdesc(fd));
    }
    gfal_file_handle_delete(fd);
    return r;
}


int gfal_xrootd_mkdirpG(plugin_handle handle, const char *url, mode_t mode,
        gboolean pflag, GError **err)
{
    std::string sanitizedUrl = prepare_url((gfal2_context_t) handle, url);

    // EOS returns success for mkdir for a directory that exists
    struct stat buf;
    if (XrdPosixXrootd::Stat(sanitizedUrl.c_str(), &buf) == 0) {
        errno = EEXIST;
        gfal2_xrootd_set_error(err, errno, __func__, "Failed to create directory %s", url);
        return -1;
    }

    if (XrdPosixXrootd::Mkdir(sanitizedUrl.c_str(), mode) != 0) {
        if (errno == ECANCELED) {
            errno = EEXIST;
        }
        gfal2_xrootd_set_error(err, errno, __func__, "Failed to create directory %s", url);
        return -1;
    }
    return 0;
}


int gfal_xrootd_chmodG(plugin_handle handle, const char *url, mode_t mode,
        GError **err)
{
    std::string sanitizedUrl = prepare_url((gfal2_context_t) handle, url);
    set_xrootd_log_level();

    XrdCl::URL xrdcl_url( sanitizedUrl );
    XrdCl::FileSystem fs( xrdcl_url );
    XrdCl::Access::Mode xrdcl_mode = file_mode_to_xrdcl_access( mode );
    XrdCl::XRootDStatus status = fs.ChMod( xrdcl_url.GetPath(), xrdcl_mode );

    if (!status.IsOK()) {
      gfal2_xrootd_set_error(err, xrootd_status_to_posix_errno(status),
                             __func__, status.ToStr().c_str());
      return -1;
    }

    return 0;
}


int gfal_xrootd_unlinkG(plugin_handle handle, const char *url,
        GError **err)
{
    std::string sanitizedUrl = prepare_url((gfal2_context_t) handle, url);

    if (XrdPosixXrootd::Unlink(sanitizedUrl.c_str()) != 0) {
        gfal2_xrootd_set_error(err, errno, __func__, "Failed to delete file");
        return -1;
    }
    return 0;
}


int gfal_xrootd_rmdirG(plugin_handle handle, const char *url, GError **err)
{
    std::string sanitizedUrl = prepare_url((gfal2_context_t) handle, url);

    if (XrdPosixXrootd::Rmdir(sanitizedUrl.c_str()) != 0) {
        struct stat buf;
        // Need some errno massaging because of EOS
        if (errno == EEXIST) {
            errno =  ENOTEMPTY;
        }
        else if (errno == EIO) {
            if (XrdPosixXrootd::Stat(sanitizedUrl.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode)) {
                errno = ENOTEMPTY;
            }
            else {
                errno = ENOTDIR;
            }
        }
        else if (errno == ENOENT) {
            if (XrdPosixXrootd::Stat(sanitizedUrl.c_str(), &buf) == 0) {
                errno = ENOTDIR;
            }
        }
        gfal2_xrootd_set_error(err, errno, __func__, "Failed to delete directory");
        return -1;
    }
    return 0;
}


int gfal_xrootd_accessG(plugin_handle handle, const char *url, int mode,
        GError **err)
{
    std::string sanitizedUrl = prepare_url((gfal2_context_t) handle, url);

    if (XrdPosixXrootd::Access(sanitizedUrl.c_str(), mode) != 0) {
        gfal2_xrootd_set_error(err, errno, __func__, "Failed to access file or directory");
        return -1;
    }
    return 0;
}


int gfal_xrootd_renameG(plugin_handle handle, const char *oldurl,
        const char *urlnew, GError **err)
{
    std::string oldSanitizedUrl = prepare_url((gfal2_context_t) handle, oldurl);
    std::string newSanitizedUrl = prepare_url((gfal2_context_t) handle, urlnew);

    if (XrdPosixXrootd::Rename(oldSanitizedUrl.c_str(), newSanitizedUrl.c_str()) != 0) {
        gfal2_xrootd_set_error(err, errno, __func__, "Failed to rename file or directory");
        // EOS may return EEXIST when new is a directory
        if (*err && (*err)->code == EEXIST) {
            struct stat buf;
            if (XrdPosixXrootd::Stat(newSanitizedUrl.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode)) {
                (*err)->code = EISDIR;
            }
        }
        return -1;
    }
    return 0;
}

void StatInfo2Xattr(const XrdCl::StatInfo* stinfo,  char * buff)
{

    bool ontape = false;
    bool ondisk = false;
    if (stinfo->TestFlags(XrdCl::StatInfo::BackUpExists))
        ontape = true;
    if (!stinfo->TestFlags(XrdCl::StatInfo::Offline))
        ondisk= true;

    if (ontape && ondisk) {
        strcpy(buff,GFAL_XATTR_STATUS_NEARLINE_ONLINE);
        gfal2_log(G_LOG_LEVEL_DEBUG, GFAL_XATTR_STATUS_NEARLINE_ONLINE);
    }
    else if (ontape) {
        strcpy(buff,GFAL_XATTR_STATUS_NEARLINE);
        gfal2_log(G_LOG_LEVEL_DEBUG, GFAL_XATTR_STATUS_NEARLINE);
    }
    else if (ondisk) {
        strcpy(buff,GFAL_XATTR_STATUS_ONLINE);
        gfal2_log(G_LOG_LEVEL_DEBUG, GFAL_XATTR_STATUS_ONLINE);
    }
    else {
        strcpy(buff,GFAL_XATTR_STATUS_UNKNOWN);
        gfal2_log(G_LOG_LEVEL_DEBUG, GFAL_XATTR_STATUS_UNKNOWN);
    }
}

// Callback class for directory listing
class DirListHandler: public XrdCl::ResponseHandler
{
private:
    XrdCl::URL url;
    XrdCl::FileSystem fs;
    std::list<XrdCl::DirectoryList::ListEntry*> entries;

    struct dirent dbuffer;

    std::mutex mutex;
    std::condition_variable cv;
    bool done;

public:
    int errcode;
    std::string errstr;

    DirListHandler(const XrdCl::URL& url): url(url), fs(url), done(false), errcode(0)
    {
        memset(&dbuffer, 0, sizeof(dbuffer));
    }

    int List()
    {
        XrdCl::XRootDStatus status = fs.DirList(url.GetPath(), XrdCl::DirListFlags::Stat, this);
        if (!status.IsOK()) {
            errcode = status.code;
            errstr = status.ToString();
            return -1;
        }
        return 0;
    }

    // AFAIK, this is called only once
    void HandleResponse(XrdCl::XRootDStatus* status, XrdCl::AnyObject* response)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (status->IsOK()) {
            XrdCl::DirectoryList* list;
            response->Get<XrdCl::DirectoryList*>(list);
            if (list) {
                XrdCl::DirectoryList::ConstIterator i;
                for (i = list->Begin(); i != list->End(); ++i) {
                    entries.push_back(*i);
                }
            }
        }
        else {
            errcode = status->code;
            errstr = status->ToString();
        }
        done = true;
        cv.notify_all();
    }

    void StatInfo2Stat(const XrdCl::StatInfo* stinfo, struct stat* st)
    {
        st->st_size = stinfo->GetSize();
        st->st_mtime = stinfo->GetModTime();
        st->st_mode = 0;
        if (stinfo->TestFlags(XrdCl::StatInfo::IsDir))
            st->st_mode |= S_IFDIR;
        if (stinfo->TestFlags(XrdCl::StatInfo::IsReadable))
            st->st_mode |= (S_IRUSR | S_IRGRP | S_IROTH);
        if (stinfo->TestFlags(XrdCl::StatInfo::IsWritable))
            st->st_mode |= (S_IWUSR | S_IWGRP | S_IWOTH);
        if (stinfo->TestFlags(XrdCl::StatInfo::XBitSet))
            st->st_mode |= (S_IXUSR | S_IXGRP | S_IXOTH);
    }

    struct dirent* Get(struct stat* st = NULL)
    {
        if (!done) {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait_for(lock, std::chrono::seconds(60));
            if (!done) {
                return NULL;
            }
        }

        if (entries.empty())
            return NULL;

        XrdCl::DirectoryList::ListEntry* entry = entries.front();
        entries.pop_front();

        XrdCl::StatInfo* stinfo = entry->GetStatInfo();

        g_strlcpy(dbuffer.d_name, entry->GetName().c_str(), sizeof(dbuffer.d_name));
        dbuffer.d_reclen = strnlen(dbuffer.d_name, sizeof(dbuffer.d_reclen));

        if (stinfo && stinfo->TestFlags(XrdCl::StatInfo::IsDir))
            dbuffer.d_type = DT_DIR;
        else
            dbuffer.d_type = DT_REG;

        if (st != NULL) {
            if (stinfo != NULL) {
                StatInfo2Stat(stinfo, st);
            }
            else {

                stinfo = new XrdCl::StatInfo();
                std::string fullPath = url.GetPath() + "/" + dbuffer.d_name;
                XrdCl::XRootDStatus status = this->fs.Stat(fullPath, stinfo);
                if (!status.IsOK()) {
                    errcode = status.code;
                    errstr = status.ToString();
                    return NULL;
                }
                StatInfo2Stat(stinfo, st);
                delete stinfo;
            }
        }

        delete entry;
        return &dbuffer;
    }
};


gfal_file_handle gfal_xrootd_opendirG(plugin_handle handle,
        const char* url, GError** err)
{
    std::string sanitizedUrl = prepare_url((gfal2_context_t) handle, url);
    XrdCl::URL parsed(sanitizedUrl);

    // Need to do stat first so we can fail syncrhonously for some errors!
    struct stat st;
    if (XrdPosixXrootd::Stat(sanitizedUrl.c_str(), &st) != 0) {
        gfal2_xrootd_set_error(err, errno, __func__, "Failed to stat file");
        return NULL;
    }

    if (!S_ISDIR(st.st_mode)) {
        gfal2_xrootd_set_error(err, ENOTDIR, __func__, "Not a directory");
        return NULL;
    }

    DirListHandler* handler = new DirListHandler(parsed);

    if (handler->List() != 0) {
        gfal2_xrootd_set_error(err, handler->errcode, __func__, "Failed to open dir: %s",
                handler->errstr.c_str());
        return NULL;
    }

    return gfal_file_handle_new2(gfal_xrootd_getName(), (gpointer) handler, NULL, url);
}


struct dirent* gfal_xrootd_readdirG(plugin_handle plugin_data,
        gfal_file_handle dir_desc, GError** err)
{
    DirListHandler* handler = (DirListHandler*)(gfal_file_handle_get_fdesc(dir_desc));
    if (!handler) {
        gfal2_xrootd_set_error(err, errno, __func__, "Bad dir handle");
        return NULL;
    }
    dirent* entry = handler->Get();
    if (!entry && handler->errcode != 0) {
        gfal2_xrootd_set_error(err, handler->errcode, __func__, "Failed reading directory: %s",
                handler->errstr.c_str());
        return NULL;
    }
    return entry;
}


struct dirent* gfal_xrootd_readdirppG(plugin_handle plugin_data,
        gfal_file_handle dir_desc, struct stat* st, GError** err)
{
    DirListHandler* handler = (DirListHandler*)(gfal_file_handle_get_fdesc(dir_desc));
    if (!handler) {
        gfal2_xrootd_set_error(err, errno, __func__, "Bad dir handle");
        return NULL;
    }
    dirent* entry = handler->Get(st);
    if (!entry && handler->errcode != 0) {
        gfal2_xrootd_set_error(err, handler->errcode, __func__, "Failed reading directory: %s",
                handler->errstr.c_str());
        return NULL;
    }
    return entry;
}


int gfal_xrootd_closedirG(plugin_handle plugin_data, gfal_file_handle dir_desc,
        GError** err)
{
    // Free all objects associated with this client
    DirListHandler* handler = (DirListHandler*)(gfal_file_handle_get_fdesc(dir_desc));
    if (handler) {
        delete handler;
    }
    gfal_file_handle_delete(dir_desc);
    return 0;
}


int gfal_xrootd_checksumG(plugin_handle plugin_data, const char* url,
        const char* check_type, char * checksum_buffer, size_t buffer_length,
        off_t start_offset, size_t data_length, GError ** err)
{

    std::string sanitizedUrl = prepare_url((gfal2_context_t) plugin_data, url);
    std::string lowerChecksumType = predefined_checksum_type_to_lower(check_type);

    if (start_offset != 0 || data_length != 0) {
        gfal2_xrootd_set_error(err, ENOTSUP, __func__, "XROOTD does not support partial checksums");
        return -1;
    }

    if (sanitizedUrl.find("?") == std::string::npos) {
        sanitizedUrl += "?";
    } else {
        sanitizedUrl += "&";
    }
    sanitizedUrl += "cks.type=";
    sanitizedUrl += lowerChecksumType;

    time_t mTime;
    if (XrdPosixXrootd::QueryChksum(sanitizedUrl.c_str(), mTime, checksum_buffer, buffer_length) < 0) {
        gfal2_xrootd_set_error(err, errno, __func__, "Could not get the checksum");
        return -1;
    }

    // Note that the returned value is "type value"
    char* space = ::index(checksum_buffer, ' ');
    if (!space) {
        gfal2_xrootd_set_error(err, errno, __func__, "Could not get the checksum (Wrong format)");
        return -1;
    }
    *space = '\0';

    if (strncasecmp(checksum_buffer, lowerChecksumType.c_str(),
            lowerChecksumType.length()) != 0) {
        gfal2_xrootd_set_error(err, errno, __func__, "Got '%s' while expecting '%s'",
                checksum_buffer, lowerChecksumType.c_str());
        return -1;
    }

    g_strlcpy(checksum_buffer, space + 1, buffer_length);

    return 0;
}


ssize_t gfal_xrootd_getxattrG(plugin_handle plugin_data, const char* url, const char* key,
                            void* buff, size_t s_buff, GError** err)
{
    ssize_t len = 0;

    if (strcmp(key, GFAL_XATTR_SPACETOKEN) == 0) {
        len = gfal_xrootd_space_reporting(plugin_data, url, key, buff, s_buff, err);
    } else if (strcmp(key, GFAL_XATTR_STATUS) == 0) {

        std::string sanitizedUrl = prepare_url((gfal2_context_t) plugin_data, url);
        XrdCl::URL parsed(sanitizedUrl);
        XrdCl::FileSystem fs(parsed);
        XrdCl::StatInfo * info;
        XrdCl::XRootDStatus st = fs.Stat( parsed.GetPath(), info );

        if (!st.IsOK()) {
            errno = ENOENT;
            gfal2_xrootd_set_error(err, errno, __func__, "Failed to get the xattr \"%s\"", key);
            return -1;
        }

        StatInfo2Xattr(info,(char*)buff);
        len = strnlen((char*)buff, s_buff);
        delete info;
    } else {
        std::string sanitizedUrl = prepare_url((gfal2_context_t) plugin_data, url);
        memset(buff, 0x00, s_buff);
        len = XrdPosixXrootd::Getxattr(sanitizedUrl.c_str(), key, buff, s_buff);
        if (len < 0) {
            gfal2_xrootd_set_error(err, errno, __func__, "Failed to get the xattr \"%s\"", key);
        }
    }
    return len;
}


ssize_t gfal_xrootd_listxattrG(plugin_handle plugin_data, const char* url,
        char* list, size_t s_list, GError** err)
{
    static const char props[] = "xroot.cksum\0xroot.space\0xroot.xattr\0spacetoken";
    static const size_t proplen = sizeof(props);
    size_t len = proplen > s_list ? s_list : proplen;
    memcpy(list, props, len);
    return len;
}


int gfal_xrootd_setxattrG(plugin_handle plugin_data, const char* url, const char* key,
                    const void* buff , size_t s_buff, int flags, GError** err)
{
    gfal2_xrootd_set_error(err, ENOSYS, __func__, "Can not set extended attributes");
    return -1;
}


const char* gfal_xrootd_getName()
{
    return GFAL2_PLUGIN_VERSIONED("xrootd", VERSION);
}
