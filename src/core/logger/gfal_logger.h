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
#ifndef GFAL_LOGGER_H_
#define GFAL_LOGGER_H_

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif


#include <glib.h>
#include <common/gfal_deprecated.h>


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Log a message with the given level.
 * The default handler prints to stderr.
 */
void gfal2_log(GLogLevelFlags level, const char* msg, ...);

/**
 * Log a message with the given level.
 * The default handler prints to stderr.
 */
void gfal2_logv(GLogLevelFlags level, const char* msg, va_list args);


/**
 * Set the log level. Only messages with level higher or equal will be passed to the handler.
 * For instance, if set to G_LOG_LEVEL_WARNING,
 * only messages with level WARNING, CRITICAL and ERROR will be considered.
 */
void gfal2_log_set_level(GLogLevelFlags level);

/**
 * Return the log level configured
 */
GLogLevelFlags gfal2_log_get_level(void);

/**
 * Set a custom handler
 * See Glib2 message logging system for more informations about log_func.
 * Internally, gfal2 uses the glib2 log system with the "GFAL2" domain.
 * Usual glib2 functions can be used to control the gfal2 messages.
 */
int gfal2_log_set_handler(GLogFunc func, gpointer user_data);


#ifdef __cplusplus
}
#endif

#endif /* GFAL_LOGGER_H_ */
