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

    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridftpSimpleListReader::GridftpSimpleListReader]");
    globus_result_t res = globus_ftp_client_list(
            // start req
            this->handler->get_ftp_client_handle(), path,
            this->handler->get_ftp_client_operationattr(),
            globus_ftp_client_done_callback,
            this->request_state);
    gfal_globus_check_result(GridFTPSimpleReaderQuark, res);

    stream_buffer = new GridFTPStreamBuffer(this->stream_state, GridFTPSimpleReaderQuark);

    gfal_log(GFAL_VERBOSE_TRACE, " <- [GridftpSimpleListReader::GridftpSimpleListReader]");
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
    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridftpSimpleListReader::readdir]");

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

    gfal_log(GFAL_VERBOSE_VERBOSE, "  list file %s ", dbuffer.d_name);
    gfal_log(GFAL_VERBOSE_TRACE, "  [GridftpSimpleListReader::readdir] <- ");
    return &dbuffer;
}


struct dirent* GridFTPSimpleListReader::readdirpp(struct stat* st)
{
    throw Gfal::CoreException(GridFTPSimpleReaderQuark, EBADF,
            "Can not call readdirpp after simple readdir");
}
