/**
 * Compile command : gcc -o gfalt_copyfile gfalt_copyfile.c `pkg-config --libs --cflags gfal_transfer`
 */

#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <transfer/gfal_transfer.h>

#include <common/gfal_lib_test.h>


int main(int argc, char** argv){
	if( argc <3 ){
        printf(" Usage %s [src_url_big_file] [dst_dir] \n",argv[0]);
		return 1;
	}
	GError * tmp_err = NULL; // classical GError/glib error management
	gfal2_context_t handle;
	int ret=-1;
	
	char * src_uri = argv[1];
	char dst_uri[2048];
	char dst_uri2[2048];
	generate_random_uri(argv[2], "generate_folder", dst_uri, 2048);
	generate_random_uri(dst_uri, "generate_folder", dst_uri2, 2048);

    gfalt_params_t params = gfalt_params_handle_new(NULL);

	// initialize gfal
    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);
	 if( (handle = gfal2_context_new(&tmp_err)) == NULL ) {
		 printf(" bad initialization %d : %s.\n", tmp_err->code,tmp_err->message);
		 return -1;
	 }	
	
	 // begin copy
	printf(" begin to copyfile from %s to %s", src_uri, dst_uri);
    ret = gfalt_set_timeout(params,5, &tmp_err);
    g_assert(tmp_err == NULL && ret == 0);

    if( (ret = gfalt_copy_file(handle, params, src_uri, dst_uri, &tmp_err) )  == 0){
         fprintf(stderr, "should fail... transfer successfull \n");
         ret= -1;
    }else if( tmp_err->code != ETIMEDOUT){
        fprintf(stderr, " not a timeout error : problem %s \n", tmp_err->message);
        ret= -1;
    }else{
        fprintf(stderr, " success : timeout trigerred :%s \n", tmp_err->message);
        g_clear_error(&tmp_err);
        ret = 0;
    }

	gfal2_context_free(handle);
    gfalt_params_handle_delete(params,NULL);
	return ret;
}

