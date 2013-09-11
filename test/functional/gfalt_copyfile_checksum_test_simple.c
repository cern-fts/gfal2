/**
 * Compile command : gcc -o gfalt_copyfile gfalt_copyfile.c `pkg-config --libs --cflags gfal_transfer`
 */

#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <errno.h>
#include <transfer/gfal_transfer.h>

#include <common/gfal_lib_test.h>

int main(int argc, char** argv){
	if( argc <3 ){
		printf(" Usage %s [src_url] [dst_dir] \n",argv[0]);
		return 1;
	}
	GError * tmp_err = NULL; // classical GError/glib error management
	gfal2_context_t handle;
	
	char * src_uri = argv[1];
	char dst_uri[2048];
    char uri_enoent[2048];
    generate_random_uri(argv[2], "checksum", dst_uri, 2048);
    generate_random_uri(argv[2], "checksum_enoent", uri_enoent, 2048);

	// initialize gfal
    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);
	 if( (handle = gfal2_context_new(&tmp_err)) == NULL ) {
		 printf(" bad initialization %d : %s.\n", tmp_err->code,tmp_err->message);
		 return -1;
	 }
	
	int ret=-1;

	// create source if not there
	if (generate_file_if_not_exists(handle, src_uri, "file:///etc/hosts", &tmp_err) != 0) {
	    fprintf(stderr, "Could not generate the source: %s", tmp_err->message);
	    g_assert_not_reached();
	}

    printf(" create params without replace but with checksum verification\n");
    gfalt_params_t params = gfalt_params_handle_new(&tmp_err);
    g_assert(tmp_err==NULL);
    gfalt_set_checksum_check(params, TRUE, &tmp_err);
    g_assert(tmp_err==NULL);
	
	 // begin copy
    if( (ret = gfalt_copy_file(handle, params, src_uri, dst_uri, &tmp_err) )  != 0){
		 printf(" error while the file transfer %d : %s.\n", tmp_err->code,tmp_err->message);
		g_assert_not_reached();		 
		 return -1;		
	}else
		printf(" transfer sucessfull, valid initial copy! \n");
		
	
    printf(" add params replace \n");
	gfalt_set_replace_existing_file(params, TRUE, &tmp_err);
	g_assert(tmp_err==NULL);

	 
	printf(" try replace transfer \n");	 
	if( (ret = gfalt_copy_file(handle, params, src_uri, dst_uri, &tmp_err) )  != 0){
		 printf(" error while the file transfer %d : %s.\n", tmp_err->code,tmp_err->message);
		 g_assert_not_reached();		 
		 return -1;		
	}else
		printf(" second transfer is a success  \n");
		
    printf(" try a enoent src transfer \n");
    if( (ret = gfalt_copy_file(handle, params, uri_enoent, dst_uri, &tmp_err) )  == 0){
         g_assert_not_reached();
         return -1;
    }else{
        g_assert(tmp_err);
        g_assert(tmp_err->code == ENOENT);
        g_clear_error(&tmp_err);
    }


	gfalt_params_handle_delete(params, &tmp_err);
	gfal2_context_free(handle);
	return 0;
}

