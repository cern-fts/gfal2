/*
 * Copyright (c) CERN 2013-2017
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
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

#pragma once
#ifndef GFAL_CANCEL_H_
#define GFAL_CANCEL_H_

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif

#include "gfal_common.h"
#include "gfal_constants.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct gfal_cancel_token_s* gfal_cancel_token_t;
typedef void (*gfal_cancel_hook_cb)(gfal2_context_t context, void* userdata);

/**
 * @brief cancel operation
 *
 * cancel all pending operation on the given context
 * blocking until all operations finish
 * all operations will return and trigger an ECANCELED if interrupted.
 * Thread safe
 * @param context : gfal 2 context
 * @return number of operations canceled
 */
int gfal2_cancel(gfal2_context_t context);

/**
 * @brief cancel status
 * @return true if \ref gfal2_cancel has been called
 *
 * @param context
 * @return true if success
 */
gboolean gfal2_is_canceled(gfal2_context_t context);

/**
 * Register a cancel hook, called in each cancellation
 * Thread-safe
 */
gfal_cancel_token_t gfal2_register_cancel_callback(gfal2_context_t context,
        gfal_cancel_hook_cb cb, void* userdata);

/**
 * Remove a cancel hook
 * Thread-safe
 */
void gfal2_remove_cancel_callback(gfal2_context_t context, gfal_cancel_token_t token);

/**
 * Mark the beginning of a cancellable scope
 */
int gfal2_start_scope_cancel(gfal2_context_t context, GError** err);

/**
 * Mark the end of a cancellable scope
 */
int gfal2_end_scope_cancel(gfal2_context_t context);

/**
 * GQuark of a cancel action
 */
GQuark gfal_cancel_quark();

/** Convenience macro for gfal2_start_scope_cancel */
#define GFAL2_BEGIN_SCOPE_CANCEL(context, ret_err_value, err) \
    do{                                                       \
    if(gfal2_start_scope_cancel(context, err) < 0){           \
        return ret_err_value;                                 \
    }                                                         \
    }while(0)

/** Convenience macro for gfal2_end_scope_cancel */
#define GFAL2_END_SCOPE_CANCEL(context) \
    gfal2_end_scope_cancel(context)

#ifdef __cplusplus
}
#endif


#endif /* GFAL_CANCEL_H_ */
