/**
 * Regression test for DMC-529
 * Set the credentials via configuration, and make sure that it does work!
 */
#include <gfal_api.h>


int main(int argc, char** argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s proxy surl\n", argv[0]);
        return 1;
    }

    const char* uproxy = argv[1];
    const char* surl = argv[2];

    if (getenv("X509_USER_CERT") ||
        getenv("X509_USER_KEY") ||
        getenv("X509_USER_PROXY")) {
        fprintf(stderr, "Unset X509_USER_* environment variables before calling");
        return 1;
    }

    GError* error = NULL;
    gfal2_context_t context = gfal2_context_new(&error);
    if (error) {
        fprintf(stderr, "%s\n", error->message);
        g_clear_error(&error);
        return 1;
    }

    gfal2_set_opt_string(context, "X509", "CERT", uproxy, NULL);
    gfal2_set_opt_string(context, "X509", "KEY", uproxy, NULL);

    struct stat st;
    int ret = gfal2_stat(context, surl, &st, &error);
    if (ret < 0) {
        fprintf(stderr, "Failed to stat: %s\n", error->message);
        g_clear_error(&error);
        return 1;
    }

    printf("Stat OK:\n\tMode:\t%04o\n\tSize:\t%d\n", st.st_mode, st.st_size);

    gfal2_context_free(context);
    return 0;
}
