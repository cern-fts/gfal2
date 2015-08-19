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

#include <stdio.h>
#include <errno.h>
#include <glib.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>

#include "gfal_posix_internal.h"



ssize_t gfal_posix_internal_listxattr(const char *path, char *list, size_t size)
{
    GError* tmp_err = NULL;
    gfal2_context_t handle;
    ssize_t res = -1;

    if ((handle = gfal_posix_instance()) == NULL) {
        errno = EIO;
        return -1;
    }

    res = gfal2_listxattr(handle, path, list, size, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(handle, "[gfal_listxattr]", tmp_err);
        errno = tmp_err->code;
    }
    return res;
}




