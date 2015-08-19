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
#ifndef GRIDFTP_STREAMBUF_H
#define GRIDFTP_STREAMBUF_H

#include <iostream>
#include "../gridftpwrapper.h"

class GridFTPStreamBuffer: public std::streambuf {
protected:
    GridFTPStreamState* gstream;
    char buffer[4096];

    GQuark quark;

    ssize_t fetch_more() {
        ssize_t rsize = gridftp_read_stream(quark, gstream, buffer, sizeof(buffer) - 1, false);
        this->setg(buffer, buffer, buffer + rsize);
        return rsize;
    }

public:
    GridFTPStreamBuffer(GridFTPStreamState* gsiftp_stream, GQuark quark):
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
