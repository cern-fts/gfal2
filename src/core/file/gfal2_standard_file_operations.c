/*
 * Copyright (c) CERN 2013-2017
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

#include <file/gfal_file_api.h>

#include <common/gfal_handle.h>
#include <common/gfal_plugin.h>
#include <common/gfal_error.h>
#include <common/gfal_cancel.h>

int gfal2_access(gfal2_context_t context, const char *url, int amode, GError **err)
{
    int res = -1;
    GError *tmp_err = NULL;

    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context or/and url are incorrect arguments");
    }
    else {
        res = gfal_plugins_accessG(context, (char *) url, amode, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_chmod(gfal2_context_t context, const char *url, mode_t mode, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;

    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context or/and url are incorrect arguments");
    }
    else {
        res = gfal_plugin_chmodG(context, url, mode, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_rename(gfal2_context_t context, const char *oldurl, const char *newurl, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;

    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (oldurl == NULL || newurl == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "oldurl/newurl/context are incorrect arguments");
    }
    else {
        res = gfal_plugin_renameG(context, oldurl, newurl, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_stat(gfal2_context_t context, const char *url, struct stat *buff, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL || buff == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context or/and url or/and buff are incorrect arguments");
    }
    else {
        res = gfal_plugin_statG(context, url, buff, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_lstat(gfal2_context_t context, const char *url, struct stat *buff, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL || buff == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context or/and url are incorrect arguments");
    }
    else {
        res = gfal_plugin_lstatG(context, url, buff, &tmp_err);
        // protocol does not support lstat, try to map to stat
        if (res != 0 && tmp_err && tmp_err->code == EPROTONOSUPPORT) {
            res = gfal2_stat(context, url, buff, err);
            g_error_free(tmp_err);
            tmp_err = NULL;
        }
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_mkdir(gfal2_context_t context, const char *url, mode_t mode, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context or/and url are incorrect arguments");
    }
    else {
        res = gfal_plugin_mkdirp(context, url, mode, FALSE, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_mkdir_rec(gfal2_context_t context, const char *url, mode_t mode,
    GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context or/and url are incorrect arguments");
    }
    else {
        gfal2_log(G_LOG_LEVEL_DEBUG, "execute directory creation for %s", url);
        res = gfal_plugin_mkdirp(context, url, mode, TRUE, &tmp_err);
        if (tmp_err) {
            if (tmp_err->code == EEXIST) {
                g_clear_error(&tmp_err);
                res = 0;
            }
            else if (tmp_err->code == ENOENT) {
                gfal2_log(G_LOG_LEVEL_DEBUG, "execute step-by-step recursive directory creation for %s", url);
                GList *stack_url = NULL;
                char current_url[GFAL_URL_MAX_LEN];
                g_strlcpy(current_url, url, GFAL_URL_MAX_LEN);

                while (tmp_err && tmp_err->code == ENOENT) {
                    stack_url = g_list_prepend(stack_url,
                        g_strdup(current_url));
                    g_clear_error(&tmp_err);
                    const size_t s_url = strlen(current_url);
                    char *p_url = current_url + s_url - 1;
                    while (p_url > current_url && *p_url == '/') { // remove trailing '/'
                        *p_url = '\0';
                        p_url--;
                    }
                    while (p_url > current_url && *p_url != '/') { // find the parent directory
                        p_url--;
                    }
                    if (p_url > current_url) {
                        *p_url = '\0';

                        res = gfal_plugin_mkdirp(context, current_url, mode,
                            FALSE, &tmp_err);
                        if (res == 0) {
                            gfal2_log(G_LOG_LEVEL_DEBUG, "created directory %s", current_url);
                        }
                    }
                }

                // Directory might have been created by a separate process
                if (tmp_err && tmp_err->code == EEXIST) {
                    g_clear_error(&tmp_err);
                }

                if (!tmp_err) {
                    gfal2_log(G_LOG_LEVEL_DEBUG, "recursive directory create from stack root %s", current_url);
                    res = 0;
                    GList *tmp_list = stack_url;
                    while (tmp_list != NULL && res == 0) {
                        res = gfal_plugin_mkdirp(context, (char *) tmp_list->data, mode, FALSE, &tmp_err);
                        if (res == 0) {
                            gfal2_log(G_LOG_LEVEL_DEBUG, "created directory %s", (char *) tmp_list->data);
                        }
                            // Due to a race condition, maybe someone else created the directory
                        else if (tmp_err->code == EEXIST) {
                            res = 0;
                            g_clear_error(&tmp_err);
                        }
                        tmp_list = g_list_next(tmp_list);
                    }
                }

                g_list_free_full(stack_url, g_free);
            }
        }

    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_rmdir(gfal2_context_t context, const char *url, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context or/and url are incorrect arguments");
    }
    else {
        res = gfal_plugin_rmdirG(context, url, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_symlink(gfal2_context_t context, const char *oldurl, const char *newurl, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (oldurl == NULL || newurl == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT,
            "oldurl and/or newurl and/or context are incorrect arguments");
    }
    else {
        res = gfal_plugin_symlinkG(context, oldurl, newurl, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(((res) ? -1 : 0), tmp_err, err);
}


ssize_t
gfal2_getxattr(gfal2_context_t context, const char *url, const char *name, void *value, size_t size, GError **err)
{
    GError *tmp_err = NULL;
    ssize_t res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL || name == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT,
            "url or/and context or/and name are incorrect arguments");
    }
    else {
        res = gfal_plugin_getxattrG(context, url, name, value, size, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


ssize_t gfal2_readlink(gfal2_context_t context, const char *url, char *buff, size_t buffsiz, GError **err)
{
    GError *tmp_err = NULL;
    ssize_t res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || buff == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "url and/or buff or/and context are incorrect arguments");
    }
    else {
        res = gfal_plugin_readlinkG(context, url, buff, buffsiz, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_unlink(gfal2_context_t context, const char *url, GError **err)
{
    GError *tmp_err = NULL;
    ssize_t res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "url and/or context are incorrect arguments");
    }
    else {
        res = gfal_plugin_unlinkG(context, url, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


ssize_t gfal2_listxattr(gfal2_context_t context, const char *url, char *list, size_t size, GError **err)
{
    GError *tmp_err = NULL;
    ssize_t res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context or/and url or/and list are incorrect arguments");
    }
    else {
        res = gfal_plugin_listxattrG(context, url, list, size, &tmp_err);

    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int
gfal2_setxattr(gfal2_context_t context, const char *url, const char *name, const void *value, size_t size, int flags,
    GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || name == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT,
            "url or/and name or/and context are an incorrect arguments");
    }
    else {
        res = gfal_plugin_setxattrG(context, url, name, value, size, flags,
            &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_bring_online(gfal2_context_t context, const char *url, time_t pintime,
    time_t timeout, char *token, size_t tsize, int async, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context or/and url are incorrect arguments");
    }
    else {
        res = gfal_plugin_bring_onlineG(context, url, pintime, timeout, token, tsize, async, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_bring_online_v2(gfal2_context_t context, const char *url, const char *metadata, time_t pintime,
    time_t timeout, char *token, size_t tsize, int async, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL || metadata == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context, url or/and metadata are incorrect arguments");
    }
    else {
        res = gfal_plugin_bring_online_v2G(context, url, metadata, pintime, timeout, token, tsize, async, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_bring_online_poll(gfal2_context_t context, const char *url, const char *token, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context or/and url are incorrect arguments");
    }
    else {
        res = gfal_plugin_bring_online_pollG(context, url, token, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_release_file(gfal2_context_t context, const char *url, const char *token, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context or/and url are incorrect arguments");
    }
    else {
        res = gfal_plugin_release_fileG(context, url, token, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


ssize_t gfal2_qos_check_classes(gfal2_context_t context, const char *url, const char *type,
                                char *buff, size_t s_buff, GError **err)
{
    GError *tmp_err = NULL;
    ssize_t res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || type == NULL || context == NULL || buff == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context, buff, type or/and url are incorrect arguments");
    }
    else {
        res = gfal_plugin_qos_check_classes(context, url, type, buff, s_buff, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}

ssize_t gfal2_check_file_qos(gfal2_context_t context, const char *url, char* buff, size_t s_buff, GError ** err)
{
    GError *tmp_err = NULL;
    ssize_t res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL || buff == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context, buff or/and url are incorrect arguments");
    }
    else {
        res = gfal_plugin_check_file_qos(context, url, buff, s_buff, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}

ssize_t gfal2_check_available_qos_transitions(gfal2_context_t context, const char *qos_class_url,
                                              char *buff, size_t s_buff, GError **err)
{
    GError *tmp_err = NULL;
    ssize_t res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (qos_class_url == NULL || context == NULL || buff == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context, buff or/and url are incorrect arguments");
    }
    else {
        res = gfal_plugin_check_qos_available_transitions(context, qos_class_url, buff, s_buff, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}

ssize_t gfal2_check_target_qos(gfal2_context_t context, const char *url, char *buff, size_t s_buff, GError ** err)
{
    GError *tmp_err = NULL;
    ssize_t res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL || buff == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context, buff or/and url are incorrect arguments");
    }
    else {
        res = gfal_plugin_check_target_qos(context, url, buff, s_buff, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}

int gfal2_change_object_qos(gfal2_context_t context, const char *url, const char *target_qos, GError ** err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL || target_qos == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context, url or/and target qos are incorrect arguments");
    }
    else {
        res = gfal_plugin_change_object_qos(context, url, target_qos, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}

ssize_t gfal2_token_retrieve(gfal2_context_t context, const char* url, const char* issuer,
                             gboolean write_access, unsigned validity, const char* const* activities,
                             char* buff, size_t s_buff, GError ** err)
{
    GError *tmp_err = NULL;
    ssize_t res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL || buff == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context, buff or/and resource url are incorrect arguments");
    }
    else {
        res = gfal_plugin_token_retrieveG(context, url, issuer, write_access, validity, activities,
                                          buff, s_buff, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}

int gfal2_bring_online_list(gfal2_context_t context, int nbfiles,
    const char *const *urls, time_t pintime, time_t timeout, char *token,
    size_t tsize, int async, GError **errors)
{
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, errors);

    if (urls == NULL || *urls == NULL || context == NULL) {
        int i;
        for (i = 0; i < nbfiles; ++i) {
            g_set_error(&errors[i], gfal2_get_core_quark(), EFAULT, "context or/and urls are incorrect arguments");
        }
        res = -1;
    }
    else {
        res = gfal_plugin_bring_online_listG(context, nbfiles, urls, pintime, timeout, token, tsize, async, errors);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    return res;
}


int gfal2_bring_online_list_v2(gfal2_context_t context, int nbfiles,
    const char *const *urls, const char *const *metadata, time_t pintime, time_t timeout, char *token,
    size_t tsize, int async, GError **errors)
{
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, errors);

    if (urls == NULL || *urls == NULL || context == NULL || metadata == NULL || *metadata == NULL) {
        int i;
        for (i = 0; i < nbfiles; ++i) {
            g_set_error(&errors[i], gfal2_get_core_quark(), EFAULT, "context, urls or/and metadata are incorrect arguments");
        }
        res = -1;
    }
    else {
        res = gfal_plugin_bring_online_list_v2G(context, nbfiles, urls, metadata, pintime, timeout, token, tsize, async, errors);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    return res;
}


int gfal2_bring_online_poll_list(gfal2_context_t context, int nbfiles,
    const char *const *urls, const char *token, GError **errors)
{
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, errors);

    if (urls == NULL || *urls == NULL || context == NULL) {
        int i;
        for (i = 0; i < nbfiles; ++i) {
            g_set_error(&errors[i], gfal2_get_core_quark(), EFAULT, "context or/and urls are incorrect arguments");
        }
        res = -1;
    }
    else {
        res = gfal_plugin_bring_online_poll_listG(context, nbfiles, urls, token, errors);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    return res;
}


int gfal2_release_file_list(gfal2_context_t context, int nbfiles, const char *const *urls, const char *token,
    GError **errors)
{
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, errors);

    if (urls == NULL || *urls == NULL || context == NULL) {
        int i;
        for (i = 0; i < nbfiles; ++i) {
            g_set_error(&errors[i], gfal2_get_core_quark(), EFAULT, "context or/and urls are incorrect arguments");
        }
        res = -1;
    }
    else {
        res = gfal_plugin_release_file_listG(context, nbfiles, urls, token, errors);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    return res;
}


int gfal2_unlink_list(gfal2_context_t context, int nbfiles, const char *const *urls, GError **errors)
{
    GError *tmp_err = NULL;
    int res = 0;

    if (urls == NULL || *urls == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT,
            "urls or/and name or/and context are an incorrect arguments");
        res = -1;
    }
    else {
        res = gfal2_start_scope_cancel(context, &tmp_err);
        if (res == 0) {
            res = gfal_plugin_unlink_listG(context, nbfiles, urls, errors);
            gfal2_end_scope_cancel(context);
        }
    }

    if (tmp_err) {
        int i;
        for (i = 0; i < nbfiles; ++i) {
            errors[i] = g_error_copy(tmp_err);
        }
        g_error_free(tmp_err);
    }
    return res;
}


int gfal2_abort_files(gfal2_context_t context, int nbfiles, const char *const *urls, const char *token, GError **err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (urls == NULL || *urls == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context or/and urls are incorrect arguments");
    }
    else {
        res = gfal_plugin_abort_filesG(context, nbfiles, urls, token, err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_archive_poll(gfal2_context_t context, const char* url, GError ** err)
{
    GError *tmp_err = NULL;
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, err);
    if (url == NULL || context == NULL) {
        g_set_error(&tmp_err, gfal2_get_core_quark(), EFAULT, "context or/and url are incorrect arguments");
    }
    else {
        res = gfal_plugin_archive_pollG(context, url, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    G_RETURN_ERR(res, tmp_err, err);
}


int gfal2_archive_poll_list(gfal2_context_t context, int nbfiles, const char* const *urls,
                            GError **errors)
{
    int res = -1;
    GFAL2_BEGIN_SCOPE_CANCEL(context, -1, errors);

    if (urls == NULL || *urls == NULL || context == NULL) {
        int i;
        for (i = 0; i < nbfiles; ++i) {
            g_set_error(&errors[i], gfal2_get_core_quark(), EFAULT, "context or/and urls are incorrect arguments");
        }
        res = -1;
    }
    else {
        res = gfal_plugin_archive_poll_listG(context, nbfiles, urls, errors);
    }
    GFAL2_END_SCOPE_CANCEL(context);
    return res;
}


int gfal2_checksum(gfal2_context_t handle, const char *url, const char *check_type,
    off_t start_offset, size_t data_length,
    char *checksum_buffer, size_t buffer_length, GError **err)
{
    if (!(handle != NULL && url != NULL && check_type != NULL
          && checksum_buffer != NULL && buffer_length != 0)) {
        g_set_error(err, gfal2_get_core_quark(), EFAULT, "Invalid parameters to %s", __func__);
        return -1;
    }

    GFAL2_BEGIN_SCOPE_CANCEL(handle, -1, err);
    int res = -1;
    GError *tmp_err = NULL;
    gfal_plugin_interface *p = gfal_find_plugin(handle, url, GFAL_PLUGIN_CHECKSUM, &tmp_err);

    if (p) {
        res = p->checksum_calcG(gfal_get_plugin_handle(p), url, check_type, checksum_buffer, buffer_length,
            start_offset,
            data_length, &tmp_err);
    }
    GFAL2_END_SCOPE_CANCEL(handle);
    G_RETURN_ERR(res, tmp_err, err);
}
