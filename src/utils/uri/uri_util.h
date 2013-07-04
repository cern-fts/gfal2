#pragma once
#ifndef GFAL_URI_UTIL_H
#define GFAL_URI_UTIL_H
/*
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

#include <glib.h>

#ifdef __cplusplus
extern "C"
{
#endif 

int gfal_hostname_from_uri(const char * uri, char* buff_hostname, size_t s_buff, GError ** err);

/*
  take the first valid error in the arg list and set it in err_out, all the over ones are cleared properly
  last argument must be NULL
  return true if a valid error has been found, else false
*/
gboolean gfal_error_keep_first_err(GError** err_out, ...);


#ifdef __cplusplus
}
#endif 

#endif
