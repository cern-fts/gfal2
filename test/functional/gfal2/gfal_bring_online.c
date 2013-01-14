#include <gfal_api.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    gfal2_context_t handle;
    GError* error = NULL;
    const char *surl, *protocols[10];
    int nprotocols, i;

    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE);
    if (argc < 2) {
        fprintf (stderr, "usage: %s filename [protocol [protocol|...]\n", argv[0]);
        fprintf (stderr, "Protocols are optional. By default: rfio");
        exit (1);
    }

    surl = argv[1];
    if (argc > 2) {
        nprotocols = argc - 2;
        for (i = 0; i < 10; ++i)
            protocols[i] = argv[i + 2];
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


    handle = gfal2_context_new(&error);
    if (!handle) {
        fprintf(stderr, " bad initialization %d : %s.\n", error->code, error->message);
        return -1;
    }

    gfal2_set_opt_string_list(handle, "SRM PLUGIN", "TURL_PROTOCOLS",
                              protocols, nprotocols, &error);
    if (error) {
        fprintf(stderr, " Could not set the protocol list: %s\n",
                error->message);
        return -1;
    }

    if (gfal2_bring_online(handle, surl, &error) < 0) {
        printf("Bring online failed: %s (%d)\n", error->message, error->code);
    }
    else {
        printf("Bring online succeeded!\n");
    }

    gfal2_context_free(handle);
    return 0;
}
