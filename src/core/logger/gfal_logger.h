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
 * @file gfal_logger.h
 * @brief log functions 
 * @author Devresse Adrien
 * */

#include <glib.h>
#include <common/gfal_constants.h>


#ifdef __cplusplus
extern "C"
{
#endif 


//! \def GFAL_VERBOSE_NORMAL only errors are printed 
#define GFAL_VERBOSE_NORMAL     0x00   
//! \def GFAL_VERBOSE_VERBOSE a bit more verbose information is printed 
#define GFAL_VERBOSE_VERBOSE    0x01  
//! \def GFAL_VERBOSE_DEBUG  extra information is printed  
#define GFAL_VERBOSE_DEBUG      0x02 
//! \def GFAL_VERBOSE_TRACE execution trace internal to GFAL 2.0
#define GFAL_VERBOSE_TRACE		0x08
//! \def GFAL_VERBOSE_TRACE_PLUGIN log all the plugin related debug information
#define GFAL_VERBOSE_TRACE_PLUGIN 0x04

/**
 * \brief print error message with the gfal2 logger
 */
void gfal_log(int verbose_lvl,const char* msg, ...);

/**
 * \brief set the current log level
 *  log level can be a combinaison of GFAL_VERBOSE_* flags
 */
int gfal_set_verbose (int value);



/**
 * \brief get the current log level
 *  log level can be a combinaison of GFAL_VERBOSE_* flags
 */
int gfal_get_verbose();

/**
 * define a log handler for the gfal messages
 * see Glib 2.0 message logging system for more informations about log_func
 * 
 * internally, GFAL 2.0 use the glib 2.0 log system with the "GFAL2" domain : 
 * usual glib 2.0 functions can be used to control the GFAL 2.0 messages flow.
 **/
guint gfal_log_set_handler(GLogFunc log_func,
                                gpointer user_data);

#ifdef __cplusplus
}
#endif 

