/*
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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


/*
 * @file gfal_posix_read.c
 * @brief file for the internal read function for the posix interface
 * @author Devresse Adrien
 * @version 2.0
 * @date 01/07/2011
 * */

 #include <glib.h>
#include <stdlib.h>
#include <common/gfal_types.h>
#include <common/gfal_common_filedescriptor.h>
#include "gfal_posix_internal.h"
#include <common/gfal_common_file_handle.h>
#include <common/gfal_common_plugin.h>



int gfal_posix_internal_read(int fd, void* buff, size_t s_buff)
{
    GError* tmp_err = NULL;
    gfal2_context_t handle;
    int res = -1;

    if ((handle = gfal_posix_instance()) == NULL) {
        errno = EIO;
        return -1;
    }

    res = gfal2_read(handle, fd, buff, s_buff, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(handle, "[gfal_read]", tmp_err);
        errno = tmp_err->code;
    }
    return res;
}


ssize_t gfal_posix_internal_pread(int fd, void* buff, size_t s_buff,
        off_t offset)
{
    GError* tmp_err = NULL;
    gfal2_context_t handle;
    ssize_t res = -1;

    if ((handle = gfal_posix_instance()) == NULL) {
        errno = EIO;
        return -1;
    }

    res = gfal2_pread(handle, fd, buff, s_buff, offset, &tmp_err);
    if (tmp_err) {
        gfal_posix_register_internal_error(handle, "[gfal_pread]", tmp_err);
        errno = tmp_err->code;
    }
    return res;
}
