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

    // initiate reading stream
    ssize_t r_size = gridftp_read_stream(GridftpSimpleReaderQuark,
                        stream, buffer, sizeof(buffer) - 1);
    *(buffer + r_size) = '\0';
    list = std::string(buffer);

    gfal_log(GFAL_VERBOSE_TRACE, " <- [GridftpSimpleListReader::GridftpSimpleListReader]");
}


GridftpSimpleListReader::~GridftpSimpleListReader()
{
    delete stream;
}


// try to extract dir information
int GridftpSimpleListReader::readdirParser()
{
    const char * c_list = list.c_str();
    char* p, *p1;
    if ((p = strchr((char*) c_list, '\n')) == NULL)
        return 0; // no new entry, c'est la fin des haricots
    p1 = (char*) mempcpy(dbuffer.d_name, c_list,
                         std::min((long) NAME_MAX - 1, (long) (p - c_list)));
    *p1 = '\0';
    while (*(--p1) == '\r' || *p1 == '\n') // clear new line madness
        *p1 = '\0';
    list = std::string(p + 1);
    return 1;
}


struct dirent* GridftpSimpleListReader::readdir()
{
    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridftpSimpleListReader::readdir]");
    Glib::Mutex::Lock locker(stream->lock);

    while (readdirParser() == 0) {
        ssize_t r_size;
        if ((r_size = gridftp_read_stream(GridftpSimpleReaderQuark,
                          stream, buffer, sizeof(buffer - 1))) == 0) // end of stream
            return NULL;
        *(buffer + r_size) = '\0';
        list += std::string(buffer);
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
