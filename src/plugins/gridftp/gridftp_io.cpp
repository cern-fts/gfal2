/*
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <sstream>

#include <exceptions/cpp_to_gerror.hpp>
#include "gridftp_io.h"
#include "gridftp_namespace.h"
#include "gridftp_plugin.h"

static Glib::Quark GFAL_GRIDFTP_SCOPE_OPEN("GridFTPModule::open");
static Glib::Quark GFAL_GRIDFTP_SCOPE_READ("GridFTPModule::read");
static Glib::Quark GFAL_GRIDFTP_SCOPE_INTERNAL_PREAD("GridFTPModule::internal_pread");
static Glib::Quark GFAL_GRIDFTP_SCOPE_WRITE("GridFTPModule::write");
static Glib::Quark GFAL_GRIDFTP_SCOPE_INTERNAL_PWRITE("GridFTPModule::internal_pwrite");
static Glib::Quark GFAL_GRIDFTP_SCOPE_LSEEK("GridFTPModule::lseek");
static Glib::Quark GFAL_GRIDFTP_SCOPE_CLOSE("GridFTPModule::close");


const size_t readdir_len = 65000;

struct GridFTPFileDesc {
    GridFTPStreamState* stream;
    GridFTPRequestState* req_state;
    int open_flags;
    off_t current_offset;
    std::string url;
    Glib::Mutex lock;

    GridFTPFileDesc(GridFTPStreamState* ss, GridFTPRequestState* rs, const std::string & _url, int flags) :
            stream(ss), req_state(rs)
    {
        gfal_log(GFAL_VERBOSE_TRACE, "create descriptor for %s", _url.c_str());
        this->open_flags = flags;
        current_offset = 0;
        url = _url;
    }

    virtual ~GridFTPFileDesc()
    {
        gfal_log(GFAL_VERBOSE_TRACE, "destroy descriptor for %s", url.c_str());
        delete stream;
        delete req_state;
    }

    bool is_not_seeked()
    {
        return (stream != NULL && current_offset == stream->get_offset());
    }

    bool is_eof()
    {
        return stream && stream->is_eof();
    }

    void reset()
    {
        delete stream;
        stream = NULL;
    }

};


inline bool is_read_only(int open_flags)
{
    return ((open_flags & O_RDONLY) || ((open_flags & (O_WRONLY | O_RDWR)) == 0));
}


inline bool is_write_only(int open_flags)
{
    return (open_flags & ( O_WRONLY | O_CREAT));
}


inline int gridftp_rw_commit_put(const Glib::Quark & scope,
        GridFTPFileDesc* desc)
{
    char buffer[2];
    if (is_write_only(desc->open_flags) && desc->stream && !desc->stream->is_eof()) {
        gfal_log(GFAL_VERBOSE_TRACE, "Commit change for the current stream PUT ... ");
        gridftp_write_stream(GFAL_GRIDFTP_SCOPE_WRITE, desc->stream, buffer, 0, true);
        desc->req_state->wait_callback(GFAL_GRIDFTP_SCOPE_WRITE);
        gfal_log(GFAL_VERBOSE_TRACE, "Committed with success ... ");
    }
    return 0;
}


inline int gridftp_rw_valid_get(const Glib::Quark & scope,
        GridFTPFileDesc* desc)
{
    if (is_read_only(desc->open_flags) && desc->stream) {
        if (desc->is_eof()) {
            desc->req_state->wait_callback(scope);
        }
        else {
            gfal_log(GFAL_VERBOSE_TRACE,
                    "Not a full read -> kill the connection ");
            try {
                desc->req_state->cancel_operation(scope,
                        "Not a full read, connection killed");
            }
            catch (Glib::Error & e) {
                // silent !!
            }
        }
    }
    return 0;
}

// internal pread, do a read query with offset on a different descriptor, do not change the position of the current one.
ssize_t gridftp_rw_internal_pread(GridFTPFactory * factory,
        GridFTPFileDesc* desc, void* buffer, size_t s_buff, off_t offset)
{
    // throw Gfal::CoreException
    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridFTPModule::internal_pread]");
    GridFTPStreamState stream(
            factory->gfal_globus_ftp_take_handle(
                    gridftp_hostname_from_url(desc->url.c_str())));

    globus_result_t res = globus_ftp_client_partial_get(
            // start req
            stream.sess->get_ftp_handle(), desc->url.c_str(),
            stream.sess->get_op_attr_ftp(),
            NULL, offset, offset + s_buff, globus_basic_client_callback,
            &stream);
    gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_INTERNAL_PREAD, res);

    stream.set_offset(offset);

    ssize_t r_size = gridftp_read_stream(GFAL_GRIDFTP_SCOPE_INTERNAL_PREAD, &stream, buffer, s_buff, true);
    stream.wait_callback(GFAL_GRIDFTP_SCOPE_INTERNAL_PREAD);
    gfal_log(GFAL_VERBOSE_TRACE, "[GridFTPModule::internal_pread] <-");
    return r_size;

}

// internal pwrite, do a write query with offset on a different descriptor, do not change the position of the current one.
ssize_t gridftp_rw_internal_pwrite(GridFTPFactory * factory,
        GridFTPFileDesc* desc, const void* buffer, size_t s_buff, off_t offset)
{ // throw Gfal::CoreException
    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridFTPModule::internal_pwrite]");
    GridFTPStreamState stream(
            factory->gfal_globus_ftp_take_handle(
                    gridftp_hostname_from_url(desc->url.c_str())));

    globus_result_t res = globus_ftp_client_partial_put(
            // start req
            stream.sess->get_ftp_handle(), desc->url.c_str(),
            stream.sess->get_op_attr_ftp(),
            NULL, offset, offset + s_buff, globus_basic_client_callback,
            static_cast<GridFTPRequestState*>(&stream));
    gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_INTERNAL_PWRITE, res);

    ssize_t r_size = gridftp_write_stream(GFAL_GRIDFTP_SCOPE_INTERNAL_PWRITE,
            &stream, buffer, s_buff, true); // write block

    stream.wait_callback(GFAL_GRIDFTP_SCOPE_INTERNAL_PWRITE);
    gfal_log(GFAL_VERBOSE_TRACE, "[GridFTPModule::internal_pwrite] <-");
    return r_size;

}

// gridFTP open is restricted by the protocol : READ or Write but not both
//
gfal_file_handle GridFTPModule::open(const char* url, int flag, mode_t mode)
{
    GridFTPSession* session = _handle_factory->gfal_globus_ftp_take_handle(gridftp_hostname_from_url(url));
    std::auto_ptr<GridFTPFileDesc> desc(
            new GridFTPFileDesc(
                    new GridFTPStreamState(session, false),
                    new GridFTPRequestState(session, true),
                    url,
                    flag));
    gfal_log(GFAL_VERBOSE_TRACE, " -> [GridFTPModule::open] ");
    globus_result_t res;
    if (is_read_only(desc->open_flags) // check ENOENT condition for R_ONLY
    && this->exists(url) == false) {
        char err_buff[2048];
        snprintf(err_buff, 2048, " gridftp open error : %s on url %s",
                strerror(ENOENT), url);
        throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_OPEN, err_buff, ENOENT);
    }

    if (is_read_only(desc->open_flags)) {// portability hack for O_RDONLY mask // bet on a full read
        gfal_log(GFAL_VERBOSE_TRACE,
                " -> initialize FTP GET global operations... ");
        desc->req_state->start();
        res = globus_ftp_client_get(
                // start req
                desc->req_state->sess->get_ftp_handle(), url,
                desc->req_state->sess->get_op_attr_ftp(),
                NULL, globus_basic_client_callback, desc->req_state);
        gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_OPEN, res);
    }
    else if (is_write_only(desc->open_flags)) {
        gfal_log(GFAL_VERBOSE_TRACE,
                " -> initialize FTP PUT global operations ... ");
        desc->req_state->start();
        res = globus_ftp_client_put(
                // bet on a full write
                desc->req_state->sess->get_ftp_handle(), url,
                desc->req_state->sess->get_op_attr_ftp(),
                NULL, globus_basic_client_callback, desc->req_state);
        gfal_globus_check_result(GFAL_GRIDFTP_SCOPE_OPEN, res);
    }
    else {
        gfal_log(GFAL_VERBOSE_TRACE,
                " -> no operation initialization, switch to partial read/write mode...");
        desc->reset();
    }

    gfal_log(GFAL_VERBOSE_TRACE, " <- [GridFTPModule::open] ");
    return gfal_file_handle_new2(gridftp_plugin_name(),
            (gpointer) desc.release(), NULL, url);
}


ssize_t GridFTPModule::read(gfal_file_handle handle, void* buffer, size_t count)
{
    GridFTPFileDesc* desc = static_cast<GridFTPFileDesc*>(gfal_file_handle_get_fdesc(handle));
    ssize_t ret;

    Glib::Mutex::Lock locker(desc->lock);
    if (desc->is_not_seeked() && is_read_only(desc->open_flags) && desc->stream != NULL) {
        gfal_log(GFAL_VERBOSE_TRACE, " read in the GET main flow ... ");
        ret = gridftp_read_stream(GFAL_GRIDFTP_SCOPE_READ, desc->stream, buffer, count, false);
    }
    else {
        gfal_log(GFAL_VERBOSE_TRACE, " read with a pread ... ");
        ret = gridftp_rw_internal_pread(_handle_factory, desc, buffer, count,
                desc->current_offset);
    }
    desc->current_offset += ret;
    return ret;
}


ssize_t GridFTPModule::write(gfal_file_handle handle, const void* buffer,
        size_t count)
{
    GridFTPFileDesc* desc = static_cast<GridFTPFileDesc*>(gfal_file_handle_get_fdesc(handle));
    ssize_t ret;

    Glib::Mutex::Lock locker(desc->lock);
    if (desc->is_not_seeked() && is_write_only(desc->open_flags) && desc->stream != NULL) {
        gfal_log(GFAL_VERBOSE_TRACE, " write in the PUT main flow ... ");
        ret = gridftp_write_stream(GFAL_GRIDFTP_SCOPE_WRITE, desc->stream,
                buffer, count, false);
    }
    else {
        gfal_log(GFAL_VERBOSE_TRACE, " write with a pwrite ... ");
        ret = gridftp_rw_internal_pwrite(_handle_factory, desc, buffer, count,
                desc->current_offset);
    }
    desc->current_offset += ret;
    return ret;
}


ssize_t GridFTPModule::pread(gfal_file_handle handle, void* buffer,
        size_t count, off_t offset)
{
    GridFTPFileDesc* desc = static_cast<GridFTPFileDesc*>(gfal_file_handle_get_fdesc(handle));
    return gridftp_rw_internal_pread(_handle_factory, desc, buffer, count, offset);
}


ssize_t GridFTPModule::pwrite(gfal_file_handle handle, const void* buffer,
        size_t count, off_t offset)
{
    GridFTPFileDesc* desc = static_cast<GridFTPFileDesc*>(gfal_file_handle_get_fdesc(handle));
    return gridftp_rw_internal_pwrite(_handle_factory, desc, buffer, count,
            offset);
}


off_t GridFTPModule::lseek(gfal_file_handle handle, off_t offset, int whence)
{
    GridFTPFileDesc* desc = static_cast<GridFTPFileDesc*>(gfal_file_handle_get_fdesc(handle));
    Glib::Mutex::Lock locker(desc->lock);

    // Calculate new offset
    off_t new_offset;
    switch (whence) {
        case SEEK_SET:
            new_offset = offset;
            break;
        case SEEK_CUR:
            new_offset = desc->current_offset + offset;
            break;
        case SEEK_END:
        default:
            throw Gfal::CoreException(GFAL_GRIDFTP_SCOPE_LSEEK, "Invalid whence", EINVAL);
    }

    // If the new offset is the same we have, we are good
    // This is done to avoid seeking when actually the reads/writes are done sequentially
    // This happens, for instance, with gfalFS, which will do parallel writes and reads, but
    // in order
    if (new_offset == desc->current_offset) {
        gfal_log(GFAL_VERBOSE_VERBOSE, "New and current offsets are the same (%lld), so do not seek",
                (long long)(new_offset));
        return desc->current_offset;
    }

    // If the new offset does not correspond with the current offset,
    // abort initial GET/PUT operation if running
    // When we seek, cancel the operation we start automatically when opening
    try {
        desc->req_state->cancel_operation(GFAL_GRIDFTP_SCOPE_LSEEK,
                "Not a full read, connection killed");
    }
    catch (Glib::Error & e) {
        // silent !!
    }
    desc->reset();
    desc->current_offset = new_offset;

    return desc->current_offset;
}


int GridFTPModule::close(gfal_file_handle handle)
{
    GridFTPFileDesc* desc = static_cast<GridFTPFileDesc*>(gfal_file_handle_get_fdesc(handle));
    if (desc) {
        gridftp_rw_commit_put(GFAL_GRIDFTP_SCOPE_CLOSE, desc);
        gridftp_rw_valid_get(GFAL_GRIDFTP_SCOPE_CLOSE, desc);
        gfal_file_handle_delete(handle);
        delete desc;
    }
    return 0;
}


// open C bind
extern "C" gfal_file_handle gfal_gridftp_openG(plugin_handle handle,
        const char* url, int flag, mode_t mode, GError** err)
{
    g_return_val_err_if_fail(handle != NULL && url != NULL, NULL, err,
            "[gfal_gridftp_openG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    gfal_file_handle ret = NULL;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_openG]");
    CPP_GERROR_TRY
                ret = ((static_cast<GridFTPModule*>(handle))->open(url, flag,
                        mode));
            CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_openG]<-");
    G_RETURN_ERR(ret, tmp_err, err);
}


extern "C" ssize_t gfal_gridftp_readG(plugin_handle ch, gfal_file_handle fd,
        void* buff, size_t s_buff, GError** err)
{
    g_return_val_err_if_fail(ch != NULL && fd != NULL, -1, err,
            "[gfal_gridftp_readG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_readG]");
    CPP_GERROR_TRY
                ret = (int) ((static_cast<GridFTPModule*>(ch))->read(fd, buff,
                        s_buff));
            CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_readG]<-");
    G_RETURN_ERR(ret, tmp_err, err);
}


extern "C" ssize_t gfal_gridftp_writeG(plugin_handle ch, gfal_file_handle fd,
        const void* buff, size_t s_buff, GError** err)
{
    g_return_val_err_if_fail(ch != NULL && fd != NULL, -1, err,
            "[gfal_gridftp_writeG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_writeG]");
    CPP_GERROR_TRY
                ret = (int) ((static_cast<GridFTPModule*>(ch))->write(fd, buff,
                        s_buff));
            CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_writeG] <-");
    G_RETURN_ERR(ret, tmp_err, err);
}


extern "C" int gfal_gridftp_closeG(plugin_handle ch, gfal_file_handle fd,
        GError** err)
{
    g_return_val_err_if_fail(ch != NULL && fd != NULL, -1, err,
            "[gfal_gridftp_closeG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    int ret = -1;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_closeG]");
    CPP_GERROR_TRY
                ret = ((static_cast<GridFTPModule*>(ch))->close(fd));
            CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_closeG]<-");
    G_RETURN_ERR(ret, tmp_err, err);

}


extern "C" off_t gfal_gridftp_lseekG(plugin_handle ch, gfal_file_handle fd,
        off_t offset, int whence, GError** err)
{
    g_return_val_err_if_fail(ch != NULL && fd != NULL, -1, err,
            "[gfal_gridftp_lseekG][gridftp] Invalid parameters");

    GError * tmp_err = NULL;
    off_t ret = -1;
    gfal_log(GFAL_VERBOSE_TRACE, "  -> [gfal_gridftp_lseekG]");
    CPP_GERROR_TRY
                ret = ((static_cast<GridFTPModule*>(ch))->lseek(fd, offset,
                        whence));
            CPP_GERROR_CATCH(&tmp_err);
    gfal_log(GFAL_VERBOSE_TRACE, "  [gfal_gridftp_lseekG]<-");
    G_RETURN_ERR(ret, tmp_err, err);

}
