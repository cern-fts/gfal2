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

#include <regex.h>
#include <time.h>
#include <string.h>
#include "gfal_rfio_plugin_layer.h"
#include "gfal_rfio_plugin_main.h"
#include "gfal_rfio_plugin_bindings.h"

gboolean gfal_rfio_check_url(plugin_handle, const char* url,  plugin_mode mode, GError** err);
gboolean gfal_rfio_internal_check_url(gfal_plugin_rfio_handle rh, const char* surl, GError** err);
const char* gfal_rfio_getName();
void gfal_rfio_destroyG(plugin_handle handle);


int gfal_rfio_regex_compile(regex_t * rex, GError** err){
	int ret = regcomp(rex, "^rfio://([:alnum:]|-|/|.|_)+$",REG_ICASE | REG_EXTENDED);
	g_return_val_err_if_fail(ret==0,-1,err,"[gfal_rfio_internal_check_url] fail to compile regex, report this bug");
	return ret;
}


// RFIO plugin GQuark
GQuark gfal2_get_plugin_rfio_quark(){
    return g_quark_from_static_string(GFAL2_QUARK_PLUGINS "::RFIO");
}


/*
 * Init function, called before all
 * */
gfal_plugin_interface gfal_plugin_init(gfal2_context_t handle, GError** err){
	gfal_plugin_interface rfio_plugin;
	GError* tmp_err=NULL;
	memset(&rfio_plugin,0,sizeof(gfal_plugin_interface));	// clear the plugin
	gfal_plugin_rfio_handle h = g_new(struct _gfal_plugin_rfio_handle,1);
	h->handle = handle;
	h->rf = gfal_rfio_internal_loader(&tmp_err);
	gfal_rfio_regex_compile(&h->rex, err);
	rfio_plugin.plugin_data = (void*) h;
	rfio_plugin.check_plugin_url = &gfal_rfio_check_url;
	rfio_plugin.getName= &gfal_rfio_getName;
	rfio_plugin.plugin_delete= &gfal_rfio_destroyG;
	rfio_plugin.openG= &gfal_rfio_openG;
	rfio_plugin.closeG= &gfal_rfio_closeG;
	rfio_plugin.readG= &gfal_rfio_readG;
	rfio_plugin.writeG= &gfal_rfio_writeG;
	rfio_plugin.lseekG = &gfal_rfio_lseekG;
	rfio_plugin.statG = &gfal_rfio_statG;
	rfio_plugin.lstatG= &gfal_rfio_lstatG;
	rfio_plugin.opendirG = &gfal_rfio_opendirG;
	rfio_plugin.readdirG = &gfal_rfio_readdirG;
	rfio_plugin.closedirG = &gfal_rfio_closedirG;
	if(tmp_err)
		gfal2_propagate_prefixed_error(err, tmp_err, __func__);
	return rfio_plugin;
}


gboolean gfal_rfio_internal_check_url(gfal_plugin_rfio_handle rh, const char* surl, GError** err){
	if(surl == NULL || strnlen(surl, GFAL_URL_MAX_LEN) == GFAL_URL_MAX_LEN){
        gfal2_set_error(err, gfal2_get_plugin_rfio_quark(), EINVAL, __func__,
                "Invalid surl, surl too long or NULL");
		return FALSE;
	}
	int ret=  regexec(&rh->rex,surl,0,NULL,0);
	return (ret==0)?TRUE:FALSE;
}


/*
 *  Check the rfio url in the gfal module way
 * */
gboolean gfal_rfio_check_url(plugin_handle ch, const char* url,  plugin_mode mode, GError** err){
	int ret;
	GError* tmp_err=NULL;
	gfal_plugin_rfio_handle rh = (gfal_plugin_rfio_handle) ch;
	switch(mode){
			case GFAL_PLUGIN_OPEN:
			case GFAL_PLUGIN_STAT:
			case GFAL_PLUGIN_LSTAT:
			case GFAL_PLUGIN_OPENDIR:
				ret = gfal_rfio_internal_check_url(rh, url, &tmp_err);
				break;
			default:
				ret =  FALSE;
				break;
	}
	if(tmp_err)
		gfal2_propagate_prefixed_error(err, tmp_err, __func__);
	return ret;
}

void gfal_rfio_destroyG(plugin_handle handle){
	gfal_plugin_rfio_handle h = (gfal_plugin_rfio_handle) handle;
	g_free(h->rf);
	regfree(&h->rex);
	g_free(h);
}

