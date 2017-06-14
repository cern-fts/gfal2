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
#ifndef GFAL_CONFIG_H_
#define GFAL_CONFIG_H_

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif

#include "gfal_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define CORE_CONFIG_GROUP "CORE"
#define CORE_CONFIG_CHECKSUM_TIMEOUT "CHECKSUM_TIMEOUT"
#define CORE_CONFIG_NAMESPACE_TIMEOUT "NAMESPACE_TIMEOUT"


/**
 * @file gfal_config.h
 * @brief gfal2 configuration API
 * set/get option for the running configuration of GFAL 2.0
 * @author Adrien Devresse
 */

/*!
    \defgroup config_group Parameter API


    Allows to overwrite or/and define any parameter of gfal2.
    A complete list of parameter is accessible in the gfal2 configuration files directory
    ( by default : /etc/gfal2.d/  )

    Example ( enable IPv6 support for the gridFTP plugin ) :
		gfal2_set_opt_boolean("GRIDFTP PLUGIN", "IPV6", true, NULL);

*/

/*!
    \addtogroup config_group
	@{
*/


/**
 * @brief get a string parameter in the current GFAL 2.0 configuration
 *  see gfal2.d configuration files or gfal2 documentation to know group/key/values
 *
 * @param context : context of gfal2
 * @param group_name : group name of the parameter
 * @param key : key of the parameter
 * @param error : GError error report system
 * @return parameter value. Must be freed using g_free
 **/
gchar * gfal2_get_opt_string(gfal2_context_t context, const gchar *group_name,
                                    const gchar *key, GError **error);

/**
* @brief similar to \ref gfal2_get_opt_string but return a default value if
* an error occurs
 *
 * @param handle : context of gfal2
 * @param group_name : group name of the parameter
 * @param key : key of the parameter
 * @param default_value : Default value
 * @return parameter value. Must be freed using g_free
 **/
gchar * gfal2_get_opt_string_with_default(gfal2_context_t handle, const gchar *group_name,
                                    const gchar *key, const gchar* default_value);

/**
 * @brief set a string parameter in the current GFAL 2.0 configuration
 *  see gfal2.d configuration files or gfal2 documentation to know group/key/values
 *
 * @param context : context of gfal2
 * @param group_name : group name of the parameter
 * @param key : key of the parameter
 * @param value : value to set
 * @param error : GError error report system
 * @return parameter value
 **/
gint gfal2_set_opt_string(gfal2_context_t context, const gchar *group_name,
                                    const gchar *key, const gchar* value, GError **error);

/**
 * @brief get an integer parameter in the current GFAL 2.0 configuration
 *  see gfal2.d configuration files or gfal2 documentation to know group/key/values
 *
 * @param context : context of gfal2
 * @param group_name : group name of the parameter
 * @param key : key of the parameter
 * @param error : GError error report system
 * @return parameter value
 **/
gint gfal2_get_opt_integer(gfal2_context_t context, const gchar *group_name,
                                 const gchar *key, GError **error);

/**
* @brief similar to \ref gfal2_get_opt_integer but return a default value if
* an error occurs
*
* @param handle : context of gfal2
* @param group_name : group name of the parameter
* @param key : key of the parameter
* @param default_value : default value returned if not present
* @return parameter value
**/
gint gfal2_get_opt_integer_with_default(gfal2_context_t handle, const gchar *group_name,
                                        const gchar *key, gint default_value);

/**
 * @brief set an integer parameter in the current GFAL 2.0 configuration
 *  see gfal2.d configuration files or gfal2 documentation to know group/key/values
 *
 * @param context : context of gfal2
 * @param group_name : group name of the parameter
 * @param key : key of the parameter
 * @param value : value to set
 * @param error : GError error report system
 * @return parameter value
 **/
gint gfal2_set_opt_integer(gfal2_context_t context, const gchar *group_name,
                                  const gchar *key, gint value,
                                  GError** error);
/**
 * @brief set a boolean parameter in the current GFAL 2.0 configuration
 *  see gfal2.d configuration files or gfal2 documentation to know group/key/values
 *
 * @param context : context of gfal2
 * @param group_name : group name of the parameter
 * @param key : key of the parameter
 * @param value : value to set
 * @param error : GError error report system
 * @return parameter value
 **/
gint gfal2_set_opt_boolean(gfal2_context_t context, const gchar *group_name,
                                  const gchar *key, gboolean value, GError **error);

/**
* @brief get a boolean parameter in the current GFAL 2.0 configuration
*  see gfal2.d configuration files or gfal2 documentation to know group/key/values
*
* @param context : context of gfal2
* @param group_name : group name of the parameter
* @param key : key of the parameter
* @param error : GError error report system
* @return parameter value
**/
gboolean gfal2_get_opt_boolean(gfal2_context_t context, const gchar *group_name,
                                        const gchar *key, GError **error);


/**
* @brief similar to \ref gfal2_get_opt_boolean but return a default value if
* an error occures
*
* @param context : context of gfal2
* @param group_name : group name of the parameter
* @param key : key of the parameter
* @param default_value : default value returned if not present
* @return parameter value
**/
gboolean gfal2_get_opt_boolean_with_default(gfal2_context_t context, const gchar *group_name,
                                           const gchar *key, gboolean default_value);


/**
 * @brief set a list of string parameter in the current GFAL 2.0 configuration
 *  see gfal2.d configuration files or gfal2 documentation to know group/key/values
 *
 * @param context : context of gfal2
 * @param group_name : group name of the parameter
 * @param key : key of the parameter
 * @param list : list of strings
 * @param length : length of the list
 * @param error : GError error report system
 * @return parameter value
 **/
gint gfal2_set_opt_string_list(gfal2_context_t context, const gchar *group_name,
                                     const gchar *key,
                                     const gchar * const list[],
                                     gsize length,
                                     GError ** error);

/**
* @brief get a list of string parameter in the current GFAL 2.0 configuration
*  see gfal2.d configuration files or gfal2 documentation to know group/key/values
*
* @param context : context of gfal2
* @param group_name : group name of the parameter
* @param key : key of the parameter
* @param length : the length of the string is stored here
* @param error : GError error report system
* @return parameter value
**/
gchar ** gfal2_get_opt_string_list(gfal2_context_t context, const gchar *group_name,
                                          const gchar *key, gsize *length, GError **error);

/**
* @brief get a list of string parameter in the current GFAL 2.0 configuration
*  see gfal2.d configuration files or gfal2 documentation to know group/key/values
*
* @param context : context of gfal2
* @param group_name : group name of the parameter
* @param key : key of the parameter
* @param length : the length of the string is stored here
* @param default_value : Default array of not found
* @return parameter value
**/
gchar ** gfal2_get_opt_string_list_with_default(gfal2_context_t context, const gchar *group_name,
                                          const gchar *key, gsize *length, char** default_value);

/**
 * @brief load configuration parameters from the file specified by path
 */
gint gfal2_load_opts_from_file(gfal2_context_t context, const char* path, GError** error);

/**
 * Get all keys defined for the given group_name
 * @param context : context of gfal2
 * @param group_name : group name of the parameters
 * @param length : the number of keys is stored here
 * @param error : GError error report system
 * @return NULL on error. A NULL-terminated array with the list of keys. Use g_strfreev() to free it.
 */
gchar **gfal2_get_opt_keys(gfal2_context_t context, const gchar *group_name, gsize *length, GError **error);


/**
 * Removes a key from the settings
 * @param context : context of gfal2
 * @param group_name : group name of the parameters
 * @param key : key of the parameter
 * @param error : GError error report system
 * @return TRUE if the key was removed, FALSE otherwise
 */
gboolean gfal2_remove_opt(gfal2_context_t context, const gchar *group_name,
    const gchar *key, GError **error);

/**
 * Set the user agent for those protocols that support this
 */
gint gfal2_set_user_agent(gfal2_context_t handle, const char* user_agent,
        const char* version, GError** error);

/**
 * Returns the user agent and version specified before with gfal2_set_user_agent
 * Leave user_agent and version to NULL if not found
 */
gint gfal2_get_user_agent(gfal2_context_t handle, const char** user_agent,
        const char** version);

/**
 * Add a new key/value pair with additional information to be passed to the storage
 * for protocols that support it.
 * For instance, this will be passed via CLIENTINFO for GridFTP, or the ClientInfo header for SRM and HTTP
 * Return < 0 on error
 */
gint gfal2_add_client_info(gfal2_context_t handle, const char* key, const char* value, GError** error);

/**
 * Removes a key/value pair set previously with gfal2_add_client_info
 * Return < 0 on error
 */
gint gfal2_remove_client_info(gfal2_context_t handle, const char* key, GError** error);

/**
 * Clear the client information
 * Return < 0 on error
 */
gint gfal2_clear_client_info(gfal2_context_t handle, GError** error);

/**
 * Return how many custom pairs have been set
 * Return < 0 on error
 */
gint gfal2_get_client_info_count(gfal2_context_t handle, GError** error);

/**
 * Put into key and value the pair at position index, or NULL if it does not exist
 * Return < 0 on error
 */
gint gfal2_get_client_info_pair(gfal2_context_t handle, int index, const char** key,
        const char** value, GError** error);

/**
 * Put into value the value associated with the given key
 * Return < 0 on error
 */
gint gfal2_get_client_info_value(gfal2_context_t handle, const char* key, const char** value, GError** error);

/**
 * For convenience, return all the key/value information in the form
 * key1=value1;key2=value2
 * The return value is NULL if there is no information. Otherwise, use g_free on it when done.
 */
char* gfal2_get_client_info_string(gfal2_context_t handle);

/**
	@}
    End of the FILE group
*/

#ifdef __cplusplus
}
#endif


#endif /* GFAL_CONFIG_H_ */

