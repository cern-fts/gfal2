#pragma once
#ifndef GRIDFTP_STREAMBUF_H
#define GRIDFTP_STREAMBUF_H

#include <iostream>
#include "../gridftpwrapper.h"

class GridftpStreamBuffer: public std::streambuf {
protected:
    GridFTP_stream_state* gstream;
    char buffer[4096];

    Glib::Quark quark;

    ssize_t fetch_more() {
        ssize_t rsize = gridftp_read_stream(quark, gstream, buffer, sizeof(buffer - 1));
        this->setg(buffer, buffer, buffer + rsize);
        return rsize;
    }

public:
    GridftpStreamBuffer(GridFTP_stream_state* gsiftp_stream, const Glib::Quark& quark):
        gstream(gsiftp_stream), quark(quark) {
        fetch_more();
    }

    virtual ~GridftpStreamBuffer() {
    }

    int_type underflow() {
        ssize_t rsize = fetch_more();
        if (rsize <= 0)
            return traits_type::eof();
        return *buffer;
    }
};

#endif // GRIDFTP_STREAMBUF_H
