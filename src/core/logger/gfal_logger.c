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


const char* gfal2_log_prefix = "GFAL2";

/*
 * Verbose level
 *   API mode (no messages on stderr) by default
 *   CLI has to set it to '0' to get normal error messages
 */
static int gfal_verbose = 0;


/**
 * \brief return verbose mode level
 */
int gfal_get_verbose(){
	return gfal_verbose;
}

/**
 * set the verbose level of gfal 2
 */
int gfal_set_verbose (int value)
{
    if (value < 0)
        return (-1);
    gfal_verbose = value;
    return (0);
}

void gfal_internal_logger(const int verbose_lvl, const char* msg, va_list args){
	GLogLevelFlags log_level=G_LOG_LEVEL_MESSAGE;
	g_logv(gfal2_log_prefix, log_level, msg, args);
}


guint gfal_log_set_handler(GLogFunc log_func,
                                gpointer user_data){
	return g_log_set_handler (gfal2_log_prefix, G_LOG_LEVEL_MASK, log_func, user_data);					 
}

/**
 * \brief display a verbose message 
 * 
 * msg is displayed if current verbose level is superior to verbose mode specified
 * 
 */
void gfal_log(int verbose_lvl, const char* msg, ...)
{
    if (verbose_lvl & gfal_get_verbose()) {
        va_list args;
        va_start(args, msg);
        gfal_internal_logger(verbose_lvl, msg, args);
        va_end(args);
    }

}

