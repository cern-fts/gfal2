#pragma once
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
 * @file gfal_common_err_helpers.h
 * @brief error management and verbose display
 * @author Devresse Adrien
 * */

#include <errno.h>
#include <glib.h>

#if (GLIB_CHECK_VERSION(2,16,0) != TRUE)
#include "future/glib.h"
#endif

/** @def macro for error error report on args
 *
 */
#define g_return_val_err_if_fail(exp, val, err, msg) if(!(exp)){ g_set_error(err, gfal2_get_core_quark(), EINVAL, msg); return val; }

/** @def macro for one-line return with error management exception-like
 */
#define G_RETURN_ERR(ret, tmp_err, err) \
if(tmp_err)\
	g_propagate_prefixed_error(err, tmp_err, "[%s]", __func__);\
return ret
