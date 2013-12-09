#include <errno.h>
#include <glib.h>
#include "gridftp_dir_reader.h"

static const Glib::Quark GridftpSimpleReaderQuark("GridftpSimpleListReader::readdir");


GridftpSimpleListReader::GridftpSimpleListReader(GridftpModule* gsiftp, const char* path):
    stream(NULL)
{
    GridFTPFactoryInterface* factory = gsiftp->get_session_factory();
    GridFTP_session* session = factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(path));

    stream = new GridFTP_stream_state(session);

    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridftpSimpleListReader::GridftpSimpleListReader]");
    Glib::Mutex::Lock locker(stream->lock);
    stream->start();
    globus_result_t res = globus_ftp_client_list(
            // start req
            stream->sess->get_ftp_handle(), path, NULL,
            globus_basic_client_callback,
            static_cast<GridFTP_Request_state*>(stream));
    gfal_globus_check_result(GridftpSimpleReaderQuark, res);

    stream_buffer = new GridftpStreamBuffer(stream, GridftpSimpleReaderQuark);

    gfal_log(GFAL_VERBOSE_TRACE, " <- [GridftpSimpleListReader::GridftpSimpleListReader]");
}


GridftpSimpleListReader::~GridftpSimpleListReader()
{
    delete stream_buffer;
    delete stream;
}


// try to extract dir information
static int gridftp_readdir_parser(const std::string& line, struct dirent* entry)
{
    strncpy(entry->d_name, line.c_str(), sizeof(entry->d_name));
    char *p = (char*)mempcpy(entry->d_name, line.c_str(), sizeof(entry->d_name));
    // clear new line madness
    *p = '\0';
    while (isspace(*p)) {
        *p = '\0';
        --p;
    }
    return 0;
}


struct dirent* GridftpSimpleListReader::readdir()
{
    Glib::Mutex::Lock locker(stream->lock);

    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridftpSimpleListReader::readdir]");

    std::string line;
    std::istream in(stream_buffer);
    if (!std::getline(in, line))
        return NULL;

    if (gridftp_readdir_parser(line, &dbuffer) != 0) {
        throw Glib::Error(GridftpSimpleReaderQuark, EINVAL, Glib::ustring("Error parsing GridFTP line: ").append(line));
    }

    gfal_log(GFAL_VERBOSE_VERBOSE, "  list file %s ", dbuffer.d_name);
    gfal_log(GFAL_VERBOSE_TRACE, "  [GridftpSimpleListReader::readdir] <- ");
    return &dbuffer;
}


struct dirent* GridftpSimpleListReader::readdirpp(struct stat* st)
{
    throw Glib::Error(GridftpSimpleReaderQuark, EBADF,
                      "Can not call readdirpp after simple readdir");
}
