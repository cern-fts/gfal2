/*
 * Copyright (c) CERN 2013-2017
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
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

#pragma once
#ifndef GFAL_LIB_TEST_H
#define GFAL_LIB_TEST_H

#include <gfal_api.h>

#ifdef __cplusplus
extern "C" {
#endif

char* generate_random_uri(const char* uri_dir, const char* prefix, char* buff,
        size_t s_buff);

char * generate_random_string_content(size_t size);

/**
 * If surl does not exist, it creates it putting the content of src.
 */
int generate_file_if_not_exists(gfal2_context_t handle, const char* surl,
        const char* src, GError** error);

/** Same thing, without a handle
 */
int generate_file_if_not_exists2(const char* surl);

/**
 * Clean up file, logging errors if there is
 * If error != ENOENT, it will be considered fatal and this method will abort!
 */
int clean_file(const char* surl);

/**
 * Return 1 if url1 and url2 share the same scheme
 */
int is_same_scheme(const char *url1, const char *url2);

#ifdef __cplusplus
}
#endif

#endif /* GFAL_LIB_TEST_H */
