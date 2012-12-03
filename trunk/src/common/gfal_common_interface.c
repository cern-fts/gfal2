
/* 
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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

#include "gfal_common_interface.h"
#include "gfal_common_internal.h"

#if 0
/**
 * @file gfal_common_interface.c
 * @brief Wrapper file for the NON "G" functions
 * @author Devresse Adrien
 * @version 2.0
 * @date 12/04/2011
 * */

/**
 * @brief get a initiated gfal_handle
 * @return a gfal_handle, need to be free after usage. return NULL if errors
 * Wrapper of gfal_initG for the "without GLib" use case.
 * */ 
gfal_handle gfal_handle_new(){
	return gfal_initG(NULL);	
}


/**
 * @brief launch a surls-> turls translation in asynchronous mode
 * @warning need a initiaed gfal_handle
 * @param handle : the gfal_handle initiated ( \ref gfal_init )
 * @param surls : table of string of the differents surls to convert, NULL pointer must be the end of the table
 * @return return positive if success else -1, check GError for more information
 * Wrapper of gfal_get_asyncG for the "without GLib" use case.

int gfal_get_async(gfal_handle handle, char** surls){
	g_return_val_if_fail(handle != NULL,-1);
	GList *list=NULL;
	while(surls!= NULL){
		list = g_list_append(list, *surls);
		surls++;
	} 
	return gfal_get_asyncG(handle, list, &(handle->err));
}
 */

/**
 * @brief progress of the last request
 * @return return positive value if the current request is finished, 0 if false or -1 if error occured
 * Wrapper of gfal_async_request_is_finishedG for the "without GLib" use case.
 */
int gfal_async_request_is_finished(gfal_handle handle){
	g_return_val_if_fail(handle != NULL,-1);
	return (int) gfal_async_request_is_finishedG(handle, &(handle->err));
}


/**
 * get the result to the last get_async request
 * @return return the number of responses in turls or negative value if error
 * @param handle : handle of the current context
 * @param char*** turls : char** turls with the full list of answer, an answer with error is a NULL pointer
 * @warning turls need to be free manually 
 * @return return the number of turls in the table, else negative value if error
 * Wrapper for the "without GLib" use case.
 */
int gfal_get_async_results(gfal_handle handle, char*** turls){
	g_return_val_if_fail(handle != NULL && turls != NULL,-1);
	GList* resu = NULL;
	turls = NULL;
	const int ret = gfal_get_async_resultsG(handle, &resu, &(handle->err));
	if(ret >= 0){
		*turls = gfal_GList_to_tab(resu);	
	}	
	return ret;
}


/**
 * get the error code list associated with the turls of the last get_async request
 * @return return the number of responses in turls or negative value if error
 * @param handle : handle of the current context
 * @param int** turl_errcode : pointer to a table of int, set to NULL if error
 * @warning turl_errcode need to be free manually 
 * @return return the number of turls in the table, else negative value if error
 * Wrapper for gfal_get_async_get_results_errcodesG for the "without GLib" use case
 */
int gfal_get_async_results_errcodes(gfal_handle handle, int** turl_errcode){
	g_return_val_if_fail(handle != NULL && turl_errcode != NULL,-1);
	GList* resu = NULL;
	turl_errcode = NULL;
	const int ret = gfal_get_async_results_errcodesG(handle, &resu, &(handle->err));
	if(ret >= 0){
		*turl_errcode = gfal_GList_to_tab_int(resu);	
	}	
	return ret;	
}


/**
 * get the error string associated with the turls of the last get_async request
 * @return return the number of responses in turls or negative value if error
 * @param handle : handle of the current context
 * @param char*** turls : char** turls with the full list of answer, an answer with error is a NULL pointer
 * @warning turl_errstring need to be free manually 
 * @return return the number of turls in the table, else negative value if error
 * Wrapper for gfal_get_async_get_results_errstringG for  the "without GLib" use case.
 */
int gfal_get_async_results_errstring(gfal_handle handle, char*** turl_errstring){
	g_return_val_if_fail(handle != NULL && turl_errstring != NULL,-1);
	GList* resu = NULL;
	turl_errstring = NULL;
	const int ret = gfal_get_async_results_errstringG(handle, &resu, &(handle->err));
	if(ret >= 0){
		*turl_errstring = gfal_GList_to_tab(resu);	
	}	
	return ret;	
}

/**
 * set the bdii value of the handle specified
 * same function than gfal_set_nobdiiG, naming convention
 *  * Wrapper for gfal_set_nobdiiG for  the "without GLib" use case.
 */
void gfal_set_nobdii_srm(gfal_handle handle, int no_bdii_chk){
	gfal_set_nobdiiG(handle, (gboolean)no_bdii_chk);	
}

/**
 * @brief wait for the current request
 * @param handle
 * @param timeout : maximum time to wait before error
   @return return 0 if finished correctly, return 1 if timeout is reached, return -1 if error
  * Wrapper for gfal_wait_async_requestG for the "without GLib" use case.
 */
int gfal_wait_async_request(gfal_handle handle, long timeout){
	g_return_val_if_fail(handle != NULL, -1);
	return gfal_wait_async_requestG(handle, timeout, &(handle->err));
}

/**
 * @return string of the last error reported or NULL if no error is reported
 */
char* gfal_last_error_string(gfal_handle handle){
	g_return_val_if_fail(handle != NULL, NULL);
	return (handle->err)?(handle->err->message):NULL;
}

/**
 * @return return TRUE value if error occured else FALSE
 */
int gfal_has_error(gfal_handle handle){
	return (handle && handle->err)?TRUE:FALSE;	
}
#endif

