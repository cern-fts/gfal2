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
  * @brief  header file for the bdii request part of gfal
  * @author : Devresse Adrien
  * @version 2.0.0
  * @date 18/04/2011
  * */
#define _GNU_SOURCE

#define GFAL_MDS_MAX_SRM_ENDPOINT 100

#include <glib.h>
#include "../gfal_prototypes.h"
#include "../gfal_types.h"

#ifndef MDS_BDII_EXTERNAL
#define MDS_BDII_EXTERNAL 0
#endif

typedef enum { SRMv2=0, SRMv1, WebDav } mds_type_endpoint;

/**
 * @struct gfal_mds_endpoint
 * represente an endpoint URL and its type
 */
typedef struct _gfal_mds_endpoint{
	char url[GFAL_URL_MAX_LEN];
	mds_type_endpoint type;
} gfal_mds_endpoint;

extern const char* bdii_env_var;

// 
int gfal_mds_resolve_srm_endpoint(const char* base_url, gfal_mds_endpoint* endpoints, size_t s_endpoint, GError** err);

// deprecated, use gfal_mds_resolve_srm_endpoint instead
int gfal_mds_get_se_types_and_endpoints(const char *host, char ***se_types, char ***se_endpoints, GError** err);

char * gfal_get_lfchost_bdii(gfal_handle handle, GError** err);


void gfal_set_nobdiiG(gfal_handle handle, gboolean no_bdii_chk);

gboolean gfal_get_nobdiiG(gfal_handle handle);

void gfal_mds_set_infosys(gfal_handle handle, const char * infosys, GError** err);
 
 
 
