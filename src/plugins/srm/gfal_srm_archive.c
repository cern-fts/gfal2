/*
 * Copyright (c) CERN 2020
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

#include "gfal_srm.h"
#include "gfal_srm_archive.h"
#include "gfal_srm_namespace.h"


int gfal_srm_archive_pollG(plugin_handle ch, const char* surl, GError** err)
{
    GError* tmp_err = NULL;
    char buffer[1024];
    int poll_result = 0;
    int ret = -1;

    g_return_val_err_if_fail(ch && surl, EINVAL, err,
                             "[gfal_srm_archive_pollG] Invalid value handle and/or surl");

    gfal2_log(G_LOG_LEVEL_DEBUG, " gfal_srm_archive_pollG ->");
    ret = gfal_srm_status_getxattrG(ch, surl, GFAL_XATTR_STATUS, buffer, sizeof(buffer), &tmp_err);

    if (ret > 0 && strlen(buffer) && tmp_err == NULL) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "GFAL_XATTR_STATUS response: %s", buffer);

        if ((strncmp(buffer, GFAL_XATTR_STATUS_NEARLINE, sizeof(GFAL_XATTR_STATUS_NEARLINE)) == 0) ||
            (strncmp(buffer, GFAL_XATTR_STATUS_NEARLINE_ONLINE, sizeof(GFAL_XATTR_STATUS_NEARLINE_ONLINE)) == 0)) {
            poll_result = 1;
        } else {
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EAGAIN, __func__,
                            "File %s is not yet archived", surl);
        }
    } else if (ret == -1 || tmp_err) {
        poll_result = -1;

        if (!tmp_err) {
            gfal2_set_error(&tmp_err, gfal2_get_plugin_srm_quark(), EINVAL, __func__,
                            "polling failed but error not set by getxattr");
        }
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, " gfal_srm_archive_pollG <-");
    G_RETURN_ERR(poll_result, tmp_err, err);
}


int gfal_srm_archive_poll_listG(plugin_handle ch, int nbfiles, const char* const* surls, GError** errors)
{
    int error_count = 0;
    int ontape_count = 0;
    int ret = -1;
    int i;

    if (nbfiles <= 0) {
        return 1;
    }

    if (!(ch && surls)) {
        for (i = 0 ; i < nbfiles; i++) {
            gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(), EINVAL, __func__,
                            "Invalid value handle and/or surls array");
        }
        return -1;
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, " gfal_srm_archive_poll_listG ->");

    for (i = 0; i < nbfiles; i++) {
        if (!surls[i]) {
            gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(), EINVAL, __func__, "Invalid surl value");
            error_count++;
            continue;
        }

        ret = gfal_srm_archive_pollG(ch, surls[i], &errors[i]);

        if (errors[i]) {
            error_count++;
        } else if (ret == 1) {
            ontape_count++;
        }
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, " Archive polling: nbfiles=%d ontape_count=%d error_count=%d",
              nbfiles, ontape_count, error_count);
    gfal2_log(G_LOG_LEVEL_DEBUG, " gfal_srm_archive_poll_listG <-");

    // All files are on tape: return 1
    if (ontape_count == nbfiles) {
        return 1;
    }

    // All files encountered errors: return -1
    if (error_count == nbfiles) {
        return -1;
    }

    // Some files are on tape, others encountered errors
    if (ontape_count + error_count == nbfiles) {
        return 2;
    }

    // Archiving in process: return 0
    return 0;
}
