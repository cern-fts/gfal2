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
 * @file gfal_common_mds_ldap_internal_layer.h
 * @brief interface for mock purpose
 * @author Adrien Devresse
 * @date 05/09/2011
 * */

#include <lber.h>
#include <ldap.h>

struct _gfal_mds_ldap{	
	int (*ldap_initialize)(LDAP **ldp, const char *uri);
	
    int (*ldap_sasl_bind_s)(LDAP *ld, const char *dn, const char *mechanism,
              struct berval *cred, LDAPControl *sctrls[],
              LDAPControl *cctrls[], struct berval **servercredp);
              
	 int (*ldap_search_ext_s)(
				LDAP			*ld,
				LDAP_CONST char	*base,
				int				scope,
				LDAP_CONST char	*filter,
				char			**attrs,
				int				attrsonly,
				LDAPControl		**serverctrls,
				LDAPControl		**clientctrls,
				struct timeval	*timeout,
				int				sizelimit,
				LDAPMessage		**res );

	int (*ldap_unbind_ext_s) (
		LDAP			*ld,
		LDAPControl		**serverctrls,
		LDAPControl		**clientctrls);
	
	LDAPMessage* (*ldap_first_entry)( LDAP *ld, LDAPMessage *result );

	LDAPMessage* (*ldap_next_entry)( LDAP *ld, LDAPMessage *entry );
	
	int (*ldap_count_entries)( LDAP *ld, LDAPMessage *result );

	char* (*ldap_first_attribute)(
			LDAP *ld, LDAPMessage *entry, BerElement **berptr );

	char* (*ldap_next_attribute)(
		  LDAP *ld, LDAPMessage *entry, BerElement *ber );
		  
    struct berval **(*ldap_get_values_len)(LDAP *ld, LDAPMessage *entry, const char *attr);


	void (*ldap_value_free_len) ( struct berval **vals );
 
	void (*ldap_memfree)(void * p);

	int (*ldap_msgfree)( LDAPMessage *msg );
	
	void (*ber_free)(BerElement *ber, int freebuf);

	
};

extern struct _gfal_mds_ldap gfal_mds_ldap;
