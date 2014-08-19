#pragma once
#ifndef GRIDFTP_PLUGIN_H
#define GRIDFTP_PLUGIN_H
/*
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef __cplusplus
extern "C" {
#endif

const char *gridftp_plugin_name();

plugin_handle gridftp_plugin_load(gfal2_context_t handle, GError ** err);

void gridftp_plugin_unload(plugin_handle handle);

gboolean gridftp_check_url_transfer(plugin_handle handle,
        gfal2_context_t context, const char* src, const char* dst,
        gfal_url2_check type);

#ifdef __cplusplus
}
#endif

#endif /* GRIDFTP_PLUGIN_H */
