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

#pragma once
#ifndef GFAL_LOGGER_H
#define GFAL_LOGGER_H

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif

/**
 * @file gfal_logger.h
 * @brief log functions
 * @author Devresse Adrien
 * */

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



/****************************************
 * The following methods are deprecated *
 ****************************************/


//! \def GFAL_VERBOSE_NORMAL only errors are printed
#define GFAL_VERBOSE_NORMAL     0x00    // G_LOG_LEVEL_MESSAGE
//! \def GFAL_VERBOSE_VERBOSE a bit more verbose information is printed
#define GFAL_VERBOSE_VERBOSE    0x01    // G_LOG_LEVEL_INFO
//! \def GFAL_VERBOSE_DEBUG  extra information is printed
#define GFAL_VERBOSE_DEBUG      0x02    // G_LOG_LEVEL_DEBUG
//! \def GFAL_VERBOSE_TRACE execution trace internal to GFAL 2.0
#define GFAL_VERBOSE_TRACE		0x08    // G_LOG_LEVEL_DEBUG
//! \def GFAL_VERBOSE_TRACE_PLUGIN log all the plugin related debug information
#define GFAL_VERBOSE_TRACE_PLUGIN 0x04  // G_LOG_LEVEL_DEBUG

/**
 * \brief print error message with the gfal2 logger
 */
GFAL2_DEPRECATED(gfal2_log) void gfal_log(int verbose_lvl, const char* msg, ...);

/**
 * \brief set the current log level
 *  log level can be a combinaison of GFAL_VERBOSE_* flags
 */
GFAL2_DEPRECATED(gfal2_log_set_level) int gfal_set_verbose (int value);



/**
 * \brief get the current log level
 *  log level can be a combinaison of GFAL_VERBOSE_* flags
 */
GFAL2_DEPRECATED(gfal2_log_get_level) int gfal_get_verbose();

/**
 * define a log handler for the gfal messages
 * see Glib 2.0 message logging system for more informations about log_func
 *
 * internally, GFAL 2.0 use the glib 2.0 log system with the "GFAL2" domain :
 * usual glib 2.0 functions can be used to control the GFAL 2.0 messages flow.
 **/
GFAL2_DEPRECATED(gfal2_log_set_handler)
    guint gfal_log_set_handler(GLogFunc log_func, gpointer user_data);


#ifdef __cplusplus
}
#endif

#endif /* GFAL_LOGGER_H */
