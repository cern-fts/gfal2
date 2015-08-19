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

#include <errno.h>
#include <glib.h>
#include "gridftp_dir_reader.h"

static const GQuark GridFTPSimpleReaderQuark = g_quark_from_static_string("GridftpSimpleListReader::readdir");


GridFTPSimpleListReader::GridFTPSimpleListReader(GridFTPModule* gsiftp, const char* path)
{
    GridFTPFactory* factory = gsiftp->get_session_factory();
    this->handler = new GridFTPSessionHandler(factory, path);
    this->request_state = new GridFTPRequestState(this->handler);
    this->stream_state = new GridFTPStreamState(this->handler);

    gfal2_log(G_LOG_LEVEL_DEBUG, " -> [GridftpSimpleListReader::GridftpSimpleListReader]");
    globus_result_t res = globus_ftp_client_list(
            // start req
            this->handler->get_ftp_client_handle(), path,
            this->handler->get_ftp_client_operationattr(),
            globus_ftp_client_done_callback,
            this->request_state);
    gfal_globus_check_result(GridFTPSimpleReaderQuark, res);

    stream_buffer = new GridFTPStreamBuffer(this->stream_state, GridFTPSimpleReaderQuark);

    gfal2_log(G_LOG_LEVEL_DEBUG, " <- [GridftpSimpleListReader::GridftpSimpleListReader]");
}


GridFTPSimpleListReader::~GridFTPSimpleListReader()
{
    this->request_state->wait(GridFTPSimpleReaderQuark);
}


// try to extract dir information
static int gridftp_readdir_parser(const std::string& line, struct dirent* entry)
{
    memset(entry->d_name, 0, sizeof(entry->d_name));
    g_strlcpy(entry->d_name, line.c_str(), sizeof(entry->d_name));
    char *p = stpncpy(entry->d_name, line.c_str(), sizeof(entry->d_name));
    // clear new line madness
    do {
        *p = '\0';
        --p;
    } while (p >= entry->d_name && isspace(*p));
    return 0;
}


struct dirent* GridFTPSimpleListReader::readdir()
{
    gfal2_log(G_LOG_LEVEL_DEBUG, " -> [GridftpSimpleListReader::readdir]");

    std::string line;
    std::istream in(stream_buffer);
    if (!std::getline(in, line))
        return NULL;

    if (gridftp_readdir_parser(line, &dbuffer) != 0) {
        throw Gfal::CoreException(GridFTPSimpleReaderQuark, EINVAL,
                std::string("Error parsing GridFTP line: ").append(line));
    }

    // Workaround for LCGUTIL-295
    // Some endpoints return the absolute path when listing an empty directory
    if (dbuffer.d_name[0] == '/' || dbuffer.d_name[0] == '\0')
        return NULL;

    gfal2_log(G_LOG_LEVEL_INFO, "  list file %s ", dbuffer.d_name);
    gfal2_log(G_LOG_LEVEL_DEBUG, "  [GridftpSimpleListReader::readdir] <- ");
    return &dbuffer;
}


struct dirent* GridFTPSimpleListReader::readdirpp(struct stat* st)
{
    throw Gfal::CoreException(GridFTPSimpleReaderQuark, EBADF,
            "Can not call readdirpp after simple readdir");
}
