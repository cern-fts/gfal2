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
 * @file gfal_logger.c
 * @brief log functions
 * @author Devresse Adrien
 * */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "gfal_logger.h"


static int gfal_verbose = 0;
static GLogLevelFlags gfal2_log_level = G_LOG_LEVEL_WARNING;



void gfal2_log(GLogLevelFlags level, const char* msg, ...)
{
    if (level <= gfal2_log_level) {
        va_list args;
        va_start(args, msg);
        g_logv("GFAL2", level, msg, args);
        va_end(args);
    }
}


void gfal2_logv(GLogLevelFlags level, const char* msg, va_list args)
{
    if (level <= gfal2_log_level) {
        g_logv("GFAL2", level, msg, args);
    }
}


void gfal2_log_set_level(GLogLevelFlags level)
{
    gfal2_log_level = level;
}


GLogLevelFlags gfal2_log_get_level(void)
{
    return gfal2_log_level;
}


int gfal2_log_set_handler(GLogFunc func, gpointer user_data)
{
    return g_log_set_handler("GFAL2", G_LOG_LEVEL_MASK, func, user_data);
}


/*** DEPRECATED ***/

int gfal_get_verbose(){
	return gfal_verbose;
}


int gfal_set_verbose (int value)
{
    if (value < 0)
        return -1;
    gfal_verbose = value;

    // For compatibility, need to set the new log level
    if ((value & GFAL_VERBOSE_DEBUG) | (value & GFAL_VERBOSE_TRACE) | (value & GFAL_VERBOSE_TRACE_PLUGIN))
        gfal2_log_level = G_LOG_LEVEL_DEBUG;
    else if (value & GFAL_VERBOSE_VERBOSE)
        gfal2_log_level = G_LOG_LEVEL_INFO;
    else
        gfal2_log_level = G_LOG_LEVEL_WARNING;

    return 0;
}


guint gfal_log_set_handler(GLogFunc log_func, gpointer user_data)
{
    // The handler is the same for both legacy and new, only the log level will change
    return gfal2_log_set_handler(log_func, user_data);
}


void gfal_log(int verbose_lvl, const char* msg, ...)
{
    if (verbose_lvl & gfal_verbose) {
        va_list args;
        va_start(args, msg);
        gfal2_logv(G_LOG_LEVEL_MESSAGE, msg, args);
        va_end(args);
    }
}
