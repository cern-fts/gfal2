#include <glibmm.h>
#include <gfal_api.h>
#include <common/gfal_common_plugin_interface.h>
#include "gfal_transfer_internal.h"

static Glib::Quark scope_local_copy("FileCopy::local_copy");
const size_t DEFAULT_BUFFER_SIZE = 200000;


void perform_local_copy(gfal2_context_t context, gfalt_params_t params,
        const std::string & src, const std::string & dst)
{
    gfal_log(GFAL_VERBOSE_TRACE, " -> Gfal::Transfer::start_local_copy ");
    GError * tmp_err_src = NULL;
    GError * tmp_err_dst = NULL;
    GError * tmp_err_out = NULL;

    gfal_file_handle f_src = NULL;
    gfal_file_handle f_dst = NULL;

    gfal_log(GFAL_VERBOSE_TRACE, " open src file : %s ", src.c_str());
    f_src = gfal_plugin_openG(context, src.c_str(), O_RDONLY, 0, &tmp_err_src);

    if (!tmp_err_src) {
        gfal_log(GFAL_VERBOSE_TRACE, "  open dst file : %s ", dst.c_str());
        f_dst = gfal_plugin_openG(context, dst.c_str(), O_WRONLY | O_CREAT, 0755, &tmp_err_dst);

        if (!tmp_err_dst) {
            const time_t timeout = time(NULL) + gfalt_get_timeout(params, NULL);
            ssize_t s_file = 1;
            char buff[DEFAULT_BUFFER_SIZE];

            gfal_log(GFAL_VERBOSE_TRACE,
                    "  begin local transfer %s ->  %s with buffer size %ld",
                    src.c_str(), dst.c_str(), sizeof(buff));

            while (s_file > 0 && !tmp_err_src && !tmp_err_dst) {
                s_file = gfal_plugin_readG(context, f_src, buff, sizeof(buff), &tmp_err_src);
                if (s_file > 0 && !tmp_err_src)
                    gfal_plugin_writeG(context, f_dst, buff, s_file, &tmp_err_dst);

                // Make sure we don't have to cancel
                if (gfal2_is_canceled(context)) {
                    g_set_error(&tmp_err_out, scope_local_copy.id(),
                    ECANCELED, "Transfer canceled");
                    break;
                }
                // Timed-out?
                else if (time(NULL) >= timeout) {
                    g_set_error(&tmp_err_out, scope_local_copy.id(),
                    ETIMEDOUT, "Transfer canceled because the timeout expired");
                    break;
                }
            }

            gfal_plugin_closeG(context, f_dst, (tmp_err_dst) ? NULL : (&tmp_err_dst));
        }
        gfal_plugin_closeG(context, f_src, (tmp_err_src) ? NULL : (&tmp_err_src));
    }

    if (tmp_err_src) {
        g_set_error(&tmp_err_out, scope_local_copy.id(), tmp_err_src->code,
                "Local transfer error on SRC %s : %s", src.c_str(),
                tmp_err_src->message);
        g_clear_error(&tmp_err_src);
    }
    else if (tmp_err_dst) {
        g_set_error(&tmp_err_out, scope_local_copy.id(), tmp_err_dst->code,
                "Local transfer error on DST %s : %s", dst.c_str(),
                tmp_err_dst->message);
        g_clear_error(&tmp_err_dst);
    }
    if (tmp_err_out)
        throw Glib::Error(tmp_err_out);

    gfal_log(GFAL_VERBOSE_TRACE, " <- Gfal::Transfer::start_local_copy ");
}
