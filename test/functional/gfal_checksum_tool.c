/**
 * Compile command : gcc gfal_teststat.c `pkg-config --libs --cflags gfal2`
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <string.h>

int main(int argc, char **argv)
{
    char checksum[2048]={0};
    GError * tmp_err=NULL;
    char * url = argv[1];
    char * chk = argv[2];
    int res =-1;

    if(argc < 3){
        fprintf(stdout, "Usage : %s [url] [checksum_type] \n", argv[0]);
        fprintf(stdout, "\t\t Example: %s srm://cvitbdpm1.cern.ch/dpm/cern.ch/home/dteam/testcheck00011 MD5 \n", argv[0]);
        return 1;
    }
    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);
    gfal2_context_t context = gfal2_context_new(&tmp_err);

    if(context){
        res= gfal2_checksum(context, url, chk,0,0, checksum,1024, &tmp_err);
        if(res == 0)
             printf("%s    %s\n",  checksum, url);
        gfal2_context_free(context);
    }
    if(tmp_err)
        fprintf(stderr," %s\n", tmp_err->message);
    return res;
}
