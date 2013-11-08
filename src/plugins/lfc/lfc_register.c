#include <regex.h>
#include "file/gfal_file_api.h"
#include "lfc_register.h"

struct size_and_checksum {
    u_signed64 filesize;
    char       csumtype[3];
    char       csumvalue[33];
};


/**
 * Check URLs
 */
int gfal_lfc_register_check(plugin_handle handle, const char* src_url,
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
int _lfc_touch(struct lfc_ops* ops, const char* path, const char* guid,
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
            g_set_error(error, gfal2_get_plugins_quark(), errno,
                        "[%s] Could not create the file: %s",
                        __func__, gfal_lfc_get_strerror(ops));
        }
        else {
            ret_status = ops->setfsizeg(guid, info->filesize, info->csumtype, info->csumvalue);
            if (ret_status != 0) {
                g_set_error(error, gfal2_get_plugins_quark(), errno,
                            "[%s] Could not set file size and checksum: %s",
                            __func__, gfal_lfc_get_strerror(ops));
            }
        }
    }

    return ret_status;
}

/**
 * Set host to point to the host part of the url
 */
int _get_host(const char* url, char** host, GError** error)
{
    regex_t    regex;
    size_t     ngroups = 4;
    regmatch_t matches[ngroups];
    int        ret = 0;

    regcomp(&regex, "(.+://([a-zA-Z0-9\\.-]+))(:\\d+)?/.+", REG_EXTENDED);

    ret = regexec(&regex, url, ngroups, matches, 0);
    if (ret == 0) {
        size_t host_len = matches[2].rm_eo - matches[2].rm_so;
        *host = g_malloc0(host_len + 1);
        strncpy(*host, url + matches[2].rm_so, host_len);
    }
    else {
        char err_buffer[64];
        regerror(ret, &regex, err_buffer, sizeof(err_buffer));

        g_set_error(error, gfal2_get_plugins_quark(), EINVAL,
                    "[%s] The destination is not a valid url: %s (%s)",
                    __func__, url, err_buffer);
        ret = -1;
    }

    regfree(&regex);

    return ret;
}

/**
 * Full checksum type from an LFC-like checksum type
 */
const char* _full_checksum_type(const char* short_name)
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
            strncpy(info->csumtype, lfc_checksums[i], sizeof(info->csumtype));
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
        g_set_error(error, gfal2_get_plugin_lfc_quark(),
                    EINVAL,
                    "[gfal_lfc_register] Replica file size (%lld) and LFC file size (%lld) do not match",
                    replica_info->filesize, statg->filesize);
        return -1;
    }

    if (statg->csumvalue[0] != '\0' && replica_info->csumvalue[0] != '\0' &&
        strncmp(replica_info->csumtype, statg->csumtype, sizeof(statg->csumtype)) == 0) {
        if (strncmp(replica_info->csumvalue, statg->csumvalue, sizeof(statg->csumvalue)) != 0) {
            g_set_error(error, gfal2_get_plugin_lfc_quark(),
                        EINVAL,
                        "[gfal_lfc_register] Replica checksum (%s) and LFC checksum (%s) do not match",
                        replica_info->csumvalue, statg->csumvalue);
            return -1;
        }
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

    // Get URL components
    ret_status = url_converter(handle, dst_url, &lfc_host, &lfc_path, error);
    if (ret_status != 0)
        goto register_end;

    ret_status = _get_host(src_url, &src_host, error);
    if (ret_status != 0)
        goto register_end;

    gfal_log(GFAL_VERBOSE_DEBUG, "lfc register: %s -> %s:%s", src_url, lfc_host, lfc_path);

    // Information about the replica
    struct size_and_checksum replica_info;
    ret_status = _get_replica_info(context, &replica_info, src_url, error);
    if (ret_status != 0)
        goto register_end;

    // Set up LFC environment
    ret_status = lfc_configure_environment(ops, lfc_host, error);
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
        ret_status = _validate_new_replica(context, &statg, &replica_info, error);
    }
    // File do not exist, try to create
    else if (lfc_errno == ENOENT) {
        gfal_generate_guidG(statg.guid, NULL);
        ret_status = _lfc_touch(ops, lfc_path, statg.guid, &replica_info, error);
    }
    // Failure
    else {
        ret_status = -1;
        g_set_error(error, gfal2_get_plugin_lfc_quark(),
                    errno, "[%s] Failed to stat the file: %s (%d)",
                    __func__, gfal_lfc_get_strerror(ops), lfc_errno);
    }

    if (ret_status != 0)
        goto register_end;

    struct lfc_fileid unique_id;
    unique_id.fileid = statg.fileid;
    strncpy(unique_id.server, lfc_host, sizeof(unique_id.server));

    ret_status = ops->addreplica(statg.guid,
                                 file_existed?&unique_id:NULL,
                                 src_host, src_url,
                                 '-', 'P',
                                 NULL, NULL);
    if (ret_status != 0) {
        g_set_error(error, gfal2_get_plugin_lfc_quark(),
                    errno, "[%s] Could not register the replica : %s ",
                    __func__, gfal_lfc_get_strerror(ops));
    }
    else {
        gfal_log(GFAL_VERBOSE_VERBOSE, "lfc register: done");
    }

register_end:
    g_free(lfc_host);
    g_free(lfc_path);
    g_free(src_host);
    return ret_status;
}
