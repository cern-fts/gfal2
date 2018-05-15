/*
* Copyright @ CERN, 2015
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

#include "gridftp_parsing.h"

#include <cstdio>
#include <cstring>
#include <glib.h>
#include <globus_libc.h>
#include <globus_gass_copy.h>
#include <grp.h>
#include <pwd.h>

#include <gfal_api.h>


// Adapted from http://cvs.globus.org/viewcvs.cgi/gass/copy/source/globus_gass_copy_glob.c
static int copy_mdtm_to_timet(char * mdtm_str, int * time_out)
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


globus_result_t parse_mlst_line(char *line, struct stat *stat_info, char *filename_buf, size_t filename_size)
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
        size_t len = g_strlcpy(filename_buf, filename, filename_size);
        char* trailing = filename_buf + len;
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
            if ((strcasecmp(factval, "dir") == 0) 
              || (strcasecmp(factval, "pdir") == 0)
              || (strcasecmp(factval, "cdir") == 0))  {
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
        if (strcmp(startfact, "unix.uid") == 0) {
            stat_info->st_uid = atoi(factval);
        }
        if (strcmp(startfact, "unix.gid") == 0) {
            stat_info->st_gid = atoi(factval);
        }


        startfact = endfact + 1;
    }

    stat_info->st_nlink = 1;
    stat_info->st_mode = -1;
    stat_info->st_size = -1;
    stat_info->st_mtime = -1;

    if (mode_s) {
        stat_info->st_mode = strtoul(mode_s, NULL, 8);
        if (type == GLOBUS_GASS_COPY_GLOB_ENTRY_DIR) {
            stat_info->st_mode |= S_IFDIR;
        }
        else {
            stat_info->st_mode |= S_IFREG;
        }
    }

    if (size_s) {
        off_t size;
        int rc;

        rc = sscanf(size_s, "%ld", &size);
        if (rc == 1) {
            stat_info->st_size = size;
        }
    }

    if (modify_s) {
        int mdtm;

        if (copy_mdtm_to_timet(modify_s, &mdtm) == GLOBUS_SUCCESS) {
            stat_info->st_mtime = mdtm;
        }
    }

    return GLOBUS_SUCCESS;

    error_invalid_mlsd:

    return result;
}


static mode_t parse_ls_submode(const char* substr)
{
    return ((substr[0] == 'r') * S_IRUSR) |
           ((substr[1] == 'w') * S_IWUSR) |
           ((substr[2] == 'x') * S_IXUSR);
}


static mode_t parse_ls_mode(const char* mode_str)
{
    mode_t mode = 0;
    if (strlen(mode_str) != 10)
        return mode;

    switch (mode_str[0]) {
        case 'd':
            mode = S_IFDIR;
            break;
        case '-':
            mode = S_IFREG;
            break;
        case 'l':
            mode = S_IFLNK;
            break;
        case 'b':
            mode = S_IFBLK;
            break;
        case 'c':
            mode = S_IFCHR;
            break;
        case 's':
            mode = S_IFSOCK;
            break;
        default:
            // Unknown
            break;
    }

    mode |= parse_ls_submode(mode_str + 1);
    mode |= (parse_ls_submode(mode_str + 4) >> 3);
    mode |= (parse_ls_submode(mode_str + 7) >> 6);

    return mode;
}


globus_result_t parse_stat_line(char* buffer, struct stat* fstat, char *filename_buf, size_t filename_size)
{
    if (!buffer || !fstat)
        return GLOBUS_FAILURE;

    if (filename_buf && filename_size > 0)
        filename_buf[0] = '\0';

    // Lines are like
    // -rw-rw-r--   1 ftp      ftp            49 Oct 29  2009 /pub/ubuntu-releases/robots.txt

    enum FtpField {
        FTP_FIELD_MODE, FTP_FIELD_NLINKS,
        FTP_FIELD_OWNER, FTP_FIELD_GROUP,
        FTP_FIELD_SIZE, FTP_FIELD_MONTH,
        FTP_FIELD_DAY, FTP_FIELD_YEAR_OR_TIME,
        FTP_FIELD_NAME, FTP_FIELD_LINK
    };
    int field = FTP_FIELD_MODE;

    struct tm timedef;
    memset(&timedef, 0, sizeof(timedef));
    time_t now = time(NULL);
    struct tm today;
    localtime_r(&now, &today);
    char *colon;

    char *start = buffer;
    while (*start && field < FTP_FIELD_LINK) {
        while (isspace(*start) && *start)
            ++start;
        if (!*start)
            break;

        bool eol = false;
        char* end = start;
        while (!isspace(*end) && *end)
            ++end;
        if (!*end)
            eol = true;
        *end = '\0';

        switch (field) {
            case FTP_FIELD_MODE:
                fstat->st_mode = parse_ls_mode(start);
                break;
            case FTP_FIELD_NLINKS:
                fstat->st_nlink = atol(start);
                break;
            case FTP_FIELD_OWNER:
                if (isdigit(*start)) {
                    fstat->st_uid = atoi(start);
                }
                else {
                    char usrbuf[128];
                    struct passwd usr, *usr_ptr;
                    if (getpwnam_r(start, &usr, usrbuf, sizeof(usrbuf), &usr_ptr) == 0) {
                        fstat->st_uid = usr.pw_uid;
                    }
                    else {
                        gfal2_log(G_LOG_LEVEL_WARNING, "Could not get uid for %s (%d)", start, errno);
                    }
                }
                break;
            case FTP_FIELD_GROUP:
                if (isdigit(*start)) {
                    fstat->st_gid = atoi(start);
                }
                else {
                    char grbuf[128];
                    struct group grp, *grp_ptr;
                    if (getgrnam_r(start, &grp, grbuf, sizeof(grbuf), &grp_ptr) == 0) {
                        fstat->st_gid = grp.gr_gid;
                    }
                    else {
                        gfal2_log(G_LOG_LEVEL_WARNING, "Could not get gid for %s (%d)", start, errno);
                    }
                }
                break;
            case FTP_FIELD_SIZE:
                fstat->st_size = atol(start);
                break;
            case FTP_FIELD_MONTH:
                strptime(start, "%b", &timedef);
                break;
            case FTP_FIELD_DAY:
                timedef.tm_mday = atoi(start);
                break;
            case FTP_FIELD_YEAR_OR_TIME:
                if ((colon = strchr(start, ':'))) {
                    timedef.tm_year = today.tm_year;
                    timedef.tm_hour = atoi(start);
                    timedef.tm_min = atoi(colon + 1);
                }
                else {
                    timedef.tm_year = atoi(start) - 1900;
                }
                break;
            case FTP_FIELD_NAME:
                if (filename_buf && filename_size) {
                    g_strlcpy(filename_buf, start, filename_size);
                }
            default:
                break;
        }

        if (eol)
            break;

        // Next field
        start = end + 1;
        field++;
    }

    struct tm gmt_now_tm;
    memset(&gmt_now_tm, '\0', sizeof(struct tm));
    globus_libc_gmtime_r(&now, &gmt_now_tm);

    time_t gmt_now = mktime(&gmt_now_tm);
    time_t offset = now - gmt_now;

    fstat->st_atime = fstat->st_mtime = fstat->st_ctime = mktime(&timedef) - offset;

    return GLOBUS_SUCCESS;
}
