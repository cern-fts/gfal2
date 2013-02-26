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
 * @file gfal_common_mds_ldap_internal.c
 * @brief file for the internal ldap query by gfal, without is interface
 * @author Adrien Devresse
 * @date 05/09/2011
 * */

#include <errno.h>
#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <pthread.h>
#include <lber.h>
#include <ldap.h>
#include <common/gfal_common_errverbose.h>
#include "gfal_common_mds_ldap_internal.h"
#include "gfal_common_mds_ldap_internal_layer.h"






static char* tabattr[] = {"GlueServiceVersion",  "GlueServiceEndpoint", "GlueServiceType", NULL};
static const char *sbasedn = "o=grid";

static const char* srm_endpoint_filter = "(|(GlueSEUniqueID=*%s*)(&(GlueServiceType=srm*)(GlueServiceEndpoint=*://%s*)))";

static const char* SRM_PREFIX_NAME="SRM";

static pthread_mutex_t mux_init_lap = PTHREAD_MUTEX_INITIALIZER;


LDAP* gfal_mds_ldap_connect(const char* uri, GError** err){
	g_return_val_err_if_fail(uri != NULL, NULL, err, "invalid arg uri");
	LDAP* ld=NULL;
	GError* tmp_err=NULL;
	int rc;

    pthread_mutex_lock(&mux_init_lap); // libldap suffers of a thread-safety bug inside initialize function
	if ( (rc = gfal_mds_ldap.ldap_initialize(&ld, uri)) != LDAP_SUCCESS ) {

		 g_set_error(&tmp_err, 0, ECOMM, "Error with contacting ldap %s : %s", uri, ldap_err2string(rc)); 
	}else{
		gfal_log(GFAL_VERBOSE_VERBOSE, "  Try to bind with the bdii %s", uri);
		struct berval cred= { .bv_val = NULL, .bv_len = 0 };
		if( (rc = gfal_mds_ldap.ldap_sasl_bind_s( ld,  NULL, LDAP_SASL_SIMPLE, &cred, NULL, NULL, NULL)) != LDAP_SUCCESS){
			 g_set_error(&tmp_err, 0, ECOMM, "Error while bind to bdii with %s : %s", uri, ldap_err2string(rc));
			 ld=NULL;
		}
	}
    pthread_mutex_unlock(&mux_init_lap);
    G_RETURN_ERR(ld, tmp_err, err);
}

/*
 *  Execute a ldap query on a connected bdii
 * */
int gfal_mds_ldap_search(LDAP* ld, const char* basedn, const char* filter, char** tabattr, LDAPMessage **res, GError** err){
	GError* tmp_err=NULL;
	int ret = -1;	
	int rc;
	
	if ( ( rc = gfal_mds_ldap.ldap_search_ext_s( ld, basedn, LDAP_SCOPE_SUBTREE, 
	 filter, tabattr, 0, NULL, NULL, LDAP_NO_LIMIT, 
	 LDAP_NO_LIMIT,  res ) ) != LDAP_SUCCESS ) { 
			 g_set_error(&tmp_err, 0, ECOMM, "Error while request %s to bdii : %s", filter, ldap_err2string(rc));
	}else
		ret = 0;
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret;	
   
}

/*
 *  convert bdii returned value to a srm_endpoint struct
 *  @return 0 if success else -1
 * */
static int gfal_mds_srm_endpoint_struct_builder(char* srm_name, char* srm_version, char* srm_endpoint, 
			gfal_mds_endpoint* endpoints, GError** err){
	int ret = 0;	
	GError* tmp_err=NULL;		
    if(strncasecmp(srm_name, SRM_PREFIX_NAME, strlen(SRM_PREFIX_NAME)) != 0){
		g_set_error(&tmp_err, 0, EINVAL, "bad value of srm endpoint returned by bdii : %s, excepted : %s ", srm_name, SRM_PREFIX_NAME);
		ret =-1;
	}else{
		if(strncmp(srm_version, "1.", 2) == 0)
			endpoints->type= SRMv1;
		else if(strncmp(srm_version, "2.", 2) == 0)
			endpoints->type= SRMv2;
		else{
			g_set_error(&tmp_err, 0, EINVAL, "bad value of srm version returned by bdii : %s, excepted 1.x or 2.x ", srm_version);
			ret = -1;
		}
		if(strstr(srm_endpoint, ":/") != NULL)
            g_strlcpy(endpoints->url, srm_endpoint, GFAL_URL_MAX_LEN);
		else{
			g_set_error(&tmp_err, 0, EINVAL, "bad value of srm endpoint returned by bdii : %s, excepted a correct endpoint url ( httpg://, https://, ... ) ", srm_endpoint);
			ret = -1;
		}
			
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret;
}

/*
 *  Analyse attr fields
 *  return > 0 if success, 0 if not it is an empty entry or -1 if error
 * */
static int gfal_mds_convert_entry_to_srm_information(LDAP* ld,LDAPMessage * entry, gfal_mds_endpoint* endpoints, GError** err){
	
	struct berval ** vals;
	char* a;
	int ret = 0;
	GError * tmp_err=NULL;
	BerElement   *ber; 		
    char srm_name[GFAL_URL_MAX_LEN+1]={0};
    char srm_version[GFAL_URL_MAX_LEN+1]={0};
    char srm_endpoint[GFAL_URL_MAX_LEN+1]={0};
				
	for (  a = gfal_mds_ldap.ldap_first_attribute( ld, entry, &ber );  a != NULL;
			a = gfal_mds_ldap.ldap_next_attribute( ld, entry, ber ) ) {  // for each attribute
	 
		if ((vals = gfal_mds_ldap.ldap_get_values_len( ld, entry, a)) != NULL ) { 
			if(strncmp(a, "GlueServiceVersion", GFAL_URL_MAX_LEN) == 0){
				*((char*) mempcpy(srm_version,vals[0]->bv_val, MIN(GFAL_URL_MAX_LEN,vals[0]->bv_len) )) = '\0';
				ret+=1;
			}else if (strncmp(a, "GlueServiceEndpoint", GFAL_URL_MAX_LEN) == 0){
				*((char*) mempcpy(srm_endpoint,vals[0]->bv_val, MIN(GFAL_URL_MAX_LEN,vals[0]->bv_len) )) = '\0';
				ret+=1;
			}else if (strncmp(a, "GlueServiceType", GFAL_URL_MAX_LEN) == 0){
				*((char*) mempcpy(srm_name,vals[0]->bv_val, MIN(GFAL_URL_MAX_LEN,vals[0]->bv_len) )) = '\0';
				ret+=1;
			}else{
				g_set_error(&tmp_err, 0, EINVAL, " Bad attribute retrived from bdii ");
				gfal_mds_ldap.ldap_value_free_len( vals ); 
				gfal_mds_ldap.ldap_memfree( a ); 
				ret = -1;
				break;
			}
			
			gfal_mds_ldap.ldap_value_free_len( vals ); 
		}
		gfal_mds_ldap.ldap_memfree(a);  
	}
	if(ber) 
		gfal_mds_ldap.ber_free(ber,0);
      	
	if(ret > 0)
		ret = (gfal_mds_srm_endpoint_struct_builder(srm_name, srm_version, srm_endpoint, endpoints, &tmp_err) ==0 )?ret:-1;
	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret;						
}


/*
 * 
 * parse the result of a query to get the srm endpoint
 */
int gfal_mds_get_srm_types_endpoint(LDAP* ld, LDAPMessage* result, gfal_mds_endpoint* endpoints, size_t s_endpoint, GError** err){
	GError* tmp_err=NULL;
	int ret = 0;
	int tmp_ret;
	int i=0;

	if( (tmp_ret= gfal_mds_ldap.ldap_count_entries(ld, result)) >= 1 ){
		LDAPMessage* e = gfal_mds_ldap.ldap_first_entry( ld, result );   
		while ( e != NULL && i < s_endpoint ) { 
			/* Iterate through each attribute in the entry. */ 
			if( ( tmp_ret = gfal_mds_convert_entry_to_srm_information(ld, e, &endpoints[i], &tmp_err)) < 0){
				ret = -1;
				break;
			}
			if(tmp_ret > 0){ // if 0 returned, empty field, try again without increment
				i++;
				ret++;
			}
			e = gfal_mds_ldap.ldap_next_entry(ld, e);
		}
	}else if(tmp_ret == -1){
		int myld_errno=0;
		ldap_get_option(ld, LDAP_OPT_RESULT_CODE,&myld_errno);
		g_set_error(&tmp_err, 0, EINVAL, " error returned in ldap results : %s", ldap_err2string(myld_errno));
		ret = -1;		
	}else {
		g_set_error(&tmp_err, 0, ENXIO, " no entries for the endpoint returned by the bdii : %d ", tmp_ret);
		ret = -1;
	}
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret;			
}

/*
 * 
 * get the current ldap URI
 **/
int gfal_mds_get_ldapuri(gfal2_context_t context, char* buff, size_t s_buff, GError** err){
	char *var;
	GError* tmp_err=NULL;
	int ret = -1;
    g_strlcpy(buff, "ldap://", s_buff);
	if((var  = getenv(bdii_env_var)) != NULL){
		g_strlcat(buff, var, s_buff);
		ret = 0;
    }else{
        gchar * internal_bdii_host = gfal2_get_opt_string(context, bdii_config_group, bdii_config_var,NULL);
        gfal_log(GFAL_VERBOSE_TRACE, " use LCG_GFAL_INFOSYS : %s", internal_bdii_host);
        g_strlcat(buff, internal_bdii_host, s_buff);
        g_free(internal_bdii_host);
        ret =0;
    }
    G_RETURN_ERR(ret, tmp_err, err);
}


void gfal_mds_ldap_disconnect(LDAP* ld){
	gfal_mds_ldap.ldap_unbind_ext_s( ld,NULL,NULL ); 
}

/*
 * resolve the SRM endpoint associated with a given base_url with the bdii
 * @param base_url : basic url to resolve
 * @param endpoints : table of gfal_mds_endpoint to set with a size of s_endpoint
 * @param s_endpoint : maximum number of endpoints to set
 * @param err: Gerror system for the report of the errors.
 *  @return : number of endpoints set or -1 if error
 */ 
int gfal_mds_bdii_get_srm_endpoint(gfal2_context_t context, const char* base_url, gfal_mds_endpoint* endpoints, size_t s_endpoint, GError** err){
	int ret =-1;
	GError* tmp_err=NULL;
	char uri[GFAL_URL_MAX_LEN];
	LDAP* ld;
	gfal_log(GFAL_VERBOSE_TRACE, " gfal_mds_bdii_get_srm_endpoint ->");
    if( gfal_mds_get_ldapuri(context, uri, GFAL_URL_MAX_LEN, &tmp_err) >= 0){
		if( (ld = gfal_mds_ldap_connect(uri, &tmp_err)) != NULL){
			LDAPMessage * res;
			char buff_filter[GFAL_URL_MAX_LEN];
			snprintf(buff_filter, GFAL_URL_MAX_LEN, srm_endpoint_filter, base_url, base_url); // construct the request
			if(gfal_mds_ldap_search(ld, sbasedn, buff_filter, tabattr, &res, &tmp_err) >= 0){
				ret = gfal_mds_get_srm_types_endpoint(ld, res, endpoints, s_endpoint, &tmp_err);
				gfal_mds_ldap.ldap_msgfree(res);
			}
			gfal_mds_ldap_disconnect(ld);
		}
	}

	gfal_log(GFAL_VERBOSE_TRACE, " gfal_mds_bdii_get_srm_endpoint <-");	
	if(tmp_err)
		g_propagate_prefixed_error(err, tmp_err, "[%s]",__func__);
	return ret;
}
