#pragma once
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
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
//! \def GFAL_VERBOSE_TRACE execution trace 
#define GFAL_VERBOSE_TRACE		0x08	

/**
 * \brief print error message with the gfal2 logger
 */
void gfal_print_verbose(int verbose_lvl,const char* msg, ...);

/**
 * \brief set the current log level
 */
int gfal_set_verbose (int value);


/**
 * \brief get the current log level
 */
int gfal_get_verbose();


#ifdef __cplusplus
}
#endif 

