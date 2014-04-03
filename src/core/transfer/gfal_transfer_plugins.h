#pragma once
#ifndef GFAL_TRANSFER_PLUGIN_H
#define GFAL_TRANSFER_PLUGIN_H

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

/**
 * @file gfal_transfer_plugins.h
 *  gfal API for file transfers of the gfal2_transfer shared library.
 *  This API provides specials functions calls reserved for the gfals plugins
 *  @author Adrien Devresse 
 */

#include <transfer/gfal_transfer_types.h>
#include <transfer/gfal_transfer.h>


#ifdef __cplusplus
#include <exceptions/gfalcoreexception.hpp>

namespace Gfal {

class TransferException: public CoreException {
public:
    std::string side;
    std::string note;

    TransferException(GQuark scope, const std::string & msg, int code,
            const std::string & side, const std::string & note = std::string()):
                CoreException(scope, msg, code), side(side), note(note)
    {
    }

    TransferException(const Glib::Quark & scope, const std::string & msg, int code,
            const std::string & side, const std::string & note = std::string()):
                CoreException(scope, msg, code), side(side), note(note)
    {
    }

    virtual ~TransferException() throw()
    {
    }
};

}

#endif

 
#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus


/// Convenience method for event callback
/// @param params The transfer parameters.
/// @param domain The plugin domain.
/// @param side   The side that triggered the change, if any.
/// @param stage  The new stage.
/// @param fmt    A format string for a message
int plugin_trigger_event(gfalt_params_t params, GQuark domain,
                         gfal_event_side_t side, GQuark stage,
                         const char* fmt, ...);

/// Convenience error methods for copy implementations
void gfalt_propagate_prefixed_error(GError **dest, GError *src, const gchar *function, const gchar *side, const gchar *note);

void gfalt_set_error(GError **err, GQuark domain, gint code, const gchar *function,
        const char *side, const gchar *note, const gchar *format, ...) G_GNUC_PRINTF (7, 8);

#define GFALT_ERROR_SOURCE      "SOURCE"
#define GFALT_ERROR_DESTINATION "DESTINATION"
#define GFALT_ERROR_TRANSFER    "TRANSFER"
#define GFALT_ERROR_CHECKSUM    "CHECKSUM"
#define GFALT_ERROR_EXISTS      "EXISTS"
#define GFALT_ERROR_OVERWRITE   "OVERWRITE"
#define GFALT_ERROR_PARENT      "MAKE_PARENT"

//
// full list of functions that are re-searched by GFAL 2.0 in the plugins
//

/** prototype for the url_check entry point : this entry point is mandatory !!!
 *
 * */
typedef int(*plugin_url_check2_call)(plugin_handle, gfal_context_t, const char* src, const char* dst, gfal_url2_check check);

// prototype for the filecopy entry point in the plugins
typedef int (*plugin_filecopy_call)(plugin_handle, gfal2_context_t, gfalt_params_t, const char* src, const char* dst, GError** );


typedef const char * (*plugin_name_call)();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_GFAL2_TRANSFER_

