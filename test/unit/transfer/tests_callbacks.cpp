/*
* Copyright @ CERN, 2015.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <gtest/gtest.h>
#include <cstdlib>
#include <gfal_api.h>
#include <gfal_plugins_api.h>


static GQuark domain = g_quark_from_static_string("TEST");


static void monitor_callback_1(gfalt_transfer_status_t h, const char* src, const char* dst,
        gpointer user_data)
{
    int *data = (int*)(user_data);
    (*data) += 1;
}


static void monitor_callback_2(gfalt_transfer_status_t h, const char* src, const char* dst,
        gpointer user_data)
{
    int *data = (int*)(user_data);
    (*data) += 10;
}


static void event_callback_1(const gfalt_event_t e, gpointer user_data)
{
    int *data = (int*)(user_data);
    (*data) += 1;
}

static void event_callback_2(const gfalt_event_t e, gpointer user_data)
{
    int *data = (int*)(user_data);
    (*data) += 10;
}


static void event_reset_counter(gpointer user_data)
{
    int *data = (int*)(user_data);
    *data = 0;
}


TEST(gfalTransfer, test_monitor_callbacks)
{
    int counter1 = 0, counter2 = 0;

    gfalt_params_t params = gfalt_params_handle_new(NULL);
    gfalt_add_monitor_callback(params, monitor_callback_1, &counter1, event_reset_counter, NULL);
    gfalt_add_monitor_callback(params, monitor_callback_2, &counter1, event_reset_counter, NULL);

    plugin_trigger_monitor(params, NULL, "source", "destination");
    g_assert(counter1 == 11);
    g_assert(counter2 == 0);

    gfalt_add_monitor_callback(params, monitor_callback_1, &counter2, event_reset_counter,
            NULL);
    gfalt_add_monitor_callback(params, monitor_callback_2, &counter2, event_reset_counter,
            NULL);

    plugin_trigger_monitor(params, NULL, "source", "destination");
    g_assert(counter1 == 0); // Was reset on the second call to gfalt_add_monitor_callback
    g_assert(counter2 == 11);

    gfalt_params_handle_delete(params, NULL);
    g_assert(counter1 == 0);
    g_assert(counter2 == 0);
}


TEST(gfalTransfer, test_event_callbacks)
{
    int counter1 = 0, counter2 = 0;

    gfalt_params_t params = gfalt_params_handle_new(NULL);
    gfalt_add_event_callback(params, event_callback_1, &counter1, event_reset_counter, NULL);
    gfalt_add_event_callback(params, event_callback_2, &counter1, event_reset_counter, NULL);

    plugin_trigger_event(params,  domain, GFAL_EVENT_NONE, domain, "TEST");
    g_assert(counter1 == 11);
    g_assert(counter2 == 0);

    gfalt_add_event_callback(params, event_callback_1, &counter2, event_reset_counter, NULL);
    gfalt_add_event_callback(params, event_callback_2, &counter2, event_reset_counter, NULL);

    plugin_trigger_event(params,  domain, GFAL_EVENT_NONE, domain, "TEST");
    g_assert(counter1 == 0); // Was reset on the second call to gfalt_add_event_callback
    g_assert(counter2 == 11);

    gfalt_params_handle_delete(params, NULL);
    g_assert(counter1 == 0);
    g_assert(counter2 == 0);
}


static const char* test_plugin_name()
{
    return "TEST-PLUGIN";
}


static int test_plugin_enter_hook(plugin_handle plugin_data, gfal2_context_t context,
        gfalt_params_t params, GError** error)
{
    gfalt_add_event_callback(params, event_callback_1, plugin_data, NULL, NULL);
    return 0;
}


static int test_plugin_check_transfer(plugin_handle plugin_data, gfal2_context_t context,
        const char* src, const char* dst, gfal_url2_check check)
{
    return 1;
}


static int test_plugin_copy(plugin_handle plugin_data, gfal2_context_t context,
        gfalt_params_t params, const char* src, const char* dst, GError**)
{
    plugin_trigger_event(params,  domain, GFAL_EVENT_NONE, domain, "TEST");
    return 0;
}


static int test_plugin_bulk(plugin_handle plugin_data, gfal2_context_t context, gfalt_params_t params,
        size_t nbfiles, const char* const* srcs, const char* const* dsts, const char* const* checksums,
        GError** op_error, GError*** file_errors)
{
    plugin_trigger_event(params,  domain, GFAL_EVENT_NONE, domain, "TEST");
    plugin_trigger_event(params,  domain, GFAL_EVENT_NONE, domain, "TEST");
    return 0;
}

TEST(gfalTransfer, test_inject_callback)
{
    int counter = 0;
    gfal_plugin_interface test_plugin;
    memset(&test_plugin, 0, sizeof(test_plugin));

    test_plugin.getName = test_plugin_name;
    test_plugin.copy_enter_hook = test_plugin_enter_hook;
    test_plugin.plugin_data = &counter;
    test_plugin.check_plugin_url_transfer = test_plugin_check_transfer;
    test_plugin.copy_file = test_plugin_copy;
    test_plugin.copy_bulk = test_plugin_bulk;

    gfal2_context_t context = gfal2_context_new(NULL);
    gfal2_register_plugin(context, &test_plugin, NULL);

    gfalt_params_t params = gfalt_params_handle_new(NULL);
    gfalt_copy_file(context, params, "test://", "test://", NULL);

    g_assert(counter == 4);

    gfalt_params_handle_delete(params, NULL);
}


TEST(gfalTransfer, test_inject_callback_bulk)
{
    int counter = 0;
    gfal_plugin_interface test_plugin;
    memset(&test_plugin, 0, sizeof(test_plugin));

    test_plugin.getName = test_plugin_name;
    test_plugin.copy_enter_hook = test_plugin_enter_hook;
    test_plugin.plugin_data = &counter;
    test_plugin.check_plugin_url_transfer = test_plugin_check_transfer;
    test_plugin.copy_file = test_plugin_copy;
    test_plugin.copy_bulk = test_plugin_bulk;

    gfal2_context_t context = gfal2_context_new(NULL);
    gfal2_register_plugin(context, &test_plugin, NULL);

    const char* sources[] = {"test://", "test://"};
    const char* destinations[] = {"test://", "test://"};

    GError** file_errors;
    gfalt_params_t params = gfalt_params_handle_new(NULL);
    gfalt_copy_bulk(context, params, 2,
            sources, destinations, NULL, NULL, &file_errors);

    g_assert(counter == 6);

    gfalt_params_handle_delete(params, NULL);
}
