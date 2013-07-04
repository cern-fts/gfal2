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
 
/*
 * @file gfal_common_mds_ldap_internal.ch
 * @brief header for the internal ldap query by gfal, without is interface
 * @author Adrien Devresse
 * @date 05/09/2011
 * */

#include <glib.h>
#include <lber.h>
#include <ldap.h>
#include <common/gfal_common_errverbose.h>
#include "gfal_common_mds.h"



LDAP* gfal_mds_ldap_connect(gfal2_context_t context, const char* uri, GError** err);

int gfal_mds_get_ldapuri(gfal2_context_t context, char* buff, size_t s_buff, GError** err);

int gfal_mds_get_srm_types_endpoint(LDAP* ld, LDAPMessage *result, gfal_mds_endpoint* endpoints, size_t s_endpoint, GError** err);

int gfal_mds_bdii_get_srm_endpoint(gfal2_context_t handle, const char* base_url, gfal_mds_endpoint* endpoints, size_t s_endpoint, GError** err);
