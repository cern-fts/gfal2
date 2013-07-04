/*
 * 
 * Simple tools for unit tests
 * 
 * */

#define _GNU_SOURCE

#include <glib.h>
#include <stdio.h> 
#include <string.h>

#include "unit_test_util.h"

gboolean gfal2_tests_is_mock(){
#if USE_MOCK
	return TRUE;
#else
	return FALSE;
#endif
}



 
gboolean check_GList_Result_String(GList* list, char** example){	// return true if two string list are the same
	GList *tmp_list=list;
	while(tmp_list != NULL){
		if( tmp_list->data != NULL &&  *example != NULL){
			if(strcmp(tmp_list->data, *example) != 0){
				g_printerr(" error, the two string are different : %s, %s \n", (char*) tmp_list->data, (char*) *example);
				return FALSE;
			} 
		} else {
			if(tmp_list->data != *example){
					g_printerr(" one string is NULL but not the other : %s, %s \n", (char*) tmp_list->data, (char*) *example);
					return FALSE;
			}		
		}
		tmp_list= g_list_next(tmp_list);
		example++;
	}
	return TRUE;
	
} 



gboolean check_GList_Result_int(GList* list, int* example){	// return true if tab int and int GList are the same
	GList *tmp_list=list;
	while(tmp_list != NULL){
		if( GPOINTER_TO_INT(tmp_list->data) != *example){
			g_printerr(" error, the two integer are different : %d, %d \n", GPOINTER_TO_INT(tmp_list->data), *example);
			return FALSE;
		}
		tmp_list= g_list_next(tmp_list);
		example++;
	}
	return TRUE;
	
} 


static int internal_compare_string(gconstpointer a, gconstpointer b){
	return (a && b)?(strcmp((char*)a, (char*)b)):-1;
}

gboolean find_string(GList* strlist, const char* str){  // return true if str is present in the list
	return ( g_list_find_custom(strlist, str, &internal_compare_string) != NULL)?TRUE:FALSE;
}
