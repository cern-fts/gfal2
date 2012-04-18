/*
 *  lfc test file 
 * 
 * */
 
 
#include <cgreen/cgreen.h>
#include <glib.h>
#include <dlfcn.h>


#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>

#include <common/rfio/gfal_rfio_plugin_main.h>
#include <common/rfio/gfal_rfio_plugin_layer.h>
#include <common/gfal_common_internal.h>
#include "../../unit_test_constants.h"
#include "test__gfal_rfio_plugin.h"

static struct rfio_proto_ops rf = { 
		.open =  rfio_mock_open,
		.read = rfio_mock_read,
		.write = rfio_mock_write,
		.close= rfio_mock_close,
		.lseek = rfio_mock_lseek,
	};

struct rfio_proto_ops * gfal_rfio_mock_loader(GError** err){
	return &rf;
 }

void test_rfio_mock_all(){
	gfal_rfio_internal_loader= gfal_rfio_mock_loader;
}

void gfal2_test_load_plugin(){
	GError* err=NULL;
	gfal_handle handle = gfal_initG(&err);
	assert_true_with_message(handle != NULL, " handle is not properly allocated");
	assert_true_with_message(err==NULL, " error must be NULL");
	if(handle==NULL)
		return;
	char** res = gfal_plugins_get_list(handle, &err);
	assert_true_with_message(err==NULL, " error must be NULL");
	int i;
	gboolean valid = FALSE;
	for(i=0; res[i] != NULL; ++i)
		valid = (strcmp(res[i], "rfio")==0)?TRUE:valid;
	
	assert_true_with_message(valid, " error must be a loaded plugin");	
	g_strfreev(res);
	gfal_handle_freeG(handle);	
	
}


void test_check_turl_rfio(){
	GError* tmp_err=NULL;
	gboolean b = gfal_rfio_internal_check_url(TEST_SRM_RFIO_EXAMPLE1, &tmp_err);
	assert_true_with_message(b && tmp_err==NULL, " must be a valid url");	
	b= gfal_rfio_internal_check_url(TEST_SRM_ONLY_READ_ACCESS, &tmp_err);
	assert_true_with_message(b==FALSE && tmp_err==NULL, " must be a invalid url");	
}
