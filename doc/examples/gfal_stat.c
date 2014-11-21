/**
 * Example source code for stating files and directories using GFAL2
 */

#include <stdio.h>
#include <stdlib.h>

// You only need this header
#include <gfal_api.h>


static void _abort_on_gerror(const char* msg, GError* error)
{
    if (!error)
        return;
    fprintf(stderr, "%s: (%d) %s\n", msg, error->code, error->message);
    exit(1);
}


int main(int argc, char** argv)
{
    const char* url;
    if (argc < 2) {
        fprintf(stderr, "Missing url parameter");
        abort();
    }
    url = argv[1];

    // Errors will be put here
    GError* error = NULL;

    // Create a gfal2 context
    gfal2_context_t context;
    context = gfal2_context_new(&error);
    _abort_on_gerror("Could not create the gfal2 context", error);

    // Stat the entry
    struct stat fstat;
    gfal2_stat(context, url, &fstat, &error);
    _abort_on_gerror("Could not stat", error);

    // Print
    printf("%s\n", url);
    printf("Mode: %04o\n", fstat.st_mode & 0777);
    printf("Size: %d\n", fstat.st_size);

    // Release the context
    gfal2_context_free(context);
    return 0;
}
