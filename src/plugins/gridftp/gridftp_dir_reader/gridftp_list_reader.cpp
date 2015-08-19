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

#include "gridftp_dir_reader.h"

static const GQuark GridftpListReaderQuark = g_quark_from_static_string("GridftpSimpleListReader::readdir");

// From gridftp_ns_stat.cpp
extern globus_result_t parse_mlst_line(char *line, globus_gass_copy_glob_stat_t *stat_info,
        char *filename_buf, size_t filename_size);


GridFTPListReader::GridFTPListReader(GridFTPModule* gsiftp, const char* path)
{
    GridFTPFactory* factory = gsiftp->get_session_factory();

    this->handler = new GridFTPSessionHandler(factory, path);
    this->request_state = new GridFTPRequestState(this->handler);
    this->stream_state = new GridFTPStreamState(this->handler);

    gfal2_log(G_LOG_LEVEL_DEBUG, " -> [GridftpListReader::GridftpListReader]");

    globus_result_t res = globus_ftp_client_machine_list(
            this->handler->get_ftp_client_handle(), path,
            this->handler->get_ftp_client_operationattr(),
            globus_ftp_client_done_callback,
            this->request_state);
    gfal_globus_check_result(GridftpListReaderQuark, res);

    this->stream_buffer = new GridFTPStreamBuffer(this->stream_state, GridftpListReaderQuark);

    gfal2_log(G_LOG_LEVEL_DEBUG, " <- [GridftpListReader::GridftpListReader]");
}


GridFTPListReader::~GridFTPListReader()
{
    this->request_state->wait(GridftpListReaderQuark);
}


struct dirent* GridFTPListReader::readdir()
{
    struct stat _;
    return readdirpp(&_);
}


static std::string& ltrim(std::string& str)
{
    size_t i = 0;
    while (i < str.length() && isspace(str[i]))
        ++i;
    str = str.substr(i);
    return str;
}


static std::string& rtrim(std::string& str)
{
    int i = str.length() - 1;
    while (i >= 0 && isspace(str[i]))
        --i;
    str = str.substr(0, i + 1);
    return str;
}

static std::string& trim(std::string& str)
{
    return ltrim(rtrim(str));
}


struct dirent* GridFTPListReader::readdirpp(struct stat* st)
{
    std::string line;
    std::istream in(stream_buffer);
    if (!std::getline(in, line))
        return NULL;

    if (trim(line).empty())
        return NULL;

    globus_gass_copy_glob_stat_t gl_stat;
    char* unparsed = strdup(line.c_str());
    if (parse_mlst_line(unparsed, &gl_stat, dbuffer.d_name, sizeof(dbuffer.d_name)) != GLOBUS_SUCCESS) {
        free(unparsed);
        throw Gfal::CoreException(GridftpListReaderQuark, EINVAL,
                std::string("Error parsing GridFTP line: '").append(line).append("\'"));
    }
    free(unparsed);

    // Workaround for LCGUTIL-295
    // Some endpoints return the absolute path when listing an empty directory
    if (dbuffer.d_name[0] == '/')
        return NULL;

    memset(st, 0, sizeof(*st));
    st->st_mode  = (mode_t) ((gl_stat.mode != -1)?gl_stat.mode:0);
    st->st_mode |= (gl_stat.type == GLOBUS_GASS_COPY_GLOB_ENTRY_DIR)?(S_IFDIR):(S_IFREG);
    st->st_size  = (off_t) gl_stat.size;
    st->st_mtime = (time_t) (gl_stat.mdtm != -1)?(gl_stat.mdtm):0;

    if (S_ISDIR(st->st_mode))
        dbuffer.d_type = DT_DIR;
    else if (S_ISLNK(st->st_mode))
        dbuffer.d_type = DT_LNK;
    else
        dbuffer.d_type = DT_REG;

    globus_libc_free(gl_stat.unique_id);
    globus_libc_free(gl_stat.symlink_target);

    return &dbuffer;
}
