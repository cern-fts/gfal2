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
	gfal_context_t handle;
	int ret=-1;
	
	char * src_uri = argv[1];
	char dst_uri[2048];
	char dst_uri2[2048];
	generate_random_uri(argv[2], "replace", dst_uri, 2048);
	generate_random_uri(argv[2], "replace", dst_uri2, 2048);

	// initialize gfal
	gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE);
	 if( (handle = gfal_context_new(&tmp_err)) == NULL ) {
		 printf(" bad initialization %d : %s.\n", tmp_err->code,tmp_err->message);
		 return -1;
	 }	
	
	// initialize gfal
	 if( (handle = gfal_context_new(&tmp_err)) == NULL ) {
		 printf(" bad initialization %d : %s.\n", tmp_err->code,tmp_err->message);
		 return -1;
	 }
	 // begin copy
	printf(" begin to copyfile from %s to %s", src_uri, dst_uri);
	if( (ret = gfalt_copy_file(handle, NULL, src_uri, dst_uri, &tmp_err) )  != 0){
		 printf(" error while the file transfer %d : %s.\n", tmp_err->code,tmp_err->message);
		 return -1;		
	}else
		printf(" transfer sucessfull ! \n");
		
	 // make a second copy, session resilience test
	printf(" make a second copy from %s to %s", src_uri, dst_uri2);
	if( (ret = gfalt_copy_file(handle, NULL, src_uri, dst_uri2, &tmp_err) )  != 0){
		 printf(" error while the file transfer %d : %s.\n", tmp_err->code,tmp_err->message);
		 return -1;		
	}else
		printf(" transfer sucessfull ! \n");		
		
	gfal_context_free(handle);
	return ret;
}

