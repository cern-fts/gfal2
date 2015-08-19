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

#pragma once
#ifndef GRIDFTP_DIR_READER_H
#define GRIDFTP_DIR_READER_H

#include <dirent.h>
#include <sys/stat.h>

#include "gridftp_streambuf.h"
#include "../gridftpmodule.h"

// Directory reader interface
class GridFTPDirReader {
protected:
    struct dirent dbuffer;

    GridFTPSessionHandler* handler;
    GridFTPRequestState* request_state;
    GridFTPStreamState *stream_state;
    GridFTPStreamBuffer  *stream_buffer;

public:
    GridFTPDirReader():
        handler(NULL), request_state(NULL), stream_state(NULL), stream_buffer(NULL)
    {
        memset(&dbuffer, 0, sizeof(dbuffer));
    };

    virtual ~GridFTPDirReader() {
        delete this->stream_buffer;
        delete this->stream_state;
        delete this->request_state;
        delete this->handler;
    };
    virtual struct dirent* readdir() = 0;
    virtual struct dirent* readdirpp(struct stat* st) = 0;
};

// Implementation for simple list
class GridFTPSimpleListReader: public GridFTPDirReader {
public:
    GridFTPSimpleListReader(GridFTPModule* gsiftp, const char* path);
    ~GridFTPSimpleListReader();
    struct dirent* readdir();
    struct dirent* readdirpp(struct stat* st);
};

// Implementation for MLSD
class GridFTPListReader: public GridFTPDirReader {
public:
    GridFTPListReader(GridFTPModule* gsiftp, const char* path);
    ~GridFTPListReader();
    struct dirent* readdir();
    struct dirent* readdirpp(struct stat* st);
};

#endif /* GRIDFTP_DIR_READER_H */
