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

#include <grp.h>
#include <exceptions/cpp_to_gerror.hpp>
#include <globus_ftp_client.h>
#include "gridftp_namespace.h"
#include "gridftp_parsing.h"


static const GQuark GFAL_GRIDFTP_SCOPE_STAT = g_quark_from_static_string("Gridftp_stat_module::stat");
static const GQuark GFAL_GRIDFTP_SCOPE_ACCESS = g_quark_from_static_string("Gridftp_stat_module::access");


void GridFTPModule::stat(const char* path, struct stat * st)
{
    if (path == NULL || st == NULL) {
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_STAT, EINVAL,
                "Invalid arguments path or stat ");
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, " -> [GridFTPModule::stat] ");
    internal_globus_gass_stat(path, st);
    gfal2_log(G_LOG_LEVEL_DEBUG, " <- [GridFTPModule::stat] ");
}


void GridFTPModule::access(const char* path, int mode)
{
    if (path == NULL) {
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_STAT, EINVAL,
                "Invalid arguments path or stat ");
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, " -> [Gridftp_stat_module::access] ");
    struct stat st;
    internal_globus_gass_stat(path, &st);

    if (st.st_mode == (mode_t)-1) { // mode not managed by server
        gfal2_log(G_LOG_LEVEL_MESSAGE,
                "Access request is not managed by this server %s , return access authorized by default",
                path);
        return;
    }

    const mode_t file_mode = (mode_t) st.st_mode;
    if (((file_mode & ( S_IRUSR | S_IRGRP | S_IROTH)) == FALSE) && (mode & R_OK)) {
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_ACCESS, EACCES,
                "No read access");
    }

    if (((file_mode & ( S_IWUSR | S_IWGRP | S_IWOTH)) == FALSE) && (mode & W_OK)) {
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_ACCESS, EACCES,
                "No write access");
    }

    if (((file_mode & ( S_IXUSR | S_IXGRP | S_IXOTH)) == FALSE) && (mode & X_OK)) {
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_ACCESS, EACCES,
                "No execute access");
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, " <- [Gridftp_stat_module::access] ");
}


static void gridftp_stat_mlst(GridFTPSessionHandler *handler, const char* path, struct stat* fstat)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, "Stat via MLST");

    globus_byte_t *buffer = NULL;
    globus_size_t buflen = 0;

    GridFTPRequestState req(handler);

    globus_result_t res = globus_ftp_client_mlst(handler->get_ftp_client_handle(), path,
                                                 handler->get_ftp_client_operationattr(), &buffer, &buflen,
                                                 globus_ftp_client_done_callback, &req);

    gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_STAT, res);
    req.wait(GFAL_GRIDFTP_SCOPE_STAT);

    gfal2_log(G_LOG_LEVEL_DEBUG, "   <- [%s]] Got '%s'", __func__, buffer);

    parse_mlst_line((char *) buffer, fstat, NULL, 0);
    globus_free(buffer);
}


static void gridftp_stat_stat(GridFTPSessionHandler *handler, const char* path, struct stat* fstat)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, "Stat via STAT");

    globus_byte_t *buffer = NULL;
    globus_size_t buflen = 0;

    GridFTPRequestState req(handler);

    globus_result_t res = globus_ftp_client_stat(handler->get_ftp_client_handle(), path,
                                                 handler->get_ftp_client_operationattr(), &buffer, &buflen,
                                                 globus_ftp_client_done_callback, &req);

    gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_STAT, res);
    req.wait(GFAL_GRIDFTP_SCOPE_STAT);

    gfal2_log(G_LOG_LEVEL_DEBUG, "   <- [%s]] Got '%s'", __func__, buffer);

    char* p = (char*)buffer;
    if (strncmp(p, "211", 3) == 0) {
        p += 4;
    }
    else if (strncmp(p, "213", 3) == 0) {
        p = strchr(p, '\n');
        if (p)
            ++p;
    }

    parse_stat_line(p, fstat, NULL, 0);
    globus_free(buffer);
}


void GridFTPModule::internal_globus_gass_stat(const char* path,
        struct stat* fstat)
{
    gfal2_log(G_LOG_LEVEL_DEBUG,
              " -> [Gridftp_stat_module::globus_gass_stat] ");

    GridFTPSessionHandler handler(get_session_factory(), path);

    globus_ftp_client_tristate_t supported;
    globus_ftp_client_is_feature_supported(handler.get_ftp_features(),
        &supported, GLOBUS_FTP_CLIENT_FEATURE_MLST);

    if (supported != GLOBUS_FTP_CLIENT_FALSE) {
        gridftp_stat_mlst(&handler, path, fstat);
    }
    else {
        gridftp_stat_stat(&handler, path, fstat);
    }

    gfal2_log(G_LOG_LEVEL_DEBUG,
              " <- [Gridftp_stat_module::internal_globus_gass_stat] ");
}


extern "C" int gfal_gridftp_statG(plugin_handle handle, const char* name,
        struct stat* buff, GError ** err)
{
    g_return_val_err_if_fail(handle != NULL && name != NULL && buff != NULL, -1,
            err, "[gfal_gridftp_statG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal2_log(G_LOG_LEVEL_DEBUG, "  -> [gfal_gridftp_statG]");
    CPP_GERROR_TRY
                (static_cast<GridFTPModule*>(handle))->stat(name, buff);
                ret = 0;
    CPP_GERROR_CATCH(&tmp_err);
    gfal2_log(G_LOG_LEVEL_DEBUG, "  [gfal_gridftp_statG]<-");
    G_RETURN_ERR(ret, tmp_err, err);
}


extern "C" int gfal_gridftp_accessG(plugin_handle handle, const char* name,
        int mode, GError** err)
{
    g_return_val_err_if_fail(handle != NULL && name != NULL, -1, err,
            "[gfal_gridftp_statG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal2_log(G_LOG_LEVEL_DEBUG, "  -> [gfal_gridftp_accessG]");
    CPP_GERROR_TRY
        (static_cast<GridFTPModule*>(handle))->access(name, mode);
        ret = 0;
    CPP_GERROR_CATCH(&tmp_err);
    gfal2_log(G_LOG_LEVEL_DEBUG, "  [gfal_gridftp_accessG]<-");
    G_RETURN_ERR(ret, tmp_err, err);
}
