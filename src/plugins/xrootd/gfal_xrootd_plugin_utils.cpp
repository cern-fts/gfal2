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

// Copied from xrootd/src/XrdPosix/XrdPosixMap.cc
int xrootd_errno_to_posix_errno(int rc)
{
  return XProtocol::toErrno( rc );
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
