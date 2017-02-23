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

#include <gfal_api.h>
#include <string>
#include <sys/stat.h>

extern GQuark xrootd_domain;

/// Convert file mode_t to ints
void file_mode_to_xrootd_ints(mode_t mode, int& user, int& group, int& other);

/// Initialize all stat fields to zero
void reset_stat(struct stat& st);

/// Return the same URL, but making sure the path is always relative
/// and adding the user credentials appended as keywords
std::string normalize_url(gfal2_context_t context, const char* url);

/// If the checksum type is one of the predefined ones, always lowercase
/// @note adler32, crc32, md5
std::string predefined_checksum_type_to_lower(const std::string& type);

/// Map an xrootd errno to a posix errno
int xrootd_errno_to_posix_errno(int rc);

/// Set error code with errno description
void gfal2_xrootd_set_error(GError **err, int errcode, const char *func, const char *desc, ...);


#endif /* GFAL_XROOTD_PLUGIN_UTILS_H_ */
