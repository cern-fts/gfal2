#include <gfal_api.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>



void usage(char **argv, const char* msg, ...)
{
    if (msg) {
        va_list args;
        va_start(args, msg);
        vfprintf(stderr, msg, args);
        va_end(args);
        fputc('\n', stderr);
    }
    fprintf (stderr, "usage: %s filename sync|async [protocol [protocol|...]\n", argv[0]);
    fprintf (stderr, "Protocols are optional. By default: rfio\n");
    exit (1);
}



int main(int argc, char **argv)
{
    gfal2_context_t handle;
    GError         *error = NULL;
    const char     *surl, *protocols[10];
    int             nprotocols, i, async, status;
    char            token[512];

    // Set verbosity
    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE);
    if (argc < 3)
        usage(argv, "Too few arguments");

    // Parse arguments
    surl = argv[2];
    if (strcasecmp(argv[1], "sync") == 0)
        async = 0;
    else if (strcasecmp(argv[1], "async") == 0)
        async = 1;
    else
        usage(argv, "Unknown argument: %s", argv[1]);

    if (argc > 3) {
        nprotocols = argc - 3;
        for (i = 0; i < 10; ++i)
            protocols[i] = argv[i + 3];
    }
    else {
        nprotocols = 1;
        protocols[0] = "rfio";
    }

    printf("Remote SURL: %s\n", surl);
    printf("Protocols: \n");
    for (i = 0; i < nprotocols; ++i) {
        printf("\t%s\n", protocols[i]);
    }

    // Set up handle
    handle = gfal2_context_new(&error);
    if (!handle) {
        fprintf(stderr, " bad initialization %d : %s.\n", error->code, error->message);
        return -1;
    }

    gfal2_set_opt_string_list(handle, "SRM PLUGIN", "TURL_PROTOCOLS",
                              protocols, nprotocols, &error);
    if (error) {
        fprintf(stderr, "Could not set the protocol list: %s\n",
                error->message);
        return -1;
    }

    // Bring online
    status = gfal2_bring_online(handle, surl, 28800, 28800, token, sizeof(token), async, &error);
    if (status < 0)
        printf("Bring online failed: %s (%d)\n", error->message, error->code);
    else if (status == 0)
        printf("Bring online queued. Got token %s\n", token);
    else if (token[0])
        printf("Bring online succeeded! Got token %s\n", token);
    else
        printf("Bring online succeeded! Did not get a token, though\n");

    while (status == 0) {
        fputc('.', stdout);
        fflush(stdout);
        usleep(100);
        status = gfal2_bring_online_poll(handle, surl, token, &error);
        if (status < 0)
            printf("\nPolling failed: %s (%d)\n", error->message, error->code);
        else
            printf("\nBring online finished!\n");
    }

    // Release
    if (!error) {
        if (gfal2_release_file(handle, surl, token, &error) < 0)
            printf("Release failed: %s (%d)\n", error->message, error->code);
        else
            printf("Release succeeded!\n");
    }

    // Free
    gfal2_context_free(handle);
    return 0;
}
