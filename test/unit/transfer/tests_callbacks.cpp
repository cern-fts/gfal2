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


TEST(gfalTransfer, test_monitor_callbacks)
{
    int counter1 = 0, counter2 = 0;

    gfalt_params_t params = gfalt_params_handle_new(NULL);
    gfalt_add_monitor_callback(params, monitor_callback_1, &counter1, NULL);
    gfalt_add_monitor_callback(params, monitor_callback_2, &counter1, NULL);

    plugin_trigger_monitor(params, NULL, "source", "destination");
    g_assert(counter1 == 11);
    g_assert(counter2 == 0);

    gfalt_add_monitor_callback(params, monitor_callback_1, &counter2, NULL);
    gfalt_add_monitor_callback(params, monitor_callback_2, &counter2, NULL);

    plugin_trigger_monitor(params, NULL, "source", "destination");
    g_assert(counter1 == 11);
    g_assert(counter2 == 11);
}


TEST(gfalTransfer, test_event_callbacks)
{
    int counter1 = 0, counter2 = 0;

    gfalt_params_t params = gfalt_params_handle_new(NULL);
    gfalt_add_event_callback(params, event_callback_1, &counter1, NULL);
    gfalt_add_event_callback(params, event_callback_2, &counter1, NULL);

    plugin_trigger_event(params,  domain, GFAL_EVENT_NONE, domain, "TEST");
    g_assert(counter1 == 11);
    g_assert(counter2 == 0);

    gfalt_add_event_callback(params, event_callback_1, &counter2, NULL);
    gfalt_add_event_callback(params, event_callback_2, &counter2, NULL);

    plugin_trigger_event(params,  domain, GFAL_EVENT_NONE, domain, "TEST");
    g_assert(counter1 == 11);
    g_assert(counter2 == 11);
}
