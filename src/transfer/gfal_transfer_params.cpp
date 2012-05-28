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

#include <transfer/gfal_transfer_types_internal.h>
#include <transfer/gfal_cpp_wrapper.h>
#include <cerrno>

const Glib::Quark scope_transfer_param("Gfal::Transfer::Params");

// external C bindings
extern "C" {
	
void gfalt_params_handle_init(gfalt_params_t p, GError ** err){
	p->callback = NULL;
	p->lock = false;
	p->nb_data_streams = GFALT_DEFAULT_NB_STREAM;
	p->timeout = GFALT_DEFAULT_TRANSFERT_TIMEOUT;
	p->start_offset = 0;
	uuid_clear(p->uuid);		
}

gfalt_params_t gfalt_params_handle_new(GError ** err){
	
	gfalt_params_t p = g_new(struct _gfalt_params_t,1); 
	gfalt_params_handle_init(p, err);
	return p;
}


void gfalt_params_handle_delete(gfalt_params_t params, GError ** err){
	if(params){
		params->lock = false;
		g_free(params);
	}
}


gint gfalt_set_timeout(gfalt_params_t params, guint64 timeout, GError** err){
	g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid parameter handle value");
	params->timeout = timeout;
	return 0;
}


guint64 gfalt_get_timeout(gfalt_params_t params, GError** err){
	g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");		
	return params->timeout;	
}	


gint gfalt_set_nbstreams(gfalt_params_t params, guint nbstreams, GError** err){
	g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid parameter handle");
	params->nb_data_streams = nbstreams;
	return 0;	
}

gint gfalt_set_replace_existing_file(gfalt_params_t params, gboolean replace, GError** err){
	g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");	
	params->replace_existing	= replace;
	return 0;
}

gint gfalt_set_offset_from_source(gfalt_params_t params, off_t offset, GError** err){
	g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");	
	params->start_offset	= offset;
	return 0;
}

gint gfalt_set_user_data(gfalt_params_t params, gpointer user_data, GError** err){
	g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");	
	params->user_data	= user_data;
	return 0;	
}

guint gfalt_get_nbstreams(gfalt_params_t params, GError** err){
	g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid parameter handle");
	return params->nb_data_streams ;
}


/*
int gfalt_set_uuid(gfalt_params_t, uuid_t uuid, GError** err);

int gfalt_set_callback_mperiod(gfalt_params_t, unsigned int mtime, GError** err); // time in ms between two callback calls.


void gfalt_set_monitor_tfr(gfalt_params_t params, gfalt_monitor_func callback);*/

}
