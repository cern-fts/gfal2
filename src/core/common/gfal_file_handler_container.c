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

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <logger/gfal_logger.h>
#include "gfal_error.h"
#include "gfal_handle.h"
#include "gfal_file_handler_container.h"


// generate a new unique key
static int gfal_file_key_generatorG(gfal_file_handle_container fhandle, GError** err)
{
    g_return_val_err_if_fail(fhandle, 0, err,
            "[gfal_file_descriptor_generatorG] Invalid  arg file handle");
    int ret = rand();
    GHashTable* c = fhandle->container;
    if (g_hash_table_size(c) > G_MAXINT / 2) {
        gfal2_set_error(err, gfal2_get_plugins_quark(), EMFILE, __func__,
                "Too many files open");
        ret = 0;
    }
    else {
        while (ret == 0 || g_hash_table_lookup(c, GINT_TO_POINTER(ret)) != NULL) {
            ret = rand();
        }
    }
    return ret;
}

/*
 * Add the given file handle to the and return a file descriptor
 * return the associated key if success else 0 and set err
 */
int gfal_add_new_file_desc(gfal_file_handle_container fhandle, gpointer pfile,
        GError** err)
{
    g_return_val_err_if_fail(fhandle && pfile, 0, err,
            "[gfal_add_new_file_desc] Invalid  arg fhandle and/or pfile");
    pthread_mutex_lock(&(fhandle->m_container));
    GError* tmp_err = NULL;
    GHashTable* c = fhandle->container;
    int key = gfal_file_key_generatorG(fhandle, &tmp_err);
    if (key != 0) {
        g_hash_table_insert(c, GINT_TO_POINTER(key), pfile);
    }
    if (tmp_err) {
        gfal2_propagate_prefixed_error(err, tmp_err, __func__);
    }
    pthread_mutex_unlock(&(fhandle->m_container));
    return key;
}

// remove the associated file handle associated with the given file descriptor
// return true if success else false
gboolean gfal_remove_file_desc(gfal_file_handle_container fhandle, int key,
        GError** err)
{
    pthread_mutex_lock(&(fhandle->m_container));
    GHashTable* c = fhandle->container;
    gboolean p = g_hash_table_remove(c, GINT_TO_POINTER(key));
    if (!p)
        gfal2_set_error(err, gfal2_get_plugins_quark(), EBADF, __func__,
                "bad file descriptor");
    pthread_mutex_unlock(&(fhandle->m_container));
    return p;
}


//create a new file descriptor container with the given destroyer function to an element of the container
gfal_file_handle_container gfal_file_descriptor_handle_create(GDestroyNotify destroyer)
{
    gfal_file_handle_container d = g_malloc0(sizeof(struct _gfal_file_handle_container));
    d->container = g_hash_table_new_full(NULL, NULL, NULL, destroyer);
    pthread_mutex_init(&(d->m_container), NULL);
    return d;
}


void gfal_file_descriptor_handle_destroy(gfal_file_handle_container fhandle)
{
    if (fhandle->container) {
        g_hash_table_destroy(fhandle->container);
    }
    pthread_mutex_destroy(&fhandle->m_container);
    g_free(fhandle);
}


 /*
 *
 * return the file handle associated with the file_desc
 * @warning does not free the handle
 *
 * */
gfal_file_handle gfal_file_handle_bind(gfal_file_handle_container h,
        int fd, GError** err)
{
    g_return_val_err_if_fail(fd, 0, err, "invalid dir descriptor");


    pthread_mutex_lock(&(h->m_container));
    gpointer p = g_hash_table_lookup(h->container, GINT_TO_POINTER(fd));
    if (!p) {
        gfal2_set_error(err, gfal2_get_plugins_quark(), EBADF, __func__,
            "bad file descriptor");
    }
    pthread_mutex_unlock(&(h->m_container));
    return (gfal_file_handle)p;
}
