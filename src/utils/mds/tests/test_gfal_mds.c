/*
 * This is a functional test that iterates through the parameters
 * and tries to resolve the final surl
 */
#include <gfal_api.h>
#include <stdio.h>
#include "../gfal_mds.h"


static const char* type2str(mds_type_endpoint type)
{
    switch (type) {
        case SRMv2:
            return "SRMv2";
        case SRMv1:
            return "SRMv1";
        case WebDav:
            return "DAV";
        default:
            return "Unknown";
    }
}


int main(int argc, char** argv)
{
    GError* error = NULL;
    gfal2_context_t context = NULL;

    // Create context
    context = gfal2_context_new(&error);
    if (context == NULL) {
        fprintf(stderr, "%s\n", error->message);
        abort();
    }

    // Explicitly set nobdii = false (bdii = true)
    gfal_set_nobdiiG(context, 0);

    // Enable debug
    gfal_set_verbose(GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);

    // Iterate through arguments
    const int MAX_ENDPOINTS = 3;
    gfal_mds_endpoint endpoints[MAX_ENDPOINTS];

    int i, j;
    for (i = 1; i < argc; ++i) {
        const char *surl = argv[i];

        fprintf(stdout, "Querying %s\n", surl);
        j = gfal_mds_resolve_srm_endpoint(context, surl, endpoints,
                        MAX_ENDPOINTS, &error);
        if (j < 0) {
            fprintf(stderr, "Error while querying %s: %s\n",
                    surl, error->message);
            g_error_free(error);
            error = NULL;
        }

        while (j > 0) {
            fprintf(stdout, "Result %d: [%s] %s\n",
                    j, type2str(endpoints[j - 1].type), endpoints[j - 1].url);
            --j;
        };
    }

    // Clean up context
    gfal2_context_free(context);
    return 0;
}
