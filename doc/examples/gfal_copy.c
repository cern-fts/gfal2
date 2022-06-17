/**
 * Example source code for copying files using GFAL2
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

// This will be called when an event happens
static void my_event_callback(const gfalt_event_t e, gpointer user_data)
{
    const char* side_str;
    switch (e->side) {
    case GFAL_EVENT_SOURCE:
        side_str = "SOURCE";
        break;
    case GFAL_EVENT_DESTINATION:
        side_str = "DESTINATION";
        break;
    default:
        side_str = "BOTH";
    }

    const char* domain = g_quark_to_string(e->domain);
    const char* stage = g_quark_to_string(e->stage);

    printf("Event received: %s %s %s %s\n", side_str, domain, stage, e->description);
}

// This will be called when there is performance information
static void my_monitor_callback(gfalt_transfer_status_t h, const char* src,
        const char* dst, gpointer user_data)
{
    if (h) {
        size_t avg = gfalt_copy_get_average_baudrate(h, NULL);
        size_t inst = gfalt_copy_get_instant_baudrate(h, NULL);
        size_t trans = gfalt_copy_get_bytes_transfered(h, NULL);
        time_t elapsed = gfalt_copy_get_elapsed_time(h, NULL);
        printf("%zu bytes/second average (%zu instant). Transferred %zu, elapsed %d seconds \n", avg, inst, trans, (int) elapsed);
    }
}


int main(int argc, char** argv)
{
    if (argc < 3) {
        fprintf(stderr, "Missing source and/or destination parameter");
        abort();
    }
    const char* source = argv[1];
    const char* destination = argv[2];

    // Errors will be put here
    GError* error = NULL;

    // Create a gfal2 context
    gfal2_context_t context;
    context = gfal2_context_new(&error);
    _abort_on_gerror("Could not create the gfal2 context", error);

    // Set some parameters for the copy
    gfalt_params_t params = gfalt_params_handle_new(&error);
    _abort_on_gerror("Could not create the gfal2 transfer parameters", error);

    gfalt_set_timeout(params, 60, &error);                  // 60 seconds timeout
    gfalt_set_dst_spacetoken(params, "TOKEN", &error);      // Destination space token, support depends on the plugin (SRM, XROOTD do support this)
    gfalt_set_replace_existing_file(params, FALSE, &error); // Just in case, do not overwrite
    gfalt_set_checksum(params, GFALT_CHECKSUM_NONE, NULL, NULL, NULL); // No checksum
    gfalt_set_create_parent_dir(params, TRUE, &error);      // Create the parent directory if needed
    // Callbacks
    gfalt_add_event_callback(params, my_event_callback, NULL, NULL, &error);     // Called when some event is triggered
    gfalt_add_monitor_callback(params, my_monitor_callback, NULL, NULL, &error); // Performance monitor

    // Do the copy
    gfalt_copy_file(context, params, source, destination, &error);
    _abort_on_gerror("Copy failed", error);

    // Release memory
    gfalt_params_handle_delete(params, &error);
    gfal2_context_free(context);
    return 0;
}
