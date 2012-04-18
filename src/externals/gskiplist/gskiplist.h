#pragma once
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
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



#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _GSkiplist GSkiplist;
typedef struct _GSkipNode GSkipNode;
typedef struct _GSkipLink GSkipLink;


const extern int max_size;

/**
 *  Thread Safe Skip list
 */
struct _GSkiplist{
  GSkipNode* head_node;
  GCompareFunc cmp_func;
  size_t length;
  GStaticRWLock lock;
};


struct _GSkipLink{
  GSkipNode* next;
};

struct _GSkipNode{
  void* key;
  void* data;
  guint height;
  GSkipLink link[];
};


GSkiplist* gskiplist_new(GCompareFunc func);

void gskiplist_delete(GSkiplist* sk);

gboolean gskiplist_insert(GSkiplist* sk, gpointer key, gpointer value);


size_t gskiplist_length(GSkiplist* sk);

gpointer gskiplist_remove(GSkiplist* sk, gpointer key);

gpointer gskiplist_search(GSkiplist* sk, gpointer key);

gpointer gskiplist_get_first_value(GSkiplist* sk);

void gskiplist_clean(GSkiplist* sk);



#ifdef __cplusplus
}
#endif


