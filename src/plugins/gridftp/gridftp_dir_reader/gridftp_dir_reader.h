#pragma once
#ifndef GRIDFTP_DIR_READER_H
#define GRIDFTP_DIR_READER_H

#include <dirent.h>
#include <sys/stat.h>

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
    GridFTP_stream_state *stream;
    char buffer[65000];
    std::string list;

    int readdirParser();

public:
    GridftpSimpleListReader(GridftpModule* gsiftp, const char* path);
    ~GridftpSimpleListReader();
    struct dirent* readdir();
    struct dirent* readdirpp(struct stat* st);
};

// Implementation for MLST
class GridftpMlstReader: public GridftpDirReader {
public:
    ~GridftpMlstReader();
    struct dirent* readdir();
    struct dirent* readdirpp(struct stat* st);
};

#endif /* GRIDFTP_DIR_READER_H */
