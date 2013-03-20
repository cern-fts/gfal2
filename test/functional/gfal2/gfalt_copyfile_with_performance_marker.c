/**
 * Compile command : gcc -o gfalt_copyfile gfalt_copyfile.c `pkg-config --libs --cflags gfal_transfer`
 */

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <transfer/gfal_transfer.h>


gfal2_context_t handle = NULL;


void mycall_perf(gfalt_transfer_status_t h, const char* src, const char* dst, gpointer user_data){
    printf(" %s -> %s  : avg byterate : %ld/s, instant byterate : %ld/s, size transfered %ld, time : %ld \n",src,
           dst, gfalt_copy_get_average_baudrate(h,NULL), gfalt_copy_get_instant_baudrate(h,NULL),
           gfalt_copy_get_bytes_transfered(h,NULL),
           gfalt_copy_get_elapsed_time(h,NULL));

}


// setup interrupt
void sigint_cancel(int param)
{
  printf("User pressed Ctrl+C\n");
  if(handle)
    gfal2_cancel(handle);
}



int main(int argc, char** argv){

    signal(SIGINT, &sigint_cancel);
	if( argc <3 ){
		printf(" Usage %s [src_url] [dst_url] \n",argv[0]);
		return 1;
	}
	GError * tmp_err = NULL; // classical GError/glib error management

	int ret=-1;
	
	// initialize gfal
    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE );
	 if( (handle = gfal2_context_new(&tmp_err)) == NULL ) {
		 printf(" bad initialization %d : %s.\n", tmp_err->code,tmp_err->message);
		 return -1;
	 }

     // create params
     gfalt_params_t params = gfalt_params_handle_new(&tmp_err);
     gfalt_set_monitor_callback(params, &mycall_perf, &tmp_err);
     g_assert(tmp_err == NULL);
	 // begin copy
    if( (ret = gfalt_copy_file(handle, params, argv[1], argv[2], &tmp_err) )  != 0){
		 printf(" error while the file transfer %d : %s.\n", tmp_err->code,tmp_err->message);
		 return -1;		
	}else
		printf(" transfer sucessfull ! \n");

    gfalt_params_handle_delete(params,NULL);
	gfal2_context_free(handle);
	return ret;
}

