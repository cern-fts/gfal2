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
 
/*
 * @file gfal_common_mds_ldap_internal.c
 * @brief file for the internal ldap query by gfal, without is interface
 * @author Adrien Devresse
 * @date 05/09/2011
 * */

#include <errno.h>
#include <stdlib.h>
#include <lber.h>
#include <ldap.h>
#include "gfal_common_mds_ldap_internal_layer.h"



struct _gfal_mds_ldap gfal_mds_ldap={
	.ldap_initialize= ldap_initialize,
	.ldap_sasl_bind_s = ldap_sasl_bind_s,
	.ldap_search_ext_s = ldap_search_ext_s,
	.ldap_unbind_ext_s = ldap_unbind_ext_s,
    .ldap_first_entry= ldap_first_entry,
	.ldap_next_entry= ldap_next_entry,
	.ldap_first_attribute = ldap_first_attribute,
	.ldap_next_attribute = ldap_next_attribute,
	.ldap_get_values_len= ldap_get_values_len,
	.ldap_value_free_len = ldap_value_free_len,
	.ldap_memfree = ldap_memfree,
	.ldap_msgfree = ldap_msgfree,
	.ber_free = ber_free,
	.ldap_count_entries = ldap_count_entries,
	
};
