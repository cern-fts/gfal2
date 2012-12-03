/**
 * Compile command : gcc gfal_teststat.c `pkg-config --libs --cflags gfal2`
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <gfal_api.h>
#include <nettle/md5.h>
#include <string.h>
#include "gfal_lib_test.h"



int main(int argc, char **argv)
{
    int s_content = rand()%65635;
    int res,i;
    ssize_t size;
    GError * tmp_err=NULL;
    char nettle_md5[MD5_DIGEST_SIZE]={0};
    char nettle_md5_char[1024]={0};
    char remote_md5_char[1024]={0};

    printf(" generate random content of size s %d ... \n", s_content);
    char * c = generate_random_string_content(s_content);

    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_DEBUG | GFAL_VERBOSE_VERBOSE);

    char filename[2048];
    generate_random_uri(argv[1], "test_checksum", filename, 2048);
   // printf(" string : %s\n", c);

    printf(" initialize gfal2 context..\n");

    gfal2_context_t context = gfal2_context_new(&tmp_err);
    g_assert(context != NULL);
    g_assert(tmp_err == NULL);

    printf("try with enoent file checksum algorithm \n");
    res= gfal2_checksum(context, filename, GFAL_CHKSUM_MD5,0,0, remote_md5_char,1024, &tmp_err);
    g_assert(res != 0);
    g_assert(tmp_err != 0);
    g_assert(tmp_err->code == ENOENT);
    g_clear_error(&tmp_err);

    printf(" try to create file %s ... \n",filename);
    int fd = gfal_open(filename, O_WRONLY | O_CREAT );
    g_assert(fd > 0);
    size = gfal_write(fd, c, s_content);
    g_assert(size == s_content);
    res= gfal_close(fd);
    g_assert(res == 0);

    printf(" file uploaded with success  %s ... \n",filename);


    printf(" calc internal checksum with nettle ... \n");
    struct md5_ctx mymd5;
    md5_init (&mymd5);
    md5_update(&mymd5, s_content, (const uint8_t *)  c);
    md5_digest(&mymd5, MD5_DIGEST_SIZE, (uint8_t *) nettle_md5);

    printf(" content md5 : ");
    for(i=0;i < MD5_DIGEST_SIZE; ++i)
        snprintf(nettle_md5_char+i*2, 1024, "%02x",  (unsigned char) nettle_md5[i]);
    printf("%s\n",nettle_md5_char);



    printf(" test NULL args for safety \n");
    res= gfal2_checksum(context, NULL, NULL,0,0, NULL,1024, &tmp_err);
    g_assert(res != 0);
    g_assert(tmp_err != 0);
    g_assert(tmp_err->code == EFAULT);
    g_clear_error(&tmp_err);

    printf("try with inconsistent checksum algorithm \n");
    res= gfal2_checksum(context, filename, "SNCF_BONJOUR",0,0, remote_md5_char,1024, &tmp_err);
    g_assert(res != 0);
    g_assert(tmp_err != 0);
    g_assert(tmp_err->code == ENOTSUP);
    g_clear_error(&tmp_err);


    printf(" calc remote checksum with gfal ... \n");


    res= gfal2_checksum(context, filename, GFAL_CHKSUM_MD5,0,0, remote_md5_char,1024, &tmp_err);
    if(tmp_err){
        fprintf(stderr, " error checksum : %s", tmp_err->message);
        g_assert_not_reached();
    }
    g_assert(res == 0);
    g_assert(tmp_err == 0);

    printf(" remote content md5 : ");
    printf("%s",  remote_md5_char);
    printf("\n");

    g_assert( memcmp(remote_md5_char, nettle_md5_char, MD5_DIGEST_SIZE) ==0);

    gfal2_context_free(context);
    g_free(c);
    return 0;
}
