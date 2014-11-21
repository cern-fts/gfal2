/**
 * Example source code for directory listing using GFAL2
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

    // Open the directory
    DIR* dir_handle = gfal2_opendir(context, url, &error);
    _abort_on_gerror("Could not open the directory", error);

    // Iterate and print
    struct dirent* ent = gfal2_readdir(context, dir_handle, &error);
    while (ent) {
        printf("%s\n", ent->d_name);
        ent = gfal2_readdir(context, dir_handle, &error);
    }
    _abort_on_gerror("Could not read the directory", error);

    // Close the directory
    gfal2_closedir(context, dir_handle, &error);
    _abort_on_gerror("Could not close the directory", error);

    // Release the context
    gfal2_context_free(context);
    return 0;
}
