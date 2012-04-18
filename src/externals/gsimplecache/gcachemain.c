/**
 * @brief Simple cache system based on Hashmap
 * @author Adev
 * @date 27/07/11
 * 
 * */

#define _GNU_SOURCE

#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "gcachemain.h"

static const guint64 max_list_len = MAX_LIST_LEN;




typedef struct _Internal_item{
	int ref_count;
	char item[];
} Internal_item;

struct _GSimpleCache_Handle{
	GHashTable*  table;
	GSimpleCache_CopyConstructor do_copy;
	size_t size_item;
	pthread_mutex_t mux;
};

static void gsimplecache_destroy_item_internal(gpointer a){
	Internal_item* i = (Internal_item*) a;
	g_free(i);
}


static gboolean hash_strings_are_equals(gconstpointer a, gconstpointer b){
	return (strcmp((char*) a, (char*) b)== 0);
}

/**
 * Construct a new cache with a capacity of max_size bytes
 * */
GSimpleCache* gsimplecache_new(guint64 max_size, GSimpleCache_CopyConstructor value_copy, size_t size_item){
	GSimpleCache* ret = (GSimpleCache*) g_new(struct _GSimpleCache_Handle,1);
	ret->table = g_hash_table_new_full(&g_str_hash, &hash_strings_are_equals, &free, &gsimplecache_destroy_item_internal );
	ret->do_copy = value_copy;
	ret->size_item = size_item;
	pthread_mutex_init(&ret->mux,NULL);
	return ret;
}

/**
 *  delete a cache object, all internals object are free
 * */
void gsimplecache_delete(GSimpleCache* cache){
	if(cache != NULL){
		pthread_mutex_lock(&cache->mux);
		g_hash_table_remove_all(cache->table);
		pthread_mutex_unlock(&cache->mux);
		pthread_mutex_destroy(&cache->mux);
		g_free(cache);
	}
}

Internal_item* gsimplecache_find_kstr_internal(GSimpleCache* cache, const char* key){
	Internal_item* ret = (Internal_item*) g_hash_table_lookup(cache->table, (gconstpointer) key);
	if(ret != NULL ){
		return ret;
	}	
	return NULL;
}

static gboolean gsimplecache_remove_internal_kstr(GSimpleCache* cache, const char* key){
		return g_hash_table_remove(cache->table, (gconstpointer) key);	
}




void gsimplecache_add_item_internal(GSimpleCache* cache, const char* key, void* item){
	Internal_item* ret = gsimplecache_find_kstr_internal(cache, key);	
	if(ret == NULL){
		ret = malloc(sizeof(struct _Internal_item) + cache->size_item);
		ret->ref_count = 2;
		cache->do_copy(item, ret->item);
		g_hash_table_insert(cache->table, strdup(key), ret);
	}else{
		(ret->ref_count)++;
	}
}


/**
 * Add an item to the cache or increment the reference of this item of one if already exist
 * */
void gsimplecache_add_item_kstr(GSimpleCache* cache, const char* key, void* item){
	pthread_mutex_lock(&cache->mux);	
	gsimplecache_add_item_internal(cache, key, item);
	pthread_mutex_unlock(&cache->mux);
}



/**
 * remove the item in the cache, return TRUE if removed else FALSE
 * destroy the internal item automatically
 * */
gboolean gsimplecache_remove_kstr(GSimpleCache* cache, const char* key){
	pthread_mutex_lock(&cache->mux);
	gboolean ret = gsimplecache_remove_internal_kstr(cache, key);
	pthread_mutex_unlock(&cache->mux);	
	return ret;
}

/**
 * find the value in the cache, and decrease its internal reference count of 1.
 * If the item exist, set the item resu to the correct value and return 0 else return -1
 * 
 * */
int gsimplecache_take_one_kstr(GSimpleCache* cache, const char* key, void* res){
	pthread_mutex_lock(&cache->mux);	
	Internal_item* ret = gsimplecache_find_kstr_internal(cache, key);
	if(ret){
		(ret->ref_count)--;
		cache->do_copy(ret->item, res);
		if(ret->ref_count <= 0)
			gsimplecache_remove_internal_kstr(cache, key);
	}
	pthread_mutex_unlock(&cache->mux);	
	return (ret)?0:-1;
}

