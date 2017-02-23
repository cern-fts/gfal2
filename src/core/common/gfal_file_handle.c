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

#include "gfal_file_handle.h"
#include "gfal_file_handler_container.h"


gfal_file_handle gfal_file_handle_new(const char* module_name, gpointer fdesc)
{
    gfal_file_handle f = g_new(struct _gfal_file_handle, 1);
    g_strlcpy(f->module_name, module_name, GFAL_MODULE_NAME_SIZE);
    f->lock = g_mutex_new();
    f->offset = 0;
    f->fdesc = fdesc;
    f->ext_data = NULL;
    f->path = NULL;
    return f;
}


gfal_file_handle gfal_file_handle_new2(const char *module_name, gpointer fdesc,
    gpointer user_data, const char *file_path)
{
    gfal_file_handle f = gfal_file_handle_new(module_name, fdesc);
    if (file_path)
        f->path = g_strdup(file_path);
    f->ext_data = user_data;
    return f;
}


gpointer gfal_file_handle_get_fdesc(gfal_file_handle fh)
{
    return fh->fdesc;
}


void gfal_file_handle_set_fdesc(gfal_file_handle fh, gpointer fdesc)
{
    fh->fdesc = fdesc;
}


gpointer gfal_file_handle_get_user_data(gfal_file_handle fh)
{
    return fh->ext_data;
}


const gchar* gfal_file_handle_get_path(gfal_file_handle fh)
{
    return fh->path;
}

void gfal_file_handle_lock(gfal_file_handle fh)
{
    g_assert(fh);
    g_mutex_lock(fh->lock);
}


void gfal_file_handle_unlock(gfal_file_handle fh)
{
    g_assert(fh);
    g_mutex_unlock(fh->lock);
}


//  Delete a gfal_file handle
void gfal_file_handle_delete(gfal_file_handle fh)
{
    if (fh) {
        g_mutex_free(fh->lock);
        g_free(fh->path);
        g_free(fh);
    }
}
