/*
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <common/gfal_common_filedescriptor.h>
#include <exceptions/cpp_to_gerror.hpp>
#include "gridftp_dir_reader/gridftp_dir_reader.h"
#include "gridftp_namespace.h"
#include "gridftp_plugin.h"

static const Glib::Quark GFAL_GRIDFTP_SCOPE_OPENDIR("gfal_gridftp_opendirG");


extern "C" gfal_file_handle gfal_gridftp_opendirG(plugin_handle handle,
        const char* path, GError** err)
{
    g_return_val_err_if_fail(handle != NULL && path != NULL, NULL, err,
            "[gfal_gridftp_opendirG][gridftp] Invalid parameters");

    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_opendirG]");

    // Since we are deferring the opening, at least check it is there and it is a directory,
    // and we have permissions
    struct stat st;
    if (gfal_gridftp_statG(handle, path, &st, err) != 0) {
        return NULL;
    }
    else if (!S_ISDIR(st.st_mode)) {
        gfal2_set_error(err, GFAL_GRIDFTP_SCOPE_OPENDIR, EISDIR, __func__,
                "%s is not a directory", path);
        return NULL;
    }
    else if ((st.st_mode & ( S_IRUSR | S_IRGRP | S_IROTH)) == 0) {
        gfal2_set_error(err, GFAL_GRIDFTP_SCOPE_OPENDIR, EACCES, __func__, "Can not read %s",
                path);
        return NULL;
    }

    // Do nothing, and defer until the first readdir or readdirpp
    // is called
    return gfal_file_handle_new2(gridftp_plugin_name(), NULL, NULL, path);
}


extern "C" struct dirent* gfal_gridftp_readdirG(plugin_handle handle,
        gfal_file_handle fh, GError** err)
{
    g_return_val_err_if_fail(handle != NULL && fh != NULL, NULL, err,
            "[gfal_gridftp_readdirG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    struct dirent* ret = NULL;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_readdirG]");
    CPP_GERROR_TRY
                GridftpDirReader* reader =
                        static_cast<GridftpDirReader*>(fh->fdesc);
                // Not open yet, so instantiate the simple reader
                if (reader == NULL) {
                    GridFTPModule* gsiftp = static_cast<GridFTPModule*>(handle);
                    reader = new GridftpSimpleListReader(gsiftp, fh->path);
                    fh->fdesc = reader;
                }
                ret = reader->readdir();
            CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_readdirG] <-");
    G_RETURN_ERR(ret, tmp_err, err);
}


extern "C" struct dirent* gfal_gridftp_readdirppG(plugin_handle handle,
        gfal_file_handle fh, struct stat* st, GError** err)
{
    g_return_val_err_if_fail(handle != NULL && fh != NULL, NULL, err,
            "[gfal_gridftp_readdirG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    struct dirent* ret = NULL;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_readdirG]");
    CPP_GERROR_TRY
                GridftpDirReader* reader =
                        static_cast<GridftpDirReader*>(fh->fdesc);
                // Not open yet, so instantiate the simple reader
                if (reader == NULL) {
                    GridFTPModule* gsiftp = static_cast<GridFTPModule*>(handle);
                    reader = new GridftpListReader(gsiftp, fh->path);
                    fh->fdesc = reader;
                }
                ret = reader->readdirpp(st);
            CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_readdirG] <-");
    G_RETURN_ERR(ret, tmp_err, err);
}


extern "C" int gfal_gridftp_closedirG(plugin_handle handle, gfal_file_handle fh,
        GError** err)
{
    g_return_val_err_if_fail(handle != NULL, -1, err,
            "[gfal_gridftp_readdirG][gridftp] Invalid parameters");
    GError * tmp_err = NULL;
    int ret = -1;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_closedirG]");
    CPP_GERROR_TRY
                GridftpDirReader* reader =
                        static_cast<GridftpDirReader*>(fh->fdesc);
                delete reader;
                gfal_file_handle_delete(fh);
                ret = 0;
            CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_closedirG] <-");
    G_RETURN_ERR(ret, tmp_err, err);
}
