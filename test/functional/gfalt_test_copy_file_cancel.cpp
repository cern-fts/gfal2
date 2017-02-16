/**
 * Start a transfer, wait half a second, and cancel it from a different thread
 */
#include <stdio.h>
#include <gfal_api.h>
#include <pthread.h>
#include <transfer/gfal_transfer.h>
#include <unistd.h>
#include <future/glib.h>


typedef struct copier_params_t {
    gfal2_context_t handle;
    gfalt_params_t  transfer_params;
    const char     *source;
    const char     *destination;
} copier_params_t;



GQuark test_domain()
{
    return g_quark_from_static_string("gfalt_copyfile_cancel");
}



void* copier_thread_method(void* data)
{
    copier_params_t *params = (copier_params_t*)data;
    GError          *error = NULL;
    struct stat      stat_buffer;

    // Source must exist
    if (gfal2_stat(params->handle, params->source, &stat_buffer, &error) !=0) {
        g_prefix_error(&error, "Source file does not exist ");
        goto out;
    }

    // Do the copy
    if (gfalt_copy_file(params->handle, params->transfer_params,
                        params->source, params->destination,
                        &error) == 0) {
        if (gfal2_stat(params->handle, params->destination, &stat_buffer, &error) != 0)
            g_prefix_error(&error, "Destination file does not exist after the copy ");
    }

    // Exit sequence
out:
    free(params);
    return error;
}



int spawn_copy(pthread_t* thread, gfal2_context_t handle,
               gfalt_params_t transfer_params,
               const char* source, const char* destination)
{
    copier_params_t *copier_params = (copier_params_t*)malloc(sizeof(*copier_params));
    copier_params->handle          = handle;
    copier_params->transfer_params = transfer_params;
    copier_params->source          = source;
    copier_params->destination     = destination;
    return pthread_create(thread, NULL, copier_thread_method, copier_params);
}



void monitor_callback(gfalt_transfer_status_t h,
                      const char* source, const char* destination,
                      gpointer udata)
{
    // We don't care about the performance data. We just want to trigger
    // all those internal locks.
    (void)h;
    (void)source;
    (void)destination;
    (void)udata;
}



void event_callback(const gfalt_event_t e, gpointer user_data)
{
    fprintf(stderr, "[%s] %s %s\n", g_quark_to_string(e->domain),
            g_quark_to_string(e->stage),
            e->description);
}



int main(int argc, char** argv)
{
    const char     *source;
    const char     *destination;
    gfal2_context_t handle = NULL;
    gfalt_params_t  transfer_params = NULL;
    GError         *error = NULL, *copier_error = NULL;
    pthread_t       copier_thread;
    int             i;

    // Parse params
    if (argc != 3) {
        fprintf(stderr, "Usage: %s SOURCE DESTINATION", argv[0]);
        return 1;
    }

    source = argv[1];
    destination = argv[2];

    // Create handle and transfer parameters
    handle = gfal2_context_new(&error);
    if (!handle)
        goto out;

    transfer_params = gfalt_params_handle_new(&error);
    if (!transfer_params)
        goto unwind_context;
    gfalt_set_replace_existing_file(transfer_params, TRUE, NULL);
    gfalt_add_monitor_callback(transfer_params, monitor_callback, NULL, NULL, NULL);
    gfalt_add_event_callback(transfer_params, event_callback, NULL, NULL, NULL);

    // Spawn the thread that will do the copy
    printf("Spawning the copy '%s' => '%s'\n", source, destination);
    if (spawn_copy(&copier_thread, handle, transfer_params, source, destination) != 0) {
        error = g_error_new(test_domain(), errno,
                            "Could not spawn the copier thread.");
        goto unwind_params;
    }

    // Give it some time
    printf("Waiting some time before canceling\n");
    for (i = 0; i < 30; ++i) {
        fputc('.', stdout);
        fflush(stdout);
        sleep(1);
    }
    fputc('\n', stdout);

    // Cancel
    printf("Calling gfal2_cancel.\n");
    if (gfal2_cancel(handle) <= 0) {
        error = g_error_new(test_domain(), EINVAL,
                            "No jobs canceled. Expected at least one.");
        goto unwind_thread;
    }
    printf("Transfer canceled normally\n");

// Unwinding
unwind_thread:
    pthread_join(copier_thread, (void**)&copier_error);
unwind_params:
    gfalt_params_handle_delete(transfer_params, NULL);
unwind_context:
    gfal2_context_free(handle);
out:
    if (error)
        fprintf(stderr, "FAILURE: %s (%d)\n", error->message, error->code);
    if (copier_error)
        fprintf(stderr, "COPIER FAILURE: %s (%d)\n", copier_error->message, copier_error->code);
    return error || (copier_error && copier_error->code != ECANCELED);
}
