/**
 * Compile command : gcc -o gfalt_copyfile gfalt_copyfile.c `pkg-config --libs --cflags gfal_transfer`
 */

#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <transfer/gfal_transfer.h>

//
// This test follows exactly the FTS 3.0 copy pattern and can simualte a long run of successive copy in different context
//


void call_perf(gfalt_transfer_status_t h, const char* src, const char* dst, gpointer user_data){

    size_t avg  =  gfalt_copy_get_average_baudrate(h,NULL) / 1024;
    size_t inst =  gfalt_copy_get_instant_baudrate(h,NULL) / 1024;
    size_t trans=  gfalt_copy_get_bytes_transfered(h,NULL);
    time_t elapsed  = gfalt_copy_get_elapsed_time(h,NULL);

    printf(" perf marker avg : %ld, inst: %ld, elapsed: %ld, trans: %ld", avg, inst, trans, elapsed);
}



int internal_copy(gfal2_context_t* handle, gfalt_params_t* params, const char* src, const char* dst){
    GError * tmp_err = NULL; // classical GError/glib error management
    struct stat buff;
    struct stat buff2;

    printf(" stat src files.... \n");
    if( gfal2_stat(*handle, src, &buff, &tmp_err) !=0){
        printf(" error while the file stat %d : %s.\n", tmp_err->code,tmp_err->message);
        return -1;
    }else{
        g_assert(buff.st_size != 0);
        printf(" file size %ld",buff.st_size );
    }

   printf(" begin transfer .... \n");
    if(  gfalt_copy_file(*handle, *params, src, dst, &tmp_err)   != 0){
         printf(" error while the file transfer %d : %s.\n", tmp_err->code,tmp_err->message);
         return -1;
    }else
        printf(" transfer sucessfull ! \n");

    printf(" stat dst file  .... \n");
    if( gfal2_stat(*handle, dst, &buff2, &tmp_err) !=0){
        printf(" error while the file stat %d : %s.\n", tmp_err->code,tmp_err->message);
        return -1;
    }{
        g_assert(buff2.st_size  == buff.st_size);
    }
    return 0;
}


int main(int argc, char** argv){
    int i, ret;
    if( argc <5 ){
        printf(" Usage %s [src_url] [checksum] [dst_dir] [nbtime] \n",argv[0]);
		return 1;
	}
	GError * tmp_err = NULL; // classical GError/glib error management
	gfal2_context_t handle;
	
	// initialize gfal
    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);

    char buff[2048];


    const int nbtime = atoi(argv[4]);
    const char * checksum_user = argv[2];
    const char* src_file = argv[1];
    printf(" sechedule %d transfets...", nbtime);

    for(i =0; i < nbtime;++i){
            printf("execute %d....", i);

         if( (handle = gfal2_context_new(&tmp_err)) == NULL ) {
             printf(" bad initialization %d : %s.\n", tmp_err->code,tmp_err->message);
             return -1;
         }

         generate_random_uri(argv[3], "dest_fts_load_test_file", buff, 2048);
         // creat params
         gfalt_params_t my_params = gfalt_params_handle_new(NULL);
         gfalt_set_replace_existing_file(my_params, TRUE, NULL);
         gfalt_set_checksum_check(my_params, TRUE, NULL);
         gfalt_set_user_defined_checksum(my_params,"ADLER32",checksum_user, NULL);
         gfalt_set_monitor_callback(my_params, &call_perf,&tmp_err);
         gfalt_set_nbstreams(my_params, 0, &tmp_err);
         gfalt_set_timeout(my_params,500, &tmp_err);
        // gfalt_set_src_spacetoken(my_params, "DTEAMLCGUTILSTEST", &tmp_err);
    // gfalt_set_dst_spacetoken(my_params, "DTEAMLCGUTILSTESTF", &tmp_err);
         // begin copy

        if(internal_copy(&handle, &my_params, src_file, buff) !=0){
            return -1;
        }

        printf(" cleanup the copy ... %s",buff);
        if( (ret =gfal2_unlink(handle, buff, &tmp_err) ) < 0){
            printf(" error message ", tmp_err->message);
            return -1;
        }



        gfal2_context_free(handle);
    }
    return 0;
}

