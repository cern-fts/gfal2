#include <logger/gfal_logger.h>
#include <regex.h>
#include "gfal_lfc.h"
#include "file/gfal_file_api.h"
#include "lfc_ifce_ng.h"

struct size_and_checksum {
    u_signed64 filesize;
    char       csumtype[3];
    char       csumvalue[33];
};


/**
 * Check URLs
 */
int gfal_lfc_register_check(plugin_handle handle, gfal2_context_t context, const char* src_url,
        const char* dst_url, gfal_url2_check check)
{
    struct lfc_ops* ops = (struct lfc_ops*) handle;

    if (check != GFAL_FILE_COPY)
        return 0;

    return regexec(&(ops->rex), dst_url, 0, NULL, 0) == 0;
}

/**
 * Create path in the LFC, including the creation of the directories
 */
static int _lfc_touch(struct lfc_ops* ops, const char* path, const char* guid,
        struct size_and_checksum* info, GError** error)
{
    char *last_slash = NULL;
    int   ret_status = 0;

    gfal_log(GFAL_VERBOSE_VERBOSE, "lfc register: trying to create %s", path);

    last_slash = strrchr(path, '/');
    if (last_slash != NULL) {
        size_t dir_len = last_slash - path;
        char *dir = g_malloc0(dir_len);
        strncpy(dir, path, dir_len);

        gfal_log(GFAL_VERBOSE_VERBOSE, "lfc register: checking parent directory %s", dir);

        if (ops->access(dir, F_OK) != 0) {
            gfal_log(GFAL_VERBOSE_VERBOSE, "lfc register: parent directory does not exist, creating", dir);
            ret_status = gfal_lfc_ifce_mkdirpG(ops, dir, 0755, TRUE, error);
        }
        g_free(dir);
    }

    if (ret_status == 0) {
        gfal_log(GFAL_VERBOSE_VERBOSE, "lfc register: creating the file");
        ret_status = ops->creatg(path, guid, 0644);
        if (ret_status != 0) {
            gfal2_set_error(error, gfal2_get_plugins_quark(), errno, __func__,
                        "Could not create the file: %s", gfal_lfc_get_strerror(ops));
        }
        else {
            ret_status = ops->setfsizeg(guid, info->filesize, info->csumtype, info->csumvalue);
            if (ret_status != 0) {
                gfal2_set_error(error, gfal2_get_plugins_quark(), errno, __func__,
                            "Could not set file size and checksum: %s", gfal_lfc_get_strerror(ops));
            }
        }
    }

    return ret_status;
}

/**
 * Set host to point to the host part of the url
 */
static int _get_host(const char* url, char** host, GError** error)
{
    regex_t    regex;
    size_t     ngroups = 4;
    regmatch_t matches[ngroups];
    int        ret = 0;

    regcomp(&regex, "(.+://([a-zA-Z0-9\\.-]+))(:[0-9]+)?/.+", REG_EXTENDED);

    ret = regexec(&regex, url, ngroups, matches, 0);
    if (ret == 0) {
        size_t host_len = matches[2].rm_eo - matches[2].rm_so;
        *host = g_malloc0(host_len + 1);
        strncpy(*host, url + matches[2].rm_so, host_len);
    }
    else {
        char err_buffer[64];
        regerror(ret, &regex, err_buffer, sizeof(err_buffer));

        gfal2_set_error(error, gfal2_get_plugins_quark(), EINVAL, __func__,
                    "The source is not a valid url: %s (%s)", url, err_buffer);
        ret = -1;
    }

    regfree(&regex);

    return ret;
}

/**
 * Full checksum type from an LFC-like checksum type
 */
static const char* _full_checksum_type(const char* short_name)
{
    if (strcmp("AD", short_name) == 0)
        return "ADLER32";
    else if (strcmp("MD", short_name) == 0)
        return "MD5";
    else
        return "CS";
}

/**
 * Get checksum and file size from the source replica
 */
int _get_replica_info(gfal2_context_t context, struct size_and_checksum* info,
        const char* replica_url, GError** error)
{
    memset(info, 0, sizeof(*info));

    struct stat replica_stat;
    if (gfal2_stat(context, replica_url, &replica_stat, error) != 0)
        return -1;
    info->filesize = replica_stat.st_size;

    const char *lfc_checksums[] = {"AD", "MD", "CS", NULL};
    unsigned i;

    for (i = 0; lfc_checksums[i] != NULL; ++i) {
        if (gfal2_checksum(context, replica_url, _full_checksum_type(lfc_checksums[i]),
                           0, 0, info->csumvalue, sizeof(info->csumvalue),
                           NULL) == 0) {
            memcpy(info->csumtype, lfc_checksums[i], sizeof(info->csumtype));
            gfal_log(GFAL_VERBOSE_DEBUG, "found checksum %s:%s for the replica",
                    info->csumtype, info->csumvalue);
            break;
        }
    }
    return 0;
}

/**
 * Checks that the new replica matches the information in the catalog
 */
int _validate_new_replica(gfal2_context_t context, struct lfc_filestatg *statg,
        struct size_and_checksum* replica_info, GError** error)
{
    if (replica_info->filesize != statg->filesize) {
        gfal2_set_error(error, gfal2_get_plugin_lfc_quark(), EINVAL, __func__,
                    "Replica file size (%lld) and LFC file size (%lld) do not match",
                    replica_info->filesize, statg->filesize);
        return -1;
    }
    else {
        gfal_log(GFAL_VERBOSE_VERBOSE, "lfc register: file size match");
    }

    if (statg->csumvalue[0] != '\0' && replica_info->csumvalue[0] != '\0' &&
        strncmp(replica_info->csumtype, statg->csumtype, sizeof(statg->csumtype)) == 0) {
        if (strncmp(replica_info->csumvalue, statg->csumvalue, sizeof(statg->csumvalue)) != 0) {
            gfal2_set_error(error, gfal2_get_plugin_lfc_quark(), EINVAL, __func__,
                        "Replica checksum (%s) and LFC checksum (%s) do not match",
                        replica_info->csumvalue, statg->csumvalue);
            return -1;
        }
        else {
            gfal_log(GFAL_VERBOSE_VERBOSE, "lfc register: checksum match");
        }
    }
    else {
        gfal_log(GFAL_VERBOSE_VERBOSE, "lfc register: no checksum available to do the validation");
    }

    return 0;
}

/**
 * src_url can be anything
 * dst_url must be an LFC url
 */
int gfal_lfc_register(plugin_handle handle, gfal2_context_t context,
        gfalt_params_t params, const char* src_url, const char* dst_url, GError** error)
{
    struct lfc_ops* ops = (struct lfc_ops*) handle;
    char* lfc_host = NULL;
    char* lfc_path = NULL;
    char* src_host = NULL;
    int   ret_status = 0;
    int   lfc_errno = 0;
    gboolean file_existed = FALSE;
    GError *tmp_err = NULL;

    // Get URL components
    ret_status = url_converter(handle, dst_url, &lfc_host, &lfc_path, &tmp_err);
    if (ret_status != 0)
        goto register_end;

    ret_status = _get_host(src_url, &src_host, &tmp_err);
    if (ret_status != 0)
        goto register_end;

    gfal_log(GFAL_VERBOSE_DEBUG, "lfc register: %s -> %s:%s", src_url, lfc_host, lfc_path);

    // Information about the replica
    struct size_and_checksum replica_info;
    ret_status = _get_replica_info(context, &replica_info, src_url, &tmp_err);
    if (ret_status != 0)
        goto register_end;

    // Set up LFC environment
    ret_status = lfc_configure_environment(ops, lfc_host, &tmp_err);
    if (ret_status != 0)
        goto register_end;

    gfal_lfc_init_thread(ops);

    // Stat LFC entry
    struct lfc_filestatg statg;
    ret_status = ops->statg(lfc_path, NULL, &statg);
    lfc_errno = gfal_lfc_get_errno(ops);

    // File exists, validate the incoming replica
    if (ret_status == 0) {
        gfal_log(GFAL_VERBOSE_VERBOSE, "lfc register: lfc exists, validate");
        file_existed = TRUE;
        ret_status = _validate_new_replica(context, &statg, &replica_info, &tmp_err);
    }
    // File do not exist, try to create
    else if (lfc_errno == ENOENT) {
        gfal_generate_guidG(statg.guid, NULL);
        ret_status = _lfc_touch(ops, lfc_path, statg.guid, &replica_info, &tmp_err);
    }
    // Failure
    else {
        ret_status = -1;
        gfal2_set_error(error, gfal2_get_plugin_lfc_quark(), errno, __func__,
                    "Failed to stat the file: %s (%d)", gfal_lfc_get_strerror(ops), lfc_errno);
    }

    if (ret_status != 0)
        goto register_end;

    struct lfc_fileid unique_id = {{0}, 0};
    unique_id.fileid = statg.fileid;
    strncpy(unique_id.server, lfc_host, sizeof(unique_id.server) - 1);

    ret_status = ops->addreplica(statg.guid,
                                 file_existed?&unique_id:NULL,
                                 src_host, src_url,
                                 '-', 'P',
                                 NULL, NULL);
    if (ret_status != 0) {
        int err_code = gfal_lfc_get_errno(ops);

        if (err_code != EEXIST) {
            gfal2_set_error(error, gfal2_get_plugin_lfc_quark(), err_code, __func__,
                        "Could not register the replica : %s ", gfal_lfc_get_strerror(ops));
        }
        else {
            gfal_log(GFAL_VERBOSE_VERBOSE, "lfc register: the replica is already registered, that is ok");
            ret_status = 0;
        }
    }
    else {
        gfal_log(GFAL_VERBOSE_VERBOSE, "lfc register: done");
    }

register_end:
    if (tmp_err)
        gfal2_propagate_prefixed_error(error, tmp_err, __func__);
    g_free(lfc_host);
    g_free(lfc_path);
    g_free(src_host);
    return ret_status;
}

/**
 * Unregister a replica
 */
int gfal_lfc_unregister(plugin_handle handle, const char* url, const char* sfn, GError** error)
{
    struct lfc_ops* ops = (struct lfc_ops*) handle;
    int ret_status, lfc_errno;
    char *lfc_host = NULL;
    char *lfc_path = NULL;
    GError *tmp_err = NULL;

    ret_status = url_converter(handle, url, &lfc_host, &lfc_path, &tmp_err);
    if (ret_status < 0)
        goto unregister_end;

    ret_status = lfc_configure_environment(ops, lfc_host, &tmp_err);
    if (ret_status != 0)
        goto unregister_end;

    struct lfc_filestatg statg;
    ret_status = ops->statg(lfc_path, NULL, &statg);

    if (ret_status != 0) {
        lfc_errno = gfal_lfc_get_errno(ops);
        gfal2_set_error(error, gfal2_get_plugin_lfc_quark(), lfc_errno, __func__,
                "Could not stat the file: %s (%d)", gfal_lfc_get_strerror(ops), lfc_errno);
        goto unregister_end;
    }

    gfal_log(GFAL_VERBOSE_VERBOSE, "lfc unregister: the replica is to be unregistered (file id %d)", statg.fileid);

    struct lfc_fileid file_id = {{0}, 0};
    file_id.fileid = statg.fileid;
    ret_status = ops->delreplica(NULL, &file_id, sfn);
    if (ret_status < 0) {
        lfc_errno = gfal_lfc_get_errno(ops);
        gfal2_set_error(error, gfal2_get_plugin_lfc_quark(), lfc_errno, __func__,
                "Could not register the replica : %s (%d) ", gfal_lfc_get_strerror(ops), lfc_errno);
    }

    gfal_log(GFAL_VERBOSE_VERBOSE, "lfc unregister: replica %s unregistered", sfn);

unregister_end:
    g_free(lfc_host);
    g_free(lfc_path);
    if (tmp_err)
        gfal2_propagate_prefixed_error(error, tmp_err, __func__);
    return ret_status;
}
