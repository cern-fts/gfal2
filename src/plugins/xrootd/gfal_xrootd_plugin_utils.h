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

#ifndef GFAL_XROOTD_PLUGIN_UTILS_H_
#define GFAL_XROOTD_PLUGIN_UTILS_H_

#include <XrdCl/XrdClFileSystem.hh>

#include <gfal_api.h>
#include <json.h>
#include <string>
#include <sys/stat.h>

extern GQuark xrootd_domain;

/// Convert file mode_t to ints
XrdCl::Access::Mode file_mode_to_xrdcl_access( mode_t mode );

/// Initialize all stat fields to zero
void reset_stat(struct stat& st);

/// Return the same URL, but making sure the path is always relative
/// and adding the user credentials appended as keywords
std::string prepare_url(gfal2_context_t context, const char *url);

/// If the checksum type is one of the predefined ones, always lowercase
/// @note adler32, crc32, md5
std::string predefined_checksum_type_to_lower(const std::string& type);

/// Parse a JSON object into a boolean value
bool json_obj_to_bool(struct json_object *boolobj);

/// Collapse multiple consecutive slashes into a single one
void collapse_slashes(std::string& path);

/// Map an xrootd status code to a posix errno
/// @note for a query prepare, mask network errors as ECOMM
int xrootd_status_to_posix_errno(const XrdCl::XRootDStatus& status, bool query_prepare = false);

/// Set error code with errno description
void gfal2_xrootd_set_error(GError **err, int errcode, const char *func, const char *desc, ...);

/// Set error code during polling, providing additional reason
void gfal2_xrootd_poll_set_error(GError **err, int errcode, const char *func, const char *err_reasno,
                                 const char *format, ...);

/// Copy contents of source buffer to dest buffer and always terminate dest buffer with null terminator
void copy_to_cstring(char* dest, size_t dest_size, const char* source, size_t source_size);

#endif /* GFAL_XROOTD_PLUGIN_UTILS_H_ */
