/*
 * Copyright (c) CERN 2016
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

#include "gfal_xrootd_plugin_interface.h"
#include "gfal_xrootd_plugin_utils.h"

#include <XrdCl/XrdClFileSystem.hh>
#include <XrdCl/XrdClFileSystemUtils.hh>

#include "space/gfal2_space.h"

ssize_t gfal_xrootd_space_reporting(plugin_handle plugin_data, const char *url,
    const char *key, void *buff, size_t s_buf, GError **err)
{
    std::string sanitizedUrl = prepare_url((gfal2_context_t) plugin_data, url);
    XrdCl::FileSystem fs(sanitizedUrl);
    XrdCl::FileSystemUtils::SpaceInfo *space = NULL;

    XrdCl::URL xurl(sanitizedUrl);

    XrdCl::XRootDStatus status = XrdCl::FileSystemUtils::GetSpaceInfo(space, &fs, xurl.GetPath());
    if (!status.IsOK()) {
        gfal2_set_error(err, xrootd_domain, EIO, __func__, "Failed to get the space information: %s",
            status.GetErrorMessage().c_str());
        return -1;
    }

    struct space_report report = {0};
    report.used = space->GetUsed();
    report.free = space->GetFree();
    report.total = space->GetTotal();

    uint64_t chunk = space->GetLargestFreeChunk();
    report.largest_chunk = &chunk;

    delete space;

    return gfal2_space_generate_json(&report, (char*)buff, s_buf);
}
