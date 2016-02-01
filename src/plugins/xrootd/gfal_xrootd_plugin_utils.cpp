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

#include <algorithm>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sstream>
#include <XProtocol/XProtocol.hh>

#include "gfal_xrootd_plugin_utils.h"

GQuark xrootd_domain = g_quark_from_static_string("xroot");


void file_mode_to_xrootd_ints(mode_t mode, int& user, int& group, int& other)
{
    user = 0;
    group = 0;
    other = 0;

    if (mode & S_IRUSR)
        user += 4;
    if (mode & S_IWUSR)
        user += 2;
    if (mode & S_IXUSR)
        user += 1;

    if (mode & S_IRGRP)
        group += 4;
    if (mode & S_IWGRP)
        group += 2;
    if (mode & S_IXGRP)
        group += 1;

    if (mode & S_IROTH)
        other += 4;
    if (mode & S_IWOTH)
        other += 2;
    if (mode & S_IXOTH) other += 1;
}


void reset_stat(struct stat& st)
{
    st.st_mode = 0;
    st.st_atime = 0;
    st.st_ctime = 0;
    st.st_mtime = 0;
    st.st_blksize = 0;
    st.st_blocks = 0;
    st.st_dev = 0;
    st.st_gid = 0;
    st.st_ino = 0;
    st.st_nlink = 0;
    st.st_rdev = 0;
    st.st_size = 0;
    st.st_uid = 0;
}


static std::string credentials_query(gfal2_context_t context)
{
    gchar* ucert = gfal2_get_opt_string(context, "X509", "CERT", NULL);
    gchar* ukey = gfal2_get_opt_string(context, "X509", "KEY", NULL);
    if (!ucert)
        return std::string();
    if (!ukey)
        ukey = ucert;

    std::ostringstream args;

    // Certificate == key, assume proxy
    if (strcmp(ucert, ukey) == 0) {
        args << "xrd.gsiusrpxy=" << ucert;
    }
    else {
        args << "xrd.gsiusrcrt=" << ucert << '&' << "xrd.gsiusrkey=" << ukey;
    }

    g_free(ucert);
    if (ucert != ukey)
        g_free(ukey);

    return args.str();
}


std::string normalize_url(gfal2_context_t context, const char* url)
{
    const char* p = url + 7; // Jump over root://
    p = strchr(p, '/');

    std::string sanitized;
    if (p == NULL) {
        sanitized = std::string(url) + "///";
    }
    else if (strncmp(p, "///", 3) == 0) {
        sanitized = url;
    }
    else if (strncmp(p, "//", 2) == 0) {
        sanitized = std::string(url, p - url) + "/" + p;
    }
    else {
        sanitized = std::string(url, p - url) + "//" + p;
    }

    std::string creds = credentials_query(context);
    if (!creds.empty()) {
        if (sanitized.find('?') == std::string::npos)
            sanitized += "?";
        sanitized += creds;
    }

    return sanitized;
}

std::string predefined_checksum_type_to_lower(const std::string& type)
{
    std::string lowerForm(type);
    std::transform(lowerForm.begin(), lowerForm.end(), lowerForm.begin(),
            ::tolower);

    if (lowerForm == "adler32" || lowerForm == "crc32" || lowerForm == "md5")
        return lowerForm;
    else
        return type;
}

// Copied from xrootd/src/XrdPosix/XrdPosixMap.cc
int xrootd_errno_to_posix_errno(int rc)
{
    switch(rc)
       {case kXR_ArgInvalid:    return EINVAL;
        case kXR_ArgMissing:    return EINVAL;
        case kXR_ArgTooLong:    return ENAMETOOLONG;
        case kXR_FileLocked:    return EDEADLK;
        case kXR_FileNotOpen:   return EBADF;
        case kXR_FSError:       return EIO;
        case kXR_InvalidRequest:return EEXIST;
        case kXR_IOError:       return EIO;
        case kXR_NoMemory:      return ENOMEM;
        case kXR_NoSpace:       return ENOSPC;
        case kXR_NotAuthorized: return EACCES;
        case kXR_NotFound:      return ENOENT;
        case kXR_ServerError:   return ENOMSG;
        case kXR_Unsupported:   return ENOSYS;
        case kXR_noserver:      return EHOSTUNREACH;
        case kXR_NotFile:       return ENOTBLK;
        case kXR_isDirectory:   return EISDIR;
        case kXR_Cancelled:     return ECANCELED;
        case kXR_ChkLenErr:     return EDOM;
        case kXR_ChkSumErr:     return EDOM;
        case kXR_inProgress:    return EINPROGRESS;
        default:                return ENOMSG;
       }
}


void gfal2_xrootd_set_error(GError **err, int errcode, const char *func, const char *desc, ...)
{
    char error_string[64];
    char *error_string_ptr;

#if ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE) || defined(__APPLE__)
    strerror_r(errcode, error_string, sizeof(error_string));
    error_string_ptr = error_string;
#else
    error_string_ptr = strerror_r(errcode, error_string, sizeof(error_string));
#endif

    char err_msg[256];
    va_list args;
    va_start(args, desc);
    vsnprintf(err_msg, sizeof(err_msg), desc, args);
    va_end(args);

    char buffer[512];
    snprintf(buffer, sizeof(buffer), "%s (%s)", err_msg, error_string_ptr);
    gfal2_set_error(err, xrootd_domain, errno, func, buffer);
}
