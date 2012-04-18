/**
 * Compile command : gcc -o gfalt_copyfile gfalt_copyfile.c `pkg-config --libs --cflags gfal_transfer`
 */

#include <stdio.h>
#include <stdlib.h>
#include <transfer/gfal_transfer.h>


int main(int argc, char** argv){
	if( argc <3 ){
		printf(" Usage %s [src_url] [dst_url] \n",argv[0]);
		return 1;
	}
	GError * tmp_err = NULL; // classical GError/glib error management
	gfal_context_t handle;
	int ret=-1;
	
	// initialize gfal
	gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE);
	 if( (handle = gfal_context_new(&tmp_err)) == NULL ) {
		 printf(" bad initialization %d : %s.\n", tmp_err->code,tmp_err->message);
		 return -1;
	 }
	 // begin copy
	if( (ret = gfalt_copy_file(handle, NULL, argv[1], argv[2], &tmp_err) )  != 0){
		 printf(" error while the file transfer %d : %s.\n", tmp_err->code,tmp_err->message);
		 return -1;		
	}else
		printf(" transfer sucessfull ! \n");
		
	gfal_context_free(handle);
	return ret;
}

