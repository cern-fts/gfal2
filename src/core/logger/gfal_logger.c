/*
 * Copyright (c) CERN 2013-2015
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "gfal_logger.h"


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

