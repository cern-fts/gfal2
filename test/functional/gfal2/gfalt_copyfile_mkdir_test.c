/**
 * Compile command : gcc -o gfalt_copyfile gfalt_copyfile.c `pkg-config --libs --cflags gfal_transfer`
 */

#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <transfer/gfal_transfer.h>

#include "gfal_lib_test.h"


int main(int argc, char** argv){
	if( argc <3 ){
		printf(" Usage %s [src_url] [dst_dir] \n",argv[0]);
		return 1;
	}
	GError * tmp_err = NULL; // classical GError/glib error management
	gfal2_context_t handle;
	int ret=-1;
	
	char * src_uri = argv[1];
	char dst_uri[2048];
	char dst_uri2[2048];
    char dst_uri_simple[2048];
	generate_random_uri(argv[2], "generate_folder", dst_uri, 2048);
	generate_random_uri(dst_uri, "generate_folder2", dst_uri2, 2048);
	generate_random_uri(dst_uri2, "generate_folder3", dst_uri, 2048);	
	generate_random_uri(dst_uri, "generate_dest_file", dst_uri2, 2048);	
    generate_random_uri(argv[2], "simple_file_at_root", dst_uri_simple, 2048);

	// initialize gfal
    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);
	 if( (handle = gfal2_context_new(&tmp_err)) == NULL ) {
		 printf(" bad initialization %d : %s.\n", tmp_err->code,tmp_err->message);
		 return -1;
	 }
	 
	 gfalt_params_t params = gfalt_params_handle_new(NULL);
	 
	 int res_params = gfalt_set_create_parent_dir(params, TRUE, &tmp_err);
	 g_assert(res_params == 0 && tmp_err==NULL);
	 g_assert( gfalt_get_create_parent_dir(params, &tmp_err) == TRUE && tmp_err == NULL);	
	
	 // begin copy
    printf(" begin to copyfile to child dir  from %s to %s", src_uri, dst_uri2);
	if( (ret = gfalt_copy_file(handle, params, src_uri, dst_uri2, &tmp_err) )  != 0){
		 printf(" error while the file transfer %d : %s.\n", tmp_err->code,tmp_err->message);
		 return -1;		
	}else
		printf(" transfer sucessfull ! \n");

    // begin copy
   printf(" begin to copyfile to root dir from %s to %s", src_uri, dst_uri_simple);
   if( (ret = gfalt_copy_file(handle, params, src_uri, dst_uri_simple, &tmp_err) )  != 0){
        printf(" error while the file transfer %d : %s.\n", tmp_err->code,tmp_err->message);
        return -1;
   }else
       printf(" transfer sucessfull ! \n");
			
	gfalt_params_handle_delete(params, NULL);	
	gfal2_context_free(handle);
	return ret;
}

