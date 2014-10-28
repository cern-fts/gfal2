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

#include <exceptions/cpp_to_gerror.hpp>
#include <globus_ftp_client.h>
#include "gridftp_namespace.h"
#include "gridftpwrapper.h"


static Glib::Quark GFAL_GRIDFTP_SCOPE_STAT("Gridftp_stat_module::stat");
static Glib::Quark GFAL_GRIDFTP_SCOPE_ACCESS("Gridftp_stat_module::access");


void GridFTPModule::stat(const char* path, struct stat * st)
{
    if (path == NULL || st == NULL)
        throw Glib::Error(GFAL_GRIDFTP_SCOPE_STAT, EINVAL,
                "Invalid arguments path or stat ");
    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridFTPModule::stat] ");
    globus_gass_copy_glob_stat_t gl_stat;
    memset(&gl_stat, 0, sizeof(globus_gass_copy_glob_stat_t));
    internal_globus_gass_stat(path, &gl_stat);

    memset(st, 0, sizeof(struct stat));
    st->st_mode = (mode_t) ((gl_stat.mode != -1) ? gl_stat.mode : 0);
//	st->st_mode |= (gl_stat.symlink_target != NULL)?(S_IFLNK):0;
    st->st_mode |=
            (gl_stat.type == GLOBUS_GASS_COPY_GLOB_ENTRY_DIR) ?
                    (S_IFDIR) : (S_IFREG);
    st->st_size = (off_t) gl_stat.size;
    st->st_mtime = (time_t) (gl_stat.mdtm != -1) ? (gl_stat.mdtm) : 0;

    globus_libc_free(gl_stat.unique_id);
    globus_libc_free(gl_stat.symlink_target);

    gfal_log(GFAL_VERBOSE_TRACE, " <- [GridFTPModule::stat] ");
}


void GridFTPModule::access(const char* path, int mode)
{
    if (path == NULL)
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_STAT,
                "Invalid arguments path or stat ",
                EINVAL);

    gfal_log(GFAL_VERBOSE_TRACE, " -> [Gridftp_stat_module::access] ");
    globus_gass_copy_glob_stat_t gl_stat;
    memset(&gl_stat, 0, sizeof(globus_gass_copy_glob_stat_t));
    internal_globus_gass_stat(path, &gl_stat);

    if (gl_stat.mode == -1) { // mode not managed by server
        gfal_log(GFAL_VERBOSE_VERBOSE,
                "Access request is not managed by this server %s , return access authorized by default",
                path);
        return;
    }

    const mode_t file_mode = (mode_t) gl_stat.mode;
    if (((file_mode & ( S_IRUSR | S_IRGRP | S_IROTH)) == FALSE)
            && (mode & R_OK))
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_ACCESS,
                "No read access ", EACCES);

    if (((file_mode & ( S_IWUSR | S_IWGRP | S_IWOTH)) == FALSE)
            && (mode & W_OK))
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_ACCESS,
                "No write access ", EACCES);

    if (((file_mode & ( S_IXUSR | S_IXGRP | S_IXOTH)) == FALSE)
            && (mode & X_OK))
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_ACCESS,
                "No execute access ", EACCES);

    gfal_log(GFAL_VERBOSE_TRACE, " <- [Gridftp_stat_module::access] ");
}

// Adapted from http://cvs.globus.org/viewcvs.cgi/gass/copy/source/globus_gass_copy_glob.c
static
int copy_mdtm_to_timet(char * mdtm_str, int * time_out)
{
    char * p;
    struct tm tm;
    struct tm gmt_now_tm;
    struct tm * gmt_now_tm_p;
    time_t offset;
    time_t gmt_now;
    time_t now;
    time_t file_time;
    int rc;

    p = mdtm_str;

    memset(&tm, '\0', sizeof(struct tm));

    /* 4 digit year */
    rc = sscanf(p, "%04d", &tm.tm_year);
    if (rc != 1) {
        goto error_exit;
    }
    tm.tm_year -= 1900;
    p += 4;

    /* 2 digit month [01-12] */
    rc = sscanf(p, "%02d", &tm.tm_mon);
    if (rc != 1) {
        goto error_exit;
    }
    tm.tm_mon--;
    p += 2;

    /* 2 digit day/month [01-31] */
    rc = sscanf(p, "%02d", &tm.tm_mday);
    if (rc != 1) {
        goto error_exit;
    }
    p += 2;

    /* 2 digit hour [00-23] */
    rc = sscanf(p, "%02d", &tm.tm_hour);
    if (rc != 1) {
        goto error_exit;
    }
    p += 2;

    /* 2 digit minute [00-59] */
    rc = sscanf(p, "%02d", &tm.tm_min);
    if (rc != 1) {
        goto error_exit;
    }
    p += 2;

    /* 2 digit second [00-60] */
    rc = sscanf(p, "%02d", &tm.tm_sec);
    if (rc != 1) {
        goto error_exit;
    }
    p += 2;

    file_time = mktime(&tm);
    if (file_time == (time_t) -1) {
        goto error_exit;
    }

    now = time(&now);
    if (now == (time_t) -1) {
        goto error_exit;
    }

    memset(&gmt_now_tm, '\0', sizeof(struct tm));
    gmt_now_tm_p = globus_libc_gmtime_r(&now, &gmt_now_tm);
    if (gmt_now_tm_p == NULL) {
        goto error_exit;
    }

    gmt_now = mktime(&gmt_now_tm);
    if (gmt_now == (time_t) -1) {
        goto error_exit;
    }

    offset = now - gmt_now;

    *time_out = file_time + offset;

    return 0;

 error_exit:
    return -1;
}


globus_result_t parse_mlst_line(char *line,
        globus_gass_copy_glob_stat_t *stat_info,
        char *filename_buf, size_t filename_size)
{
    globus_result_t result;
    int i;
    char * space;
    char * filename;
    char * startline;
    char * startfact;
    char * endfact;
    char * factval;

    char * unique_id = NULL;
    char * mode_s = NULL;
    char * symlink_target = NULL;
    char * modify_s = NULL;
    char * size_s = NULL;
    globus_gass_copy_glob_entry_t type = GLOBUS_GASS_COPY_GLOB_ENTRY_FILE;

    startline = line;

    space = strchr(startline, ' ');
    if (space == GLOBUS_NULL) {
        result = globus_error_put(
                globus_error_construct_string(GLOBUS_GASS_COPY_MODULE,
                        GLOBUS_NULL, "[%s]: Bad MLSD response", __func__));

        goto error_invalid_mlsd;
    }
    *space = '\0';
    filename = space + 1;
    startfact = startline;

    if (filename_buf) {
        g_strlcpy(filename_buf, filename, filename_size);
        char* trailing = filename_buf + strlen(filename);
        do {
            *trailing = '\0';
            --trailing;
        } while (trailing >= filename_buf && isspace(*trailing));
    }

    while (startfact != space) {
        endfact = strchr(startfact, ';');
        if (endfact) {
            *endfact = '\0';
        }
        else {
            /*
             older MLST-draft spec says ending fact can be missing
             the final semicolon... not a problem to support this,
             no need to die.  (ncftpd does this)

             result = globus_error_put(
             globus_error_construct_string(
             GLOBUS_GASS_COPY_MODULE,
             GLOBUS_NULL,
             "[%s]: Bad MLSD response",
             myname));

             goto error_invalid_mlsd;
             */

            endfact = space - 1;
        }

        factval = strchr(startfact, '=');
        if (!factval) {
            result = globus_error_put(
                    globus_error_construct_string(GLOBUS_GASS_COPY_MODULE,
                            GLOBUS_NULL, "[%s]: Bad MLSD response", __func__));

            goto error_invalid_mlsd;
        }
        *(factval++) = '\0';

        for (i = 0; startfact[i] != '\0'; i++) {
            startfact[i] = tolower(startfact[i]);
        }

        if (strcmp(startfact, "type") == 0) {
            if (strcasecmp(factval, "dir") == 0) {
                type = GLOBUS_GASS_COPY_GLOB_ENTRY_DIR;
            }
            else if (strcasecmp(factval, "file") == 0) {
                type = GLOBUS_GASS_COPY_GLOB_ENTRY_FILE;
            }
            else {
                type = GLOBUS_GASS_COPY_GLOB_ENTRY_OTHER;
            }
        }
        if (strcmp(startfact, "unique") == 0) {
            unique_id = factval;
        }
        if (strcmp(startfact, "unix.mode") == 0) {
            mode_s = factval;
        }
        if (strcmp(startfact, "modify") == 0) {
            modify_s = factval;
        }
        if (strcmp(startfact, "size") == 0) {
            size_s = factval;
        }
        if (strcmp(startfact, "unix.slink") == 0) {
            symlink_target = factval;
        }

        startfact = endfact + 1;
    }

    stat_info->type = type;
    stat_info->unique_id = globus_libc_strdup(unique_id);
    stat_info->symlink_target = globus_libc_strdup(symlink_target);
    stat_info->mode = -1;
    stat_info->size = -1;
    stat_info->mdtm = -1;

    if (mode_s) {
        stat_info->mode = strtoul(mode_s, NULL, 0);
    }

    if (size_s) {
        off_t size;
        int rc;

        rc = sscanf(size_s, "%ld", &size);
        if (rc == 1) {
            stat_info->size = size;
        }
    }

    if (modify_s) {
        int mdtm;

        if (copy_mdtm_to_timet(modify_s, &mdtm) == GLOBUS_SUCCESS) {
            stat_info->mdtm = mdtm;
        }
    }

    return GLOBUS_SUCCESS;

    error_invalid_mlsd:

    return result;
}


void GridFTPModule::internal_globus_gass_stat(const char* path,
        globus_gass_copy_glob_stat_t * gl_stat)
{

    gfal_log(GFAL_VERBOSE_TRACE,
            " -> [Gridftp_stat_module::globus_gass_stat] ");
    GridFTPSession sess(
            _handle_factory->gfal_globus_ftp_take_handle(
                    gridftp_hostname_from_url(path)));

    Gass_attr_handler gass_attr_src(sess.get_op_attr_ftp());

    globus_byte_t *buffer = NULL;
    globus_size_t buflen = 0;
    GridFTPRequestState req(&sess, false);

    globus_result_t res = globus_ftp_client_mlst(sess.get_ftp_handle(), path,
            sess.get_op_attr_ftp(), &buffer, &buflen,
            globus_basic_client_callback, &req);

    gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_STAT, res);
    req.wait_callback(GFAL_GRIDFTP_SCOPE_STAT);

    gfal_log(GFAL_VERBOSE_TRACE,
            "   <- [Gridftp_stat_module::internal_globus_gass_stat] Got '%s'",
            buffer);

    parse_mlst_line((char*) buffer, gl_stat, NULL, 0);
    globus_free(buffer);

    gfal_log(GFAL_VERBOSE_TRACE,
            " <- [Gridftp_stat_module::internal_globus_gass_stat] ");
}


extern "C" int gfal_gridftp_statG(plugin_handle handle, const char* name,
        struct stat* buff, GError ** err)
{
    g_return_val_err_if_fail(handle != NULL && name != NULL && buff != NULL, -1,
            err, "[gfal_gridftp_statG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_statG]");
    CPP_GERROR_TRY
                (static_cast<GridFTPModule*>(handle))->stat(name, buff);
                ret = 0;
            CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_statG]<-");
    G_RETURN_ERR(ret, tmp_err, err);
}


extern "C" int gfal_gridftp_accessG(plugin_handle handle, const char* name,
        int mode, GError** err)
{
    g_return_val_err_if_fail(handle != NULL && name != NULL, -1, err,
            "[gfal_gridftp_statG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_accessG]");
    CPP_GERROR_TRY
                (static_cast<GridFTPModule*>(handle))->access(name, mode);
                ret = 0;
            CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_accessG]<-");
    G_RETURN_ERR(ret, tmp_err, err);
}
