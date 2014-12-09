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
