/* 
* Copyright @ Members of the EMI Collaboration, 2013.
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
 * @file gfal_srm_bringonline.c
 * @brief brings online functions layer from srm
 * @author Devresse Adrien, Alejandro Álvarez Ayllón
 * @version 2.0
 * @date 19/12/2011
 * */
 
 

#include <common/gfal_common_internal.h>
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_plugin.h>
#include <regex.h>
#include <time.h> 

#include "gfal_srm.h"
#include "gfal_srm_endpoint.h"
#include "gfal_srm_internal_layer.h"
#include "gfal_srm_request.h"



static int gfal_srmv2_bring_online_internal(gfal_srmv2_opt* opts, const char* endpoint,
                                            const char* surl,
                                            time_t pintime, time_t timeout,
                                            char* token, size_t tsize,
                                            int async,
                                            GError** err){
    struct srm_bringonline_input  input;
    struct srm_bringonline_output output;
    GError                       *tmp_err = NULL;
    gfal_srm_params_t             params = gfal_srm_params_new(opts, &tmp_err);
    int                           status = 0;

    memset(&output, 0, sizeof(output));

    if (params != NULL) {
        char          error_buffer[2048];
        srm_context_t context = gfal_srm_ifce_context_setup(opts->handle, endpoint, error_buffer, sizeof(error_buffer), &tmp_err);

        if (context) {
            context->timeout      = timeout;
            context->timeout_conn = timeout;
            context->timeout_ops  = timeout;

            input.nbfiles        = 1;
            input.surls          = (char**)&surl;
            input.desiredpintime = pintime;
            input.protocols      = gfal_srm_params_get_protocols(params);
            input.spacetokendesc = gfal_srm_params_get_spacetoken(params);

            int ret = 0;

            if (async)
                ret = gfal_srm_external_call.srm_bring_online_async(context, &input, &output);
            else
                ret = gfal_srm_external_call.srm_bring_online(context, &input, &output);

            if (ret < 0) {
                gfal_srm_report_error(context->errbuf, &tmp_err);
            }
            else {
                status = output.filestatuses[0].status;
                switch (status) {
                    case 0:
                    case 22: // SRM_FILE_PINNED
                        if (output.token)
                            strncpy(token, output.token, tsize);
                        else
                            token[0] = '\0';
                        break;
                    default:
                        g_set_error(&tmp_err, 0,
                                    output.filestatuses[0].status,
                                    " error on the bring online request : %s ",
                                    output.filestatuses[0].explanation);
                        break;
                }
            }
            gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(output.filestatuses, ret);
            gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);
            free(output.token);

            gfal_srm_ifce_context_release(context);
        }
    }
    gfal_srm_params_free(params);

    if (tmp_err != NULL) {
        g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
        return -1;
    }
    else {
        return status == 0;
    }
}



int gfal_srmv2_bring_onlineG(plugin_handle ch, const char* surl,
                             time_t pintime, time_t timeout,
                             char* token, size_t tsize,
                             int async,
                             GError** err){
    gfal_srmv2_opt*     opts = (gfal_srmv2_opt*) ch;
    char                full_endpoint[GFAL_URL_MAX_LEN];
    enum gfal_srm_proto srm_type;
    int                 ret     = 0;
    GError             *tmp_err = NULL;


    ret = gfal_srm_determine_endpoint(opts, surl, full_endpoint,
                                      sizeof(full_endpoint),
                                      &srm_type, &tmp_err);
    if (ret >= 0) {
        switch (srm_type) {
        case PROTO_SRMv2:
            ret = gfal_srmv2_bring_online_internal(opts, full_endpoint, surl,
                                                   pintime, timeout,
                                                   token, tsize,
                                                   async,
                                                   &tmp_err);
            break;
        case PROTO_SRM:
            g_set_error(&tmp_err, 0, EPROTONOSUPPORT, "support for SRMv1 is removed in 2.0, failure");
            break;
        default:
            g_set_error(&tmp_err, 0, EPROTONOSUPPORT, "Unknow version of the protocol SRM , failure");
            break;
        }
    }

    if (tmp_err != NULL) {
        g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
        return -1;
    }
    else {
        return ret;
    }
}



static int gfal_srmv2_bring_online_poll_internal(gfal_srmv2_opt* opts, const char* endpoint,
                                                 const char* surl, const char* token,
                                                 GError ** err)
{
    struct srm_bringonline_input  input;
    struct srm_bringonline_output output;
    char                          error_buffer[1024];
    GError                       *tmp_err = NULL;
    int                           status = 0;

    memset(&input, 0, sizeof(input));
    memset(&output, 0, sizeof(output));

    input.nbfiles = 1;
    input.surls   = (char**)&surl;
    output.token  = (char*)token;

    srm_context_t context = gfal_srm_ifce_context_setup(opts->handle, endpoint,
                                                        error_buffer, sizeof(error_buffer),
                                                        &tmp_err);
    if (context) {
        int ret = gfal_srm_external_call.srm_bring_online_status(context, &input, &output);
        if (ret < 0) {
            gfal_srm_report_error(context->errbuf, &tmp_err);
        }
        else {
            status = output.filestatuses[0].status;
            switch (status) {
                case 0:
                case 22:
                    break;
                default:
                    g_set_error(&tmp_err, 0,
                                output.filestatuses[0].status,
                                " error on the bring online request : %s ",
                                output.filestatuses[0].explanation);
                    break;
            }
        }
        gfal_srm_external_call.srm_srmv2_pinfilestatus_delete(output.filestatuses, ret);
        gfal_srm_external_call.srm_srm2__TReturnStatus_delete(output.retstatus);
        gfal_srm_ifce_context_release(context);
    }

    if (tmp_err != NULL) {
        g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
        return -1;
    }
    else {
        return status == 0;
    }
}



int gfal_srmv2_bring_online_pollG(plugin_handle ch, const char* surl,
                                  const char* token, GError** err)
{
    gfal_srmv2_opt*     opts = (gfal_srmv2_opt*) ch;
    char                full_endpoint[GFAL_URL_MAX_LEN];
    enum                gfal_srm_proto srm_type;
    GError             *tmp_err = NULL;
    int                 ret;

    ret = gfal_srm_determine_endpoint(opts, surl, full_endpoint,
                                      sizeof(full_endpoint),
                                      &srm_type, &tmp_err);

    if (ret >= 0) {
          switch (srm_type) {
          case PROTO_SRMv2:
              ret = gfal_srmv2_bring_online_poll_internal(opts, full_endpoint,
                                                          surl, token,
                                                          &tmp_err);
              break;
          case PROTO_SRM:
              g_set_error(&tmp_err, 0, EPROTONOSUPPORT, "support for SRMv1 is removed in 2.0, failure");
              break;
          default:
              g_set_error(&tmp_err, 0, EPROTONOSUPPORT, "Unknow version of the protocol SRM , failure");
              break;
          }
    }

    if (tmp_err != NULL) {
        g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
        return -1;
    }
    else {
        return ret;
    }
}



static int gfal_srmv2_release_file_internal(gfal_srmv2_opt* opts, const char* endpoint,
                                            const char* surl, const char* token,
                                            GError** err)
{
    struct srm_releasefiles_input input;
    struct srmv2_filestatus      *statuses;
    GError                       *tmp_err = NULL;
    gfal_srm_params_t             params = gfal_srm_params_new(opts, &tmp_err);

    if (params != NULL) {
          char          error_buffer[2048];
          srm_context_t context = gfal_srm_ifce_context_setup(opts->handle, endpoint, error_buffer, sizeof(error_buffer), &tmp_err);

          if (token)
              gfal_log(GFAL_VERBOSE_VERBOSE, "Release file with token %s", token);
          else
              gfal_log(GFAL_VERBOSE_VERBOSE, "Release file without token");

          // Perform
          if (context ) {
              input.nbfiles  = 1;
              input.reqtoken = NULL;
              input.surls    = (char**)&surl;
	      if(token)
              	input.reqtoken = (char*)token;

              int ret = gfal_srm_external_call.srm_release_files(context, &input, &statuses);

              if (ret < 0) {
                  gfal_srm_report_error(context->errbuf, &tmp_err);
              }
              else {
                  if (statuses[0].status != 0) {
                      g_set_error(&tmp_err, 0,
                                  statuses[0].status,
                                  "error on the release request : %s ",
                                  statuses[0].explanation);
                  }
                  gfal_srm_external_call.srm_srmv2_filestatus_delete(statuses, 1);
              }
          }
          else {
              g_set_error(&tmp_err, 0, errno,
                          "[%s] %s", __func__, error_buffer);
          }
    }

    if (tmp_err != NULL) {
        g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
        return -1;
    }
    else {
        return 0;
    }
}



int gfal_srmv2_release_fileG(plugin_handle ch, const char* surl,
                             const char* token, GError** err)
{
      gfal_srmv2_opt*     opts = (gfal_srmv2_opt*) ch;
      char                full_endpoint[GFAL_URL_MAX_LEN];
      enum                gfal_srm_proto srm_type;
      GError             *tmp_err = NULL;
      int                 ret;

      ret = gfal_srm_determine_endpoint(opts, surl, full_endpoint,
                                        sizeof(full_endpoint),
                                        &srm_type, &tmp_err);

      if (ret >= 0) {
            switch (srm_type) {
            case PROTO_SRMv2:
                ret = gfal_srmv2_release_file_internal(opts, full_endpoint,
                                                       surl, token,
                                                       &tmp_err);
                break;
            case PROTO_SRM:
                g_set_error(&tmp_err, 0, EPROTONOSUPPORT, "support for SRMv1 is removed in 2.0, failure");
                break;
            default:
                g_set_error(&tmp_err, 0, EPROTONOSUPPORT, "Unknow version of the protocol SRM , failure");
                break;
            }
      }

      if (tmp_err != NULL) {
          g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);
          return -1;
      }
      else {
          return ret;
      }
}
