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
#include <XrdCl/XrdClFileSystem.hh>

#include <uri/gfal2_uri.h>
#include "gfal_xrootd_plugin_utils.h"
#include "gfal_xrootd_plugin_interface.h"

GQuark xrootd_domain = g_quark_from_static_string("xroot");


XrdCl::Access::Mode file_mode_to_xrdcl_access( mode_t mode )
{
  XrdCl::Access::Mode xrdcl_mode = XrdCl::Access::None;
  if( mode & S_IRUSR )
    xrdcl_mode |= XrdCl::Access::UR;
  if( mode & S_IWUSR )
    xrdcl_mode |= XrdCl::Access::UW;
  if( mode & S_IXUSR )
    xrdcl_mode |= XrdCl::Access::UX;

  if( mode & S_IRGRP )
    xrdcl_mode |= XrdCl::Access::GR;
  if( mode & S_IWGRP )
    xrdcl_mode |= XrdCl::Access::GW;
  if( mode & S_IXGRP )
    xrdcl_mode |= XrdCl::Access::GX;

  if( mode & S_IROTH )
    xrdcl_mode |= XrdCl::Access::OR;
  if( mode & S_IWOTH )
    xrdcl_mode |= XrdCl::Access::OW;
  if( mode & S_IXOTH )
    xrdcl_mode |= XrdCl::Access::OX;

  return xrdcl_mode;
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


static std::string query_args(gfal2_context_t context, const char *url)
{
    bool prev_args = false;

    GError *error = NULL;
    gchar *ucert = gfal2_cred_get(context, GFAL_CRED_X509_CERT, url, NULL, &error);
    g_clear_error(&error);
    gchar *ukey = gfal2_cred_get(context, GFAL_CRED_X509_KEY, url, NULL, &error);
    g_clear_error(&error);

    std::ostringstream args;

    if (ucert) {

        if (!ukey) {
            ukey = ucert;
        }


        // Certificate == key, assume proxy
        if (strcmp(ucert, ukey) == 0) {
           args << "xrd.gsiusrpxy=" << ucert;
           prev_args = true;
        }
        else {
           args << "xrd.gsiusrcrt=" << ucert << '&' << "xrd.gsiusrkey=" << ukey;
           prev_args = true;
        }

        g_free(ucert);

        if (ucert != ukey)
            g_free(ukey);
    }

    // Additional keys
    gsize keyCount;
    gchar **keys = gfal2_get_opt_keys(context, "XROOTD PLUGIN", &keyCount, &error);
    if (keys != NULL) {
        for (gsize keyIndex =0; keyIndex < keyCount; ++keyIndex) {
            if (g_str_has_prefix(keys[keyIndex], "XRD.")) {
                gchar *lowercase = g_utf8_strdown(keys[keyIndex], -1);
                gchar *value = gfal2_get_opt_string_with_default(context, "XROOTD PLUGIN", keys[keyIndex], "");

                // Note: When value is a list, value may have ';', which should be replaced with ','
                for (gsize i = 0; value[i] != '\0'; ++i) {
                    if (value[i] == ';') {
                        value[i] = ',';
                    }
                }

                if (prev_args) {
                    args << "&";
                }
                args << lowercase << "=" << value;
                prev_args = true;
                g_free(lowercase);
                g_free(value);
            }
        }
    }
    g_clear_error(&error);
    g_strfreev(keys);

    return args.str();
}

/**
 * Add to the URL query any argument required to make the configuration effective
 */
static void fill_query(gfal_handle_ *context, const char *url, gfal2_uri *uri)
{
    std::string args = query_args(context, url);
    if (!args.empty()) {
        if (uri->query != NULL) {
            char *p = uri->query;
            uri->query = g_strconcat(uri->query, "&", args.c_str(), NULL);
            g_free(p);
        } else {
            uri->query = g_strdup(args.c_str());
        }
    }
    gfal2_log(G_LOG_LEVEL_DEBUG, "Xrootd Query URI: %s", uri->query);
}

/**
 *  Normalize the URL, so the path starts with the right number of slashes
 */
static void normalize_path(gfal2_uri *uri)
{
    if (uri->path == NULL) {
        uri->path = g_strdup("///");
    }
    else if (strncmp(uri->path, "///", 3) == 0) {
        // pass
    }
    else if (strncmp(uri->path, "//", 2) == 0) {
        char *p = uri->path;
        uri->path = g_strconcat("/", uri->path, NULL);
        g_free(p);
    }
    else {
        char *p = uri->path;
        uri->path = g_strconcat("//", uri->path, NULL);
        g_free(p);
    }
}


std::string prepare_url(gfal2_context_t context, const char *url)
{
    GError *error = NULL;
    gfal2_uri *uri = gfal2_parse_uri(url, &error);
    if (error != NULL) {
        g_clear_error(&error);
        return url;
    }

    gboolean normalize = gfal2_get_opt_boolean_with_default(context,
        XROOTD_CONFIG_GROUP, XROOTD_NORMALIZE_PATH, TRUE);
    if (normalize) {
        normalize_path(uri);
    }
    fill_query(context, url, uri);

    // Url-decode path
    // XRootD and SRM expect the path to be url-decoded, while dav and gsiftp expect the opposite
    gfal2_urldecode(uri->path);

    char *new_url = gfal2_join_uri(uri);
    std::string sanitized(new_url);
    gfal2_free_uri(uri);
    g_free(new_url);
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


bool json_obj_to_bool(struct json_object *boolobj)
{
  if( !boolobj ) return false;
  static const std::string str_true( "true" );
  std::string str_bool = json_object_get_string( boolobj );
  std::transform( str_bool.begin(), str_bool.end(), str_bool.begin(), tolower );
  return ( str_bool == str_true );
}


void collapse_slashes(std::string& path)
{
  std::string::iterator itr = path.begin(), store = path.begin();
  ++itr;

  while( itr != path.end() )
  {
    if( *store != '/' || *itr != '/' )
    {
      ++store;
      *store = *itr;
    }
    ++itr;
  }

  size_t size = store - path.begin() + 1;
  if( path.size() != size )
    path.resize( size );
}


// Mask network error codes as ECOMM
#ifndef ECOMM
#define ECOMM EIO
#endif

static int mask_network_errno(int errc)
{
    switch (errc) {
        case EHOSTUNREACH:
        case ENOTSOCK:
        case ETIMEDOUT:
        case ENOTCONN:
        case ECONNRESET:
        case ECONNREFUSED:
        case ENETRESET:
        case ECONNABORTED:
            return ECOMM;
        default:
            return errc;
    }
}


// Copied from xrootd/src/XrdPosix/XrdPosixMap.cc
#ifndef ENOSR
#define ENOSR ENOSPC
#endif

#ifndef ECHRNG
#define ECHRNG EINVAL
#endif

static int map_status_code_to_errno(int code)
{
    switch (code) {
        case XrdCl::errRetry:                return EAGAIN;       // Cl:001
        case XrdCl::errInvalidOp:            return EOPNOTSUPP;   // Cl:003
        case XrdCl::errConfig:               return ENOEXEC;      // Cl:006
        case XrdCl::errInvalidArgs:          return EINVAL;       // Cl:009
        case XrdCl::errInProgress:           return EINPROGRESS;  // Cl:010
        case XrdCl::errNotSupported:         return ENOTSUP;      // Cl:013
        case XrdCl::errDataError:            return EDOM;         // Cl:014
        case XrdCl::errNotImplemented:       return ENOSYS;       // Cl:015
        case XrdCl::errNoMoreReplicas:       return ENOSR;        // Cl:016
        case XrdCl::errInvalidAddr:          return EHOSTUNREACH; // Cl:101
        case XrdCl::errSocketError:          return ENOTSOCK;     // Cl:102
        case XrdCl::errSocketTimeout:        return ETIMEDOUT;    // Cl:103
        case XrdCl::errSocketDisconnected:   return ENOTCONN;     // Cl:104
        case XrdCl::errStreamDisconnect:     return ECONNRESET;   // Cl:107
        case XrdCl::errConnectionError:      return ECONNREFUSED; // Cl:108
        case XrdCl::errInvalidSession:       return ECHRNG;       // Cl:109
        case XrdCl::errTlsError:             return ENETRESET;    // Cl:110
        case XrdCl::errInvalidMessage:       return EPROTO;       // Cl:201
        case XrdCl::errHandShakeFailed:      return EPROTO;       // Cl:202
        case XrdCl::errLoginFailed:          return ECONNABORTED; // Cl:203
        case XrdCl::errAuthFailed:           return EAUTH;        // Cl:204
        case XrdCl::errQueryNotSupported:    return ENOTSUP;      // Cl:205
        case XrdCl::errOperationExpired:     return ESTALE;       // Cl:206
        case XrdCl::errOperationInterrupted: return EINTR;        // Cl:207
        case XrdCl::errNoMoreFreeSIDs:       return ENOSR;        // Cl:301
        case XrdCl::errInvalidRedirectURL:   return ESPIPE;       // Cl:302
        case XrdCl::errInvalidResponse:      return EBADMSG;      // Cl:303
        case XrdCl::errNotFound:             return EIDRM;        // Cl:304
        case XrdCl::errCheckSumError:        return EILSEQ;       // Cl:305
        case XrdCl::errRedirectLimit:        return ELOOP;        // Cl:306
        default:                             return ENOMSG;
    }
}


int xrootd_status_to_posix_errno(const XrdCl::XRootDStatus& status, bool query_prepare)
{
    int ret;

    if (status.IsOK()) {
        return 0;
    }

    if (status.code == XrdCl::errErrorResponse) {
        ret = XProtocol::toErrno(status.errNo);
    } else {
        ret = (status.errNo ? status.errNo : map_status_code_to_errno(status.code));
    }

    if (query_prepare) {
        ret = mask_network_errno(ret);
    }

    return ret;
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
    gfal2_set_error(err, xrootd_domain, errno, func, "%s", buffer);
}


void gfal2_xrootd_poll_set_error(GError **err, int errcode, const char *func, const char *err_reason,
                                 const char *format, ...)
{
  char err_msg[256];
  va_list args;
  va_start(args, format);
  vsnprintf(err_msg, sizeof(err_msg), format, args);
  va_end(args);

  char buffer[512];

  if (err_reason != NULL) {
    snprintf(buffer, sizeof(buffer), "%s (reason: %s)", err_msg, err_reason);
  } else {
    snprintf(buffer, sizeof(buffer), "%s", err_msg);
  }

  gfal2_set_error(err, xrootd_domain, errcode, func, "%s", buffer);
}
