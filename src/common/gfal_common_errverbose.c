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
 * @brief the file of the common lib for error management and verbose display
 * @author Devresse Adrien
 * @version 0.0.1
 * @date 8/04/2011
 * */

#include <malloc.h>
#include <glib.h>
#include "gfal_common_errverbose.h"


/*
 * Verbose level
 *   API mode (no messages on stderr) by default
 *   CLI has to set it to '0' to get normal error messages
 */
static int gfal_verbose = 0;
// internal err buff for print
__thread char _gfal_err[GFAL_ERRMSG_LEN];

/**
 * \brief return verbose mode level
 */
extern int gfal_get_verbose(){
	return gfal_verbose;
}

/**
 * set the verbose level of gfal 2
 */
extern int gfal_set_verbose (int value)
{
    if (value < 0)
        return (-1);
    gfal_verbose = value;
    return (0);
}

/**
 * \brief display a verbose message 
 * 
 * msg is displayed if current verbose level is superior to verbose mode specified
 * 
 */
 extern void gfal_print_verbose(int verbose_lvl, const char* msg, ...){
	 if(verbose_lvl & gfal_get_verbose() ){
			char tab[2048];
			sprintf(tab,"[GFAL] [VERBOSE] %s\n",msg);
			va_list args;
			va_start(args, msg);
			vprintf(tab, args); 
			va_end(args);		 
	 }

 }
 
 /**
 * \brief display the full GError message on stderr and free the memory associated
 */
 extern void gfal_release_GError(GError** err)
 {
	 if(err==NULL || *err==NULL){
		 gfal_print_verbose(GFAL_VERBOSE_DEBUG," release NULL error");
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
		 gfal_print_verbose(GFAL_VERBOSE_DEBUG,"copy string NULL error");
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
 


