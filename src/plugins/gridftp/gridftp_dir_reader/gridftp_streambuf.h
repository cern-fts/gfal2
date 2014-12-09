#pragma once
#ifndef GRIDFTP_STREAMBUF_H
#define GRIDFTP_STREAMBUF_H

#include <iostream>
#include "../gridftpwrapper.h"

class GridFTPStreamBuffer: public std::streambuf {
protected:
    GridFTPStreamState* gstream;
    char buffer[4096];

    Glib::Quark quark;

    ssize_t fetch_more() {
        ssize_t rsize = gridftp_read_stream(quark, gstream, buffer, sizeof(buffer) - 1, false);
        this->setg(buffer, buffer, buffer + rsize);
        return rsize;
    }

public:
    GridFTPStreamBuffer(GridFTPStreamState* gsiftp_stream, const Glib::Quark& quark):
        gstream(gsiftp_stream), quark(quark) {
        fetch_more();
    }

    virtual ~GridFTPStreamBuffer() {
    }

    int_type underflow() {
        ssize_t rsize = fetch_more();
        if (rsize <= 0)
            return traits_type::eof();
        return *buffer;
    }
};

#endif // GRIDFTP_STREAMBUF_H
