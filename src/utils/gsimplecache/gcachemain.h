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

#include <glib.h>


#define MAX_LIST_LEN 20000

/**
 * copy the original object to a new one
 */
typedef void (*GSimpleCache_CopyConstructor)(gpointer original, gpointer copy);

typedef struct _GSimpleCache_Handle GSimpleCache;

GSimpleCache* gsimplecache_new(guint64 max_number_item, GSimpleCache_CopyConstructor value_copy, size_t size_item);

void gsimplecache_delete(GSimpleCache* cache);

void gsimplecache_add_item_kstr(GSimpleCache* cache, const char* key, void* item);

int gsimplecache_take_one_kstr(GSimpleCache* cache, const char* key, void* res);

gboolean gsimplecache_remove_kstr(GSimpleCache* cache, const char* key);

