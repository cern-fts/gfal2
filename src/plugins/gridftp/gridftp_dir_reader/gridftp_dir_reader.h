#pragma once
#ifndef GRIDFTP_DIR_READER_H
#define GRIDFTP_DIR_READER_H

#include <dirent.h>
#include <sys/stat.h>

#include "gridftp_streambuf.h"
#include "../gridftpmodule.h"

// Directory reader interface
class GridftpDirReader {
protected:
    struct dirent dbuffer;

public:
    virtual ~GridftpDirReader() {};
    virtual struct dirent* readdir() = 0;
    virtual struct dirent* readdirpp(struct stat* st) = 0;
};

// Implementation for simple list
class GridftpSimpleListReader: public GridftpDirReader {
protected:
    GridFTPStreamState *stream;
    GridftpStreamBuffer  *stream_buffer;

public:
    GridftpSimpleListReader(GridFTPModule* gsiftp, const char* path);
    ~GridftpSimpleListReader();
    struct dirent* readdir();
    struct dirent* readdirpp(struct stat* st);
};

// Implementation for MLSD
class GridftpListReader: public GridftpDirReader {
protected:
    GridFTPStreamState *stream;
    GridftpStreamBuffer  *stream_buffer;

public:
    GridftpListReader(GridFTPModule* gsiftp, const char* path);
    ~GridftpListReader();
    struct dirent* readdir();
    struct dirent* readdirpp(struct stat* st);
};

#endif /* GRIDFTP_DIR_READER_H */
