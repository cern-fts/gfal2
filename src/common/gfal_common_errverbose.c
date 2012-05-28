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

/**
 * @file gfal_common_errverbose.c
 * @brief error management and verbose display
 * @author Devresse Adrien
 * */

#include <malloc.h>
#include <glib.h>
#include <common/gfal_common_errverbose.h>


// internal err buff for print
__thread char _gfal_err[GFAL_ERRMSG_LEN];

 
 /**
 * \brief display the full GError message on stderr and free the memory associated
 */
void gfal_release_GError(GError** err)
 {
	 if(err==NULL || *err==NULL){
		 gfal_log(GFAL_VERBOSE_DEBUG," release NULL error");
		 return;
	 }
	 g_printerr("[gfal] %s\n", (*err)->message);
	 g_clear_error(err);
	 *err=NULL;	 
 }
 
 
/**
 *  return a valid string of the current error, 
 *  @warning Modifications or free() on this string can lead to an undefined behaviors.
 *  @warning : like strerror, not thread safe.
 * */
char* gfal_str_GError(GError** err){
	return gfal_str_GError_r(err, _gfal_err, GFAL_ERRMSG_LEN);
 }

/**
 *  set buff_err to the current gfal error, reentrant function
 *  @return pointer to buff_err for convenience
 */
char* gfal_str_GError_r(GError** err, char* buff_err, size_t s_err){
	if(err==NULL || *err==NULL){
		 gfal_log(GFAL_VERBOSE_DEBUG,"copy string NULL error");
		 g_strlcpy(buff_err,"[gfal] No Error reported", s_err);
	}else{
		 g_strlcpy(buff_err,"[gfal]", s_err);
		 g_strlcat(buff_err, (*err)->message, s_err);
	}
	return buff_err;	
 }
/**
 *  @brief convenient way to manage Gerror
 *  If error does not exist, just return FALSE else print error on stderr, clear it and return TRUE
 * */
gboolean gfal_check_GError(GError** err){
	if(err==NULL || *err==NULL)
		return FALSE;
	g_printerr("[gfal] %s\n", (*err)->message);
	g_clear_error(err);
	return TRUE;
}
 
#if (GLIB_CHECK_VERSION(2,16,0) != TRUE)			// add code of glib 2.16 for link with a very old glib version
static void
g_error_add_prefix (gchar       **string,
                    const gchar  *format,
                    va_list       ap)
{
  gchar *oldstring;
  gchar *prefix;

  prefix = g_strdup_vprintf (format, ap);
  oldstring = *string;
  *string = g_strconcat (prefix, oldstring, NULL);
  g_free (oldstring);
  g_free (prefix);
}
 
void
g_propagate_prefixed_error (GError      **dest,
                            GError       *src,
                            const gchar  *format,
                            ...)
{
  g_propagate_error (dest, src);

  if (dest && *dest)
    {
      va_list ap;

      va_start (ap, format);
      g_error_add_prefix (&(*dest)->message, format, ap);
      va_end (ap);
    }
}

void
g_prefix_error (GError      **err,
                const gchar  *format,
                ...)
{
  if (err && *err)
    {
      va_list ap;

      va_start (ap, format);
      g_error_add_prefix (&(*err)->message, format, ap);
      va_end (ap);
    }
}
 
 #endif
 
 
 
 #if (GLIB_CHECK_VERSION(2,28,0) != TRUE)


void g_list_free_full(GList *list, GDestroyNotify free_func)
{
    GList* tmp_list= list;
	while( tmp_list != NULL){
		free_func(tmp_list->data);
		tmp_list = g_list_next(tmp_list);		
	}
	g_list_free(list);
}

#endif
 


