/*
 * Copyright (c) CERN 2022
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

#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "gfal2_network.h"
#include "gfal_plugins_api.h"

char* gfal2_resolve_dns_to_hostname(const char* dnshost)
{
    struct addrinfo hints;
    struct addrinfo* addresses = NULL;
    struct addrinfo* addrP = NULL;
    GString* log_str = g_string_sized_new(512);
    char addrstr[INET6_ADDRSTRLEN * 2];
    char hostname[256];
    void* ptr = NULL;
    int count = 0;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    int rc = getaddrinfo(dnshost, NULL, &hints, &addresses);

    if (rc || !addresses) {
        if (addresses) {
            freeaddrinfo(addresses);
        }

        gfal2_log(G_LOG_LEVEL_ERROR, "Could not resolve DNS alias: %s", dnshost);
        return NULL;
    }

    // Count and log all resolved addresses
    for (addrP = addresses; addrP != NULL; addrP = addrP->ai_next) {
        inet_ntop(addrP->ai_family, addrP->ai_addr->sa_data, addrstr, sizeof(addrstr));

        switch (addrP->ai_family) {
            case AF_INET:
                ptr = &((struct sockaddr_in *) addrP->ai_addr)->sin_addr;
                if (ptr) {
                    inet_ntop(addrP->ai_family, ptr, addrstr, sizeof(addrstr));
                }
                break;
            case AF_INET6:
                ptr = &((struct sockaddr_in6 *) addrP->ai_addr)->sin6_addr;
                if (ptr) {
                    inet_ntop(addrP->ai_family, ptr, addrstr, sizeof(addrstr));
                }
                break;
        }

        getnameinfo(addrP->ai_addr, addrP->ai_addrlen, hostname, sizeof(hostname), NULL, 0, NI_NAMEREQD);
        g_string_append_printf(log_str, "%s[%s] ", hostname, addrstr);
        count++;
    }

    gfal2_log(G_LOG_LEVEL_DEBUG, "Resolved DNS alias %s into: %s", dnshost, log_str->str);
    g_string_free(log_str, TRUE);

    // Select at random an address between [0, count)
    srand(time(NULL));
    int selected = rand() % count;

    for (addrP = addresses; addrP != NULL; addrP = addrP->ai_next) {
        if (selected-- == 0) {
            getnameinfo(addrP->ai_addr, addrP->ai_addrlen, hostname, sizeof(hostname), NULL, 0, NI_NAMEREQD);
            break;
        }
    }

    freeaddrinfo(addresses);
    return strdup(hostname);
}
