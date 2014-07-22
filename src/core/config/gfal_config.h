#pragma once
#ifndef GFAL_CONFIG_H
#define GFAL_CONFIG_H
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

#include <glib.h>
#include <global/gfal_global.h>

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

/**
 * @file gfal_config.h
 * @brief gfal2 configuration API
 * set/get option for the running configuration of GFAL 2.0
 * @author Adrien Devresse
 */

/*!
    \defgroup config_group Parameter API


    Allows to overwrite or/and define any parameter of gfal 2.0.
    A complete list of parameter is accessible in the gfal 2.0 configuration files directory
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
 *  see gfal2.d configuration files or gfal 2.0 documentation to know group/key/values
 *
 * @param handle : context of gfal 2.0
 * @param group_name : group name of the parameter
 * @param key : key of the parameter
 * @param error : GError error report system
 * @return parameter value
 **/
gchar * gfal2_get_opt_string(gfal2_context_t handle, const gchar *group_name,
                                    const gchar *key, GError **error);


/**
 * @brief set a string parameter in the current GFAL 2.0 configuration
 *  see gfal2.d configuration files or gfal 2.0 documentation to know group/key/values
 *
 * @param handle : context of gfal 2.0
 * @param group_name : group name of the parameter
 * @param key : key of the parameter
 * @param value : value to set
 * @param error : GError error report system
 * @return parameter value
 **/
gint gfal2_set_opt_string(gfal2_context_t handle, const gchar *group_name,
                                    const gchar *key, gchar* value, GError **error);

/**
 * @brief get an integer parameter in the current GFAL 2.0 configuration
 *  see gfal2.d configuration files or gfal 2.0 documentation to know group/key/values
 *
 * @param handle : context of gfal 2.0
 * @param group_name : group name of the parameter
 * @param key : key of the parameter
 * @param error : GError error report system
 * @return parameter value
 **/
gint gfal2_get_opt_integer(gfal2_context_t handle, const gchar *group_name,
                                 const gchar *key, GError **error);

/**
* @brief similar to \ref gfal2_get_opt_integer but return a default value if
* an error occures
*
* @param handle : context of gfal 2.0
* @param group_name : group name of the parameter
* @param key : key of the parameter
* @param default_value : default value returned if not present
* @return parameter value
**/
gint gfal2_get_opt_integer_with_default(gfal2_context_t context, const gchar *group_name,
                                        const gchar *key, gint default_value);

/**
 * @brief set an integer parameter in the current GFAL 2.0 configuration
 *  see gfal2.d configuration files or gfal 2.0 documentation to know group/key/values
 *
 * @param handle : context of gfal 2.0
 * @param group_name : group name of the parameter
 * @param key : key of the parameter
 * @param value : value to set
 * @param error : GError error report system
 * @return parameter value
 **/
gint gfal2_set_opt_integer(gfal2_context_t handle, const gchar *group_name,
                                  const gchar *key, gint value,
                                  GError** error);
/**
 * @brief set a boolean parameter in the current GFAL 2.0 configuration
 *  see gfal2.d configuration files or gfal 2.0 documentation to know group/key/values
 *
 * @param handle : context of gfal 2.0
 * @param group_name : group name of the parameter
 * @param key : key of the parameter
 * @param value : value to set
 * @param error : GError error report system
 * @return parameter value
 **/
gint gfal2_set_opt_boolean(gfal2_context_t handle, const gchar *group_name,
                                  const gchar *key, gboolean value, GError **error);

/**
* @brief get a boolean parameter in the current GFAL 2.0 configuration
*  see gfal2.d configuration files or gfal 2.0 documentation to know group/key/values
*
* @param handle : context of gfal 2.0
* @param group_name : group name of the parameter
* @param key : key of the parameter
* @param error : GError error report system
* @return parameter value
**/
gboolean gfal2_get_opt_boolean(gfal2_context_t handle, const gchar *group_name,
                                        const gchar *key, GError **error);


/**
* @brief similar to \ref gfal2_get_opt_boolean but return a default value if
* an error occures
*
* @param handle : context of gfal 2.0
* @param group_name : group name of the parameter
* @param key : key of the parameter
* @param bool : default value returned if not present
* @return parameter value
**/
gboolean gfal2_get_opt_boolean_with_default(gfal2_context_t handle, const gchar *group_name,
                                           const gchar *key, gboolean default_value);


/**
 * @brief set a list of string parameter in the current GFAL 2.0 configuration
 *  see gfal2.d configuration files or gfal 2.0 documentation to know group/key/values
 *
 * @param handle : context of gfal 2.0
 * @param group_name : group name of the parameter
 * @param key : key of the parameter
 * @param value : value to set
 * @param error : GError error report system
 * @return parameter value
 **/
gint gfal2_set_opt_string_list(gfal2_context_t handle, const gchar *group_name,
                                     const gchar *key,
                                     const gchar * const list[],
                                     gsize length,
                                     GError ** error);

/**
* @brief get a list of string parameter in the current GFAL 2.0 configuration
*  see gfal2.d configuration files or gfal 2.0 documentation to know group/key/values
*
* @param handle : context of gfal 2.0
* @param group_name : group name of the parameter
* @param key : key of the parameter
* @param error : GError error report system
* @return parameter value
**/
gchar ** gfal2_get_opt_string_list(gfal2_context_t handle, const gchar *group_name,
                                          const gchar *key, gsize *length, GError **error);

gchar ** gfal2_get_opt_string_list_with_default(gfal2_context_t handle, const gchar *group_name,
                                          const gchar *key, gsize *length, char** default_value);


/**
 * @brief load configuration parameters from the file specified by path
 */
gint gfal2_load_opts_from_file(gfal2_context_t handle, const char* path, GError** error);


/**
	@}
    End of the FILE group
*/

#ifdef __cplusplus
}
#endif  // __cplusplus


#endif /* GFAL_CONFIG_H */

