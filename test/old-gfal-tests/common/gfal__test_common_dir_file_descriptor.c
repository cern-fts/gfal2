
/* unit test for file descriptor */


#include <cgreen/cgreen.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <glib.h>
#include "gfal_common_internal.h"
#include "../unit_test_constants.h"
#include "gfal_common_dir_handle.h"
#include "gfal_common_filedescriptor.h"
#include "gfal_common_errverbose.h"
#include "gfal_types.h"


void gfal2_test__dir_file_descriptor_low()
{
	
	GError* tmp_err=NULL;
	gpointer pfile = (gpointer) (long)rand();
	gfal_fdesc_container_handle  h = gfal_file_descriptor_handle_create(NULL);
	if( h == NULL){
		assert_true_with_message(FALSE, " fail must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}
	gpointer res = gfal_get_file_desc(h, 10, &tmp_err);
	if( res || !tmp_err){
		assert_true_with_message(FALSE, " no file desc must be in the container");
		return;
	}
	g_clear_error(&tmp_err);
    int key = gfal_add_new_file_desc(h, pfile, &tmp_err);
    if( key ==0 || tmp_err){
		assert_true_with_message(FALSE, " must be a valid key creation");
		gfal_release_GError(&tmp_err);
		return;
	}
	
	res = gfal_get_file_desc(h, key, &tmp_err);
	if( res != pfile || tmp_err){
		assert_true_with_message(FALSE, " must be a valid descriptor addition");
		gfal_release_GError(&tmp_err);
		return;
	}	
	
	gboolean b = gfal_remove_file_desc(h, key+1, &tmp_err);
	if(b || !tmp_err){
		assert_true_with_message(FALSE, " must be an invalid deletion");
		return;
	}
	g_clear_error(&tmp_err);
	 b = gfal_remove_file_desc(h, key, &tmp_err);
	if(!b || tmp_err){
		assert_true_with_message(FALSE, " must be a valid deletion");
		gfal_release_GError(&tmp_err);
		return;
	}	
	
}




void gfal2_test__dir_file_descriptor_high()
{
	/*GError* tmp_err= NULL; -> need to be rewrite
	const char* id_module = "mock name";
	const char* id_module2 = "mock name2";
	const gpointer desc =  GINT_TO_POINTER(rand()), desc2 =  GINT_TO_POINTER(rand());
	
	gfal_handle handle = gfal_initG(&tmp_err);
	if(handle == NULL){
		assert_true_with_message(FALSE, " error while init");
		return;
	}
	
	gfal_fdesc_container_handle h =  gfal_dir_handle_container_instance(&(handle->fdescs), &tmp_err);

	if( h == NULL){
		assert_true_with_message(FALSE, " fail must be a valid init");
		gfal_release_GError(&tmp_err);
		return;
	}
	
	gfal_file_handle d = gfal_file_handle_bind(h, 10, &tmp_err);
	if( d != NULL || !tmp_err){
		assert_true_with_message(FALSE, " fail, must an invalid bind");
		return;
	}
	g_clear_error(&tmp_err);
	
	int key = gfal_file_handle_create(h,  id_module, desc, &tmp_err);
	if( key == 0 || tmp_err){
		assert_true_with_message(FALSE, " fail, must be a valid creation");
		gfal_release_GError(&tmp_err);
		return;
	}

	int key2 = gfal_file_handle_create(h,  id_module2, desc2, &tmp_err);
	if( key == 0 || tmp_err){
		assert_true_with_message(FALSE, " fail, must be a valid creation 2");
		gfal_release_GError(&tmp_err);
		return;
	}
	
	d = gfal_file_handle_bind(h, key, &tmp_err);
	if( d == NULL || strings_are_equal(d->module_name, id_module) == FALSE || d->fdesc != desc){
		assert_true_with_message(FALSE, " fail, must be a valid get");
		gfal_release_GError(&tmp_err);
		return;	
	} 	
	
	d = gfal_file_handle_bind(h, key2, &tmp_err);
	if( d == NULL || strings_are_equal(d->module_name, id_module) == FALSE || d->fdesc != desc2){
		assert_true_with_message(FALSE, " fail, must be a valid get");
		gfal_release_GError(&tmp_err);
		return;	
	} 		

	gboolean b = gfal_remove_file_desc(h, key+key2, &tmp_err);
	if(b || !tmp_err){
		assert_true_with_message(FALSE, " must be an invalid deletion");
		return;
	}
	g_clear_error(&tmp_err);
	 b = gfal_remove_file_desc(h, key, &tmp_err);
	if(!b || tmp_err){
		assert_true_with_message(FALSE, " must be a valid deletion");
		gfal_release_GError(&tmp_err);
		return;
	}	
	b = gfal_remove_file_desc(h, key+key2, &tmp_err);
	if(b || !tmp_err){
		assert_true_with_message(FALSE, " must be an invalid deletion");
		return;
	}
	g_clear_error(&tmp_err);
	
	gfal_handle_freeG(handle);	*/
}
