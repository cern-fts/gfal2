/*
 * Copyright (c) CERN 2013-2015
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

#if  defined __i386__ ||  defined _M_IX86
#define __USE_LARGEFILE64 1
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include <regex.h>
#include <time.h>
#include <dlfcn.h>
#include "gfal_dcap_plugin_layer.h"

typedef int (*lstat_t)(const char *, struct stat *);
typedef struct dirent *(*readdir_t)(DIR *);
typedef int (*stat_t)(const char *, struct stat *);

struct dcap_proto_ops * gfal_dcap_internal_loader_base(GError** err)
{
    struct dcap_proto_ops * pops = NULL;
    GError* tmp_err = NULL;

    pops = g_new0(struct dcap_proto_ops, 1);
    pops->geterror = &__dc_errno;
    pops->strerror = &dc_strerror;
    pops->access = &dc_access;
    pops->chmod = &dc_chmod;
    pops->close = &dc_close;
    pops->closedir = &dc_closedir;
    pops->lseek = &dc_lseek;
    pops->lstat = (lstat_t) &dc_lstat;
    pops->mkdir = &dc_mkdir;
    pops->open = &dc_open;
    pops->opendir = &dc_opendir;
    pops->read = &dc_read;
    pops->pread = &dc_pread;
    pops->readdir = (readdir_t) &dc_readdir;
    pops->rename = NULL;
    pops->rmdir = &dc_rmdir;
    pops->stat = (stat_t) &dc_stat;
    pops->unlink = &dc_unlink;
    pops->write = &dc_write;
    pops->pwrite = &dc_pwrite;
    pops->debug_level = &dc_setDebugLevel;
    pops->active_mode = &dc_setClientActive;
    //
    pops->active_mode(); // switch to active mode to avoid firewalls problems
    if ((gfal2_log_get_level() >= G_LOG_LEVEL_DEBUG))
        pops->debug_level(8 | 6 | 32);

    G_RETURN_ERR(pops, tmp_err, err);
}

struct dcap_proto_ops * (*gfal_dcap_internal_loader)(GError** err)= &gfal_dcap_internal_loader_base;

