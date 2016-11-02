/*
 * Copyright (c) CERN 2016
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

#include "gfal_sftp_connection.h"
#include <uri/gfal2_uri.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pwd.h>

// libssh2_session_handshake introduced with 1.2.8
#if LIBSSH2_VERSION_NUM < 0x010208
#   define libssh2_session_handshake libssh2_session_startup
#endif


void gfal_plugin_sftp_translate_error(const char *func, gfal_sftp_handle_t *handle, GError **err)
{
    char *msg;
    int len;
    int ssh_errn = libssh2_session_last_error(handle->ssh_session, &msg, &len, 0);
    int errn = EIO;
    switch (ssh_errn) {
        case LIBSSH2_ERROR_TIMEOUT:
        case LIBSSH2_ERROR_SOCKET_TIMEOUT:
            errn = ETIMEDOUT;
            break;
        case LIBSSH2_ERROR_SOCKET_DISCONNECT:
            errn = ECONNRESET;
            break;
        case LIBSSH2_ERROR_PROTO:
            errn = EPROTO;
            break;
#ifdef LIBSSH2_ERROR_AUTHENTICATION_FAILED
        case LIBSSH2_ERROR_AUTHENTICATION_FAILED:
#endif
        case LIBSSH2_ERROR_PUBLICKEY_UNVERIFIED:
        case LIBSSH2_ERROR_CHANNEL_REQUEST_DENIED:
        case LIBSSH2_ERROR_REQUEST_DENIED:
            errn = EACCES;
            break;
        case LIBSSH2_ERROR_METHOD_NOT_SUPPORTED:
            errn = ENOSYS;
            break;
        case LIBSSH2_ERROR_INVAL:
            errn = EINVAL;
            break;
        case LIBSSH2_ERROR_EAGAIN:
            errn = EAGAIN;
            break;
        case LIBSSH2_ERROR_SFTP_PROTOCOL:
            errn = libssh2_sftp_last_error(handle->sftp_session);
            break;
    }
    gfal2_set_error(err, gfal2_get_plugin_sftp_quark(), errn, func, "%s", msg);
}


static int gfal_sftp_socket(gfal2_uri *parsed, GError **err)
{
    struct addrinfo hints, *addresses = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    int rc = getaddrinfo(parsed->host, NULL, &hints, &addresses);
    if (rc != 0) {
        gfal2_set_error(err, gfal2_get_plugin_sftp_quark(), EREMOTE, __func__, "Could not resolve host");
        return -1;
    }

    int port = htons(parsed->port ? parsed->port : 22);

    struct addrinfo *i;
    struct sockaddr_in *ipv4 = NULL;
    struct sockaddr_in6 *ipv6 = NULL;
    for (i = addresses; i != NULL; i = i->ai_next) {
        switch (i->ai_family) {
            case AF_INET:
                ipv4 = (struct sockaddr_in *) i->ai_addr;
                ipv4->sin_port = port;
                break;
            case AF_INET6:
                ipv6 = (struct sockaddr_in6 *) i->ai_addr;
                ipv6->sin6_port = port;
                break;
        }
    }
    // TODO: Configuration for IPv4 or 6
    char addrstr[100] = {0};
    struct sockaddr *addr = NULL;
    if (ipv4) {
        addr = (struct sockaddr *) ipv4;
        inet_ntop(AF_INET, &ipv4->sin_addr, addrstr, sizeof(addrstr));
    }
    else if (ipv6) {
        addr = (struct sockaddr *) ipv6;
        inet_ntop(AF_INET6, &ipv6->sin6_addr, addrstr, sizeof(addrstr));
    }
    else {
        freeaddrinfo(addresses);
        gfal2_set_error(err, gfal2_get_plugin_sftp_quark(), EHOSTUNREACH, __func__, "Could not find an IPv4 or IPv6");
        return -1;
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, "Connect to %s:%d", addrstr, port);

    int sock = socket(addr->sa_family, SOCK_STREAM, 0);
    if (sock < 0) {
        freeaddrinfo(addresses);
        gfal2_set_error(err, gfal2_get_plugin_sftp_quark(), errno, __func__, "Could not create the socket");
        return -1;
    }
    rc = connect(sock, addr, sizeof(*addr));
    freeaddrinfo(addresses);

    if (rc < 0) {
        close(sock);
        gfal2_set_error(err, gfal2_get_plugin_sftp_quark(), errno, __func__, "Could not connect");
        return -1;
    }

    return sock;
}


static void gfal_sftp_get_authn_params(gfal_sftp_context_t *data, gfal2_uri *parsed,
    char **user, char **passwd, char **privkey, char **passphrase)
{
    *user = *passwd = *privkey = NULL;

    char *config_user = gfal2_get_opt_string_with_default(data->gfal2_context, "SFTP PLUGIN", "USER", NULL);
    char *config_passwd = gfal2_get_opt_string_with_default(data->gfal2_context, "SFTP PLUGIN", "PASSWORD", NULL);

    // User and password
    if (parsed->userinfo) {
        char *separator = strchr(parsed->userinfo, ':');
        if (!separator) {
            *user = g_strdup(parsed->userinfo);
        }
        else {
            *user = g_strndup(parsed->userinfo, separator - parsed->userinfo);
            *passwd = g_strdup(separator + 1);
        }
    }
    else if (config_user) {
        *user = g_strdup(config_user);
        *passwd = g_strdup(config_passwd);
    }
    else {
        struct passwd *me_info = getpwuid(getuid());
        if (me_info) {
            *user = g_strdup(me_info->pw_name);
        }
    }
    // key
    *privkey = gfal2_get_opt_string_with_default(data->gfal2_context, "SFTP PLUGIN", "PRIVKEY", NULL);
    *passphrase = gfal2_get_opt_string_with_default(data->gfal2_context, "SFTP PLUGIN", "PASSPHRASE", NULL);
    if (!*privkey && getenv("HOME")) {
        *privkey = g_strconcat(getenv("HOME"), "/.ssh/id_rsa", NULL);
    }

    g_free(config_user);
    g_free(config_passwd);
}


static int gfal_sftp_authn(gfal_sftp_context_t *data, gfal2_uri *parsed, gfal_sftp_handle_t *handle, GError **err)
{
    char *user, *passwd, *privkey, *passphrase;
    gfal_sftp_get_authn_params(data, parsed, &user, &passwd, &privkey, &passphrase);

    gfal2_log(G_LOG_LEVEL_DEBUG, "User %s, key %s", user, privkey);

    const char *userauthlist = libssh2_userauth_list(handle->ssh_session, user, strlen(user));
    gfal2_log(G_LOG_LEVEL_DEBUG, "Supported authn methods: %s", userauthlist);

    const char *auth_method = userauthlist;
    int authenticated = 0;
    while (auth_method) {
        if (strncmp(auth_method, "publickey", 9) == 0) {
            gfal2_log(G_LOG_LEVEL_DEBUG, "Trying publickey");
            if (libssh2_userauth_publickey_fromfile(handle->ssh_session, user, passwd, privkey, passphrase) == 0) {
                authenticated = 1;
            }
        }
        else if (strncmp(auth_method, "password", 8) == 0) {
            gfal2_log(G_LOG_LEVEL_DEBUG, "Trying password");
            if (libssh2_userauth_password(handle->ssh_session, user, passwd) == 0) {
                authenticated = 1;
            }
        }
        if (authenticated) {
            break;
        }
        auth_method = strchr(auth_method, ',');
        if (auth_method) {
            ++auth_method;
        }
    }
    g_free(user);
    g_free(passwd);
    g_free(privkey);

    if (!authenticated) {
        gfal2_set_error(err, gfal2_get_plugin_sftp_quark(), EACCES, __func__,
            "All supported authentication methods failed");
        return -1;
    }
    return 0;
}


static gfal_sftp_handle_t *gfal_sftp_new_handle(gfal_sftp_context_t *data, gfal2_uri *parsed, GError **err)
{
    int rc;

    gfal_sftp_handle_t *handle = g_malloc(sizeof(gfal_sftp_handle_t));
    handle->host = g_strdup(parsed->host);
    handle->port = parsed->port;
    handle->sock = gfal_sftp_socket(parsed, err);
    if (handle->sock < 0) {
        goto get_handle_failure;
    }
    gfal2_log(G_LOG_LEVEL_DEBUG, "Connected to remote");

    handle->ssh_session = libssh2_session_init();
    if (!handle->ssh_session) {
        gfal2_set_error(err, gfal2_get_plugin_sftp_quark(), ECONNABORTED, __func__,
            "Failed to get a session");
        goto get_handle_failure;
    }

    rc = libssh2_session_handshake(handle->ssh_session, handle->sock);
    if (rc != 0) {
        goto get_handle_failure_ssh;
    }

    rc = gfal_sftp_authn(data, parsed, handle, err);
    if (rc != 0) {
        goto get_handle_failure;
    }
    gfal2_log(G_LOG_LEVEL_DEBUG, "Authenticated with remote");

    handle->sftp_session = libssh2_sftp_init(handle->ssh_session);
    if (!handle->sftp_session) {
        goto get_handle_failure_ssh;
    }
    gfal2_log(G_LOG_LEVEL_DEBUG, "SFTP initialized");

    libssh2_session_set_blocking(handle->ssh_session, 1);

    return handle;

    get_handle_failure_ssh:
    gfal_plugin_sftp_translate_error(__func__, handle, err);
    get_handle_failure:
    g_free(handle);
    return NULL;
}


static void gfal_sftp_destroy_handle(gfal_sftp_handle_t *handle, gpointer user_data)
{
    close(handle->sock);
    libssh2_sftp_shutdown(handle->sftp_session);
    libssh2_session_disconnect(handle->ssh_session, "");
    libssh2_session_free(handle->ssh_session);
    g_free(handle);
}


gfal_sftp_handle_t *gfal_sftp_connect(gfal_sftp_context_t *context, const char *url, GError **err)
{
    gfal2_uri *parsed = gfal2_parse_uri(url, err);
    if (!parsed) {
        return NULL;
    }

    gfal_sftp_handle_t *handle = gfal_sftp_cache_pop(context->cache, parsed->host, parsed->port);
    if (!handle) {
        gfal2_log(G_LOG_LEVEL_DEBUG, "Creating new SFTP handle");
        handle = gfal_sftp_new_handle(context, parsed, err);
    } else {
#if LIBSSH2_VERSION_NUM >= 0x010205
        int seconds = 10;
        gfal2_log(G_LOG_LEVEL_DEBUG, "Reusing SFTP handle from cache for %s:%d", handle->host, handle->port);
        int rc = libssh2_keepalive_send(handle->ssh_session, &seconds);
        if (rc < 0) {
            gfal2_log(G_LOG_LEVEL_DEBUG, "Recycled SFTP handle failed to send keepalive. Discard and reconnect");
            gfal_sftp_destroy_handle(handle, NULL);
            handle = gfal_sftp_new_handle(context, parsed, err);
        }
#endif
    }
    if (handle) {
        handle->path = g_strdup(parsed->path);
    }

    gfal2_free_uri(parsed);
    return handle;
}


void gfal_sftp_release(gfal_sftp_context_t *context, gfal_sftp_handle_t *handle)
{
    gfal2_log(G_LOG_LEVEL_DEBUG, "Pushing SFTP handle into cache for %s:%d", handle->host, handle->port);
    gfal_sftp_cache_push(context->cache, handle);
}


static void gfal_sftpdestroy_key(gpointer p)
{
    g_string_free((GString*)p, TRUE);
}


GHashTable *gfal_sftp_cache_new()
{
    return g_hash_table_new_full((GHashFunc) g_string_hash, (GEqualFunc) g_string_equal,
        gfal_sftpdestroy_key, NULL);
}


gfal_sftp_handle_t *gfal_sftp_cache_pop(GHashTable *cache, const char *host, int port)
{
    GString *key = g_string_new(NULL);
    g_string_printf(key, "%s:%d", host, port);
    GSList *list = (GSList*)g_hash_table_lookup(cache, key);
    if (!list) {
        g_string_free(key, TRUE);
        return NULL;
    }
    gfal_sftp_handle_t *handle = (gfal_sftp_handle_t*)list->data;
    list = g_slist_delete_link(list, list);
    // g_hash_table_insert acquires ownership of key
    g_hash_table_insert(cache, key, list);
    return handle;
}


void gfal_sftp_cache_push(GHashTable *cache, gfal_sftp_handle_t *handle)
{
    GString *key = g_string_new(NULL);
    g_string_printf(key, "%s:%d", handle->host, handle->port);
    GSList *list = (GSList*)g_hash_table_lookup(cache, key);
    list = g_slist_prepend(list, handle);
    // g_hash_table_insert acquires ownership of key
    g_hash_table_insert(cache, key, list);
}


void gfal_sftp_destroy_cache_entry(gpointer key, gpointer value, gpointer user_data)
{
    g_slist_foreach((GSList*)value, (GFunc)gfal_sftp_destroy_handle, NULL);
    g_slist_free((GSList*)value);
}


void gfal_sftp_cache_destroy(GHashTable *cache)
{
    g_hash_table_foreach(cache, gfal_sftp_destroy_cache_entry, NULL);
    g_hash_table_destroy(cache);
}
