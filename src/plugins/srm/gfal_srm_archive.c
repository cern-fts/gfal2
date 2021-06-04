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

#include <uri/gfal2_uri.h>

#include "gfal_srm.h"
#include "gfal_srm_archive.h"
#include "gfal_srm_namespace.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_url_check.h"
#include "gfal_srm_internal_ls.h"


static int gfal_srm_archive_internal_status_index(int nresponses, struct srmv2_mdfilestatus* statuses,
                                                  const char* surl)
{
    GError* tmp_err = NULL;
    int index = -1;
    int i;

    // Extract only path from surl
    // (srmv2_mdfilestatus structure keeps only the path)
    gfal2_uri* parsed = gfal2_parse_uri(surl, &tmp_err);

    if (tmp_err != NULL) {
        g_clear_error(&tmp_err);
        return -1;
    }

    for (i = 0; i < nresponses; i++) {
        if (strcmp(statuses[i].surl, parsed->path) == 0) {
            index = i;
            break;
        }
    }

    gfal2_free_uri(parsed);
    return index;
}

int gfal_srm_archive_pollG(plugin_handle ch, const char* surl, GError** err)
{
    GError* errors[1] = {NULL};
    const char* const surls[1] = {surl};
    int ret;

    g_return_val_err_if_fail(ch && surl, EINVAL, err,
                             "[gfal_srm_archive_pollG] Invalid value handle and/or surl");

    gfal2_log(G_LOG_LEVEL_DEBUG, " gfal_srm_archive_pollG ->");
    ret = gfal_srm_archive_poll_listG(ch, 1, surls, errors);

    gfal2_log(G_LOG_LEVEL_DEBUG, " gfal_srm_archive_pollG <-");
    G_RETURN_ERR(ret, errors[0], err);
}


int gfal_srm_archive_poll_listG(plugin_handle ch, int nbfiles, const char* const* surls, GError** errors)
{
    int error_count = 0;
    int ontape_count = 0;
    int i;

    GError* tmp_err = NULL;
    gfal_srmv2_opt* opts = (gfal_srmv2_opt *) ch;
    gfal_srm_easy_t easy = gfal_srm_ifce_easy_context(opts, *surls, &tmp_err);

    if (nbfiles <= 0) {
        return 1;
    }

    if (!(ch && surls && easy)) {
        for (i = 0 ; i < nbfiles; i++) {
            gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(), EINVAL, __func__,
                            "Invalid value handle and/or surls array");
        }
        g_error_free(tmp_err);
        return -1;
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, "gfal_srm_archive_poll_listG ->");

    struct srm_ls_input input;
    struct srm_ls_output output;
    struct srmv2_mdfilestatus* srmv2_mdstatuses = NULL;

    int nresponses;
    char slocality[64];
    char* surls_decoded[nbfiles];
    gboolean bad_response = FALSE;

    for (i = 0; i < nbfiles; i++) {
        surls_decoded[i] = gfal2_srm_get_decoded_path(surls[i]);
    }

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.nbfiles = nbfiles;
    input.surls = (char **) surls_decoded;

    srm_context_t srm_context = easy->srm_context;

    if ((nresponses = gfal_srm_external_call.srm_ls(srm_context, &input, &output)) < 0) {
        gfal_srm_report_error(srm_context->errbuf, &tmp_err);
        for (i = 0; i < nbfiles ; i++) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
        bad_response = TRUE;
    } else if (nresponses != nbfiles) {
        for (i = 0; i < nbfiles; i++) {
            gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(), ENOMSG, __func__,
                            "Number of files in the request doest not match!");
        }
        bad_response = TRUE;
    }

    srmv2_mdstatuses = output.statuses;

    for (i = 0; i < nbfiles && !bad_response; i++) {
        int status_index = gfal_srm_archive_internal_status_index(nresponses, srmv2_mdstatuses, surls_decoded[i]);
        if (status_index >= 0) {
            if (srmv2_mdstatuses[status_index].status != 0) {
                gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(),
                                srmv2_mdstatuses[status_index].status, __func__,
                                "Error reported from srm-ifce: %d %s",
                                srmv2_mdstatuses[status_index].status,
                                srmv2_mdstatuses[status_index].explanation);
                error_count++;
            } else {
                switch (srmv2_mdstatuses[status_index].locality) {
                    case GFAL_LOCALITY_NEARLINE_:
                    case GFAL_LOCALITY_ONLINE_USCOREAND_USCORENEARLINE:
                        ontape_count++;
                        break;
                    case GFAL_LOCALITY_ONLINE_:
                        gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(), EAGAIN, __func__,
                                        "File %s is not yet archived", surls_decoded[i]);
                        break;
                    default:
                        gfal_srm_status_copy(srmv2_mdstatuses[status_index].locality, slocality, sizeof(slocality));
                        gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(), EINVAL, __func__,
                                        "Unrecognized file status: %s", slocality);
                        error_count++;
                        break;
                }
            }
        } else {
            gfal2_set_error(&errors[i], gfal2_get_plugin_srm_quark(), EPROTO, __func__,
                            "Could not process or missing surl from the response: %s", surls_decoded[i]);
            error_count++;
        }
    }

    gfal_srm_ls_memory_management(&input, &output);
    for (i = 0; i < nbfiles; i++) {
        g_free(surls_decoded[i]);
    }

    // Return on error only here to allow memory clean-up
    if (bad_response) {
        return -1;
    }

    gfal_srm_ifce_easy_context_release(opts, easy);
    gfal2_log(G_LOG_LEVEL_DEBUG, "Archive polling: nbfiles=%d ontape_count=%d error_count=%d",
              nbfiles, ontape_count, error_count);
    gfal2_log(G_LOG_LEVEL_DEBUG, "gfal_srm_archive_poll_listG <-");

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
