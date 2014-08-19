#include <errno.h>
#include <glib.h>
#include "gridftp_dir_reader.h"

static const Glib::Quark GridftpSimpleReaderQuark("GridftpSimpleListReader::readdir");


GridftpSimpleListReader::GridftpSimpleListReader(GridftpModule* gsiftp, const char* path):
    stream(NULL)
{
    GridFTPFactory* factory = gsiftp->get_session_factory();
    GridFTPSession* session = factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(path));

    stream = new GridFTPStreamState(session);

    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridftpSimpleListReader::GridftpSimpleListReader]");
    Glib::Mutex::Lock locker(stream->lock);
    stream->start();
    globus_result_t res = globus_ftp_client_list(
            // start req
            stream->sess->get_ftp_handle(), path,
            stream->sess->get_op_attr_ftp(),
            globus_basic_client_callback,
            static_cast<GridFTPRequestState*>(stream));
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
    memset(entry->d_name, 0, sizeof(entry->d_name));
    strncpy(entry->d_name, line.c_str(), sizeof(entry->d_name) - 1);
    char *p = stpncpy(entry->d_name, line.c_str(), sizeof(entry->d_name));
    // clear new line madness
    do {
        *p = '\0';
        --p;
    } while (p >= entry->d_name && isspace(*p));
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

    // Workaround for LCGUTIL-295
    // Some endpoints return the absolute path when listing an empty directory
    if (dbuffer.d_name[0] == '/' || dbuffer.d_name[0] == '\0')
        return NULL;

    gfal_log(GFAL_VERBOSE_VERBOSE, "  list file %s ", dbuffer.d_name);
    gfal_log(GFAL_VERBOSE_TRACE, "  [GridftpSimpleListReader::readdir] <- ");
    return &dbuffer;
}


struct dirent* GridftpSimpleListReader::readdirpp(struct stat* st)
{
    throw Glib::Error(GridftpSimpleReaderQuark, EBADF,
                      "Can not call readdirpp after simple readdir");
}
