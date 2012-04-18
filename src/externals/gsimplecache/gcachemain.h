#pragma once
/**
 * @brief header for GCache
 * @author Adev
 * @date 27/07/11
 * 
 * */

#include <glib.h>


#define MAX_LIST_LEN 20000

/**
 * copy the original object to a new one
 */
typedef void (*GSimpleCache_CopyConstructor)(gpointer original, gpointer copy);

typedef struct _GSimpleCache_Handle GSimpleCache;

GSimpleCache* gsimplecache_new(guint64 max_size, GSimpleCache_CopyConstructor value_copy, size_t size_item);

void gsimplecache_delete(GSimpleCache* cache);

void gsimplecache_add_item_kstr(GSimpleCache* cache, const char* key, void* item);

int gsimplecache_take_one_kstr(GSimpleCache* cache, const char* key, void* res);

gboolean gsimplecache_remove_kstr(GSimpleCache* cache, const char* key);

