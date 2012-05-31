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

#include <glib.h>
#include <string.h>
#include <stdlib.h>

#include "gfal_common_config.h"
#include "gfal_common_errverbose.h"


void gfal_conf_elem_destroy_callback(gpointer data){
	gfal_conf_elem_t elem = (gfal_conf_elem_t) data;
	gfal_conf_elem_delete(elem);
}

gfal_conf_t gfal_conf_new(){
	gfal_conf_t res = g_new0(struct _gfal_conf, 1);
	//g_mutex_init(&(res->mux));
	res->cont_config =g_hash_table_new_full(g_str_hash, g_str_equal,
											g_free, gfal_conf_elem_destroy_callback);
	return res;
}

void gfal_conf_delete(gfal_conf_t conf){
	if(conf){
	//	g_mutex_clear(&(conf->mux));
		g_hash_table_destroy(conf->cont_config);
		g_free(conf);
	}
}


void gfal_conf_elem_delete(gfal_conf_elem_t elem ){
	if(elem){
		g_key_file_free(elem->key_file);
		g_free(elem);
	}
}
