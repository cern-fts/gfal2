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

#include <stdlib.h>
#include <glib.h>
#include "gfal_posix_api.h"

#include "gfal_posix_internal.h"
#include <common/gfal_constants.h>
#include <common/gfal_prototypes.h>
#include <common/gfal_common_filedescriptor.h>
#include <common/gfal_common_dir_handle.h>
#include <common/gfal_common_plugin.h>



int gfal_posix_internal_closedir(DIR* d)
{
    GError* tmp_err = NULL;
    int ret = -1;
    gfal2_context_t handle;
    if ((handle = gfal_posix_instance()) == NULL) {
        errno = EIO;
        return -1;
    }

    if (d == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT,
                "File descriptor is NULL");
    }
    else {
        ret = gfal2_closedir(handle, d, &tmp_err);
    }

    if (tmp_err) {
        gfal_posix_register_internal_error(handle, "[gfal_closedir]", tmp_err);
        errno = tmp_err->code;
    }
    return ret;
}
