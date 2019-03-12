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

#pragma once
#ifndef GRIDFTOMODULE_H
#define GRIDFTOMODULE_H

#include <algorithm>
#include <memory>

#include <gfal_plugins_api.h>
#include <exceptions/gfalcoreexception.hpp>
#include <globus_gass_copy.h>


class GridFTPFactory;

class GridFTPModule {
public:
    GridFTPModule(GridFTPFactory *);
    ~GridFTPModule();

    bool exists(const char* path);

    // Execute an access call, map on stat due to protocol restrictions
    void access(const char* path, int mode);

    // Execute a chmod query on path
    void chmod(const char* path, mode_t mode);

    // Execute a open query on path
    gfal_file_handle open(const char* url, int flag, mode_t mode);

    // Execute a read/pread query
    ssize_t read(gfal_file_handle handle, void* buffer, size_t count);
    ssize_t pread(gfal_file_handle handle, void* buffer, size_t count,
            off_t offset);

    // Execute a read/pread query
    ssize_t write(gfal_file_handle handle, const void* buffer,
            size_t count);
    ssize_t pwrite(gfal_file_handle handle, const void* buffer,
            size_t count, off_t offset);

    // seek a file
    off_t lseek(gfal_file_handle handle, off_t offset, int whence);

    // close a file
    int close(gfal_file_handle handle);

    //Execute a stat call on a gridftp URL
    void stat(const char* path, struct stat * st);

    // remove a file entry
    void unlink(const char* path);

    // Execute a mkdir query on path
    void mkdir(const char* path, mode_t mode);

    void checksum(const char* url, const char* check_type,
            char * checksum_buffer, size_t buffer_length, off_t start_offset,
            size_t data_length);

    // Rename
    void rename(const char* src, const char* dst);

    // rmdir query on path
    void rmdir(const char* path);

    // Query an extended attribute
    ssize_t getxattr(const char *path,
                  const char *name,
                  void *buff, size_t s_buff);

    void autoCleanFileCopy(gfalt_params_t params, int code,
            const char* dst);

    // Execute a file transfer operation for gridftp URLs
    void filecopy(gfalt_params_t params, const char* src,
            const char* dst);

    void internal_globus_gass_stat(const char* path,
            struct stat* fstat);

    GridFTPFactory* get_session_factory()
    {
        return _handle_factory;
    }

private:
    GridFTPFactory * _handle_factory;

};


void core_init();

#endif /* GRIDFTOMODULE_H */
