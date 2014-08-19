#include "gridftp_dir_reader.h"

static const Glib::Quark GridftpListReaderQuark("GridftpSimpleListReader::readdir");

// From gridftp_ns_stat.cpp
extern globus_result_t parse_mlst_line(char *line, globus_gass_copy_glob_stat_t *stat_info,
        char *filename_buf, size_t filename_size);


GridftpListReader::GridftpListReader(GridftpModule* gsiftp, const char* path)
{
    GridFTPFactory* factory = gsiftp->get_session_factory();
    GridFTP_session* session = factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(path));

    stream = new GridFTP_stream_state(session);

    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridftpListReader::GridftpListReader]");
    Glib::Mutex::Lock locker(stream->lock);
    stream->start();
    globus_result_t res = globus_ftp_client_machine_list(
            stream->sess->get_ftp_handle(), path,
            stream->sess->get_op_attr_ftp(),
            globus_basic_client_callback,
            static_cast<GridFTP_Request_state*>(stream));
    gfal_globus_check_result(GridftpListReaderQuark, res);

    stream_buffer = new GridftpStreamBuffer(stream, GridftpListReaderQuark);

    gfal_log(GFAL_VERBOSE_TRACE, " <- [GridftpListReader::GridftpListReader]");
}


GridftpListReader::~GridftpListReader()
{
    delete stream_buffer;
    delete stream;
}


struct dirent* GridftpListReader::readdir()
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

struct dirent* GridftpListReader::readdirpp(struct stat* st)
{
    Glib::Mutex::Lock locker(stream->lock);

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
        throw Glib::Error(GridftpListReaderQuark, EINVAL, Glib::ustring("Error parsing GridFTP line: '").append(line).append("\'"));
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
