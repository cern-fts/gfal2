/*
 * Copyright (c) CERN 2016
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

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include "gfal2_space.h"

#include <json.h>
#include <string.h>


static const char *retention2str(enum space_retention_policy policy)
{
    switch (policy) {
        case POLICY_REPLICA:
            return "REPLICA";
        case POLICY_OUTPUT:
            return "OUTPUT";
        case POLICY_CUSTODIAL:
            return "CUSTODIAL";
        default:
            return "UNKNOWN";
    }
}


static const char *accesslatency2str(enum space_latency latency)
{
    switch (latency) {
        case LATENCY_ONLINE:
            return "ONLINE";
        case LATENCY_NEARLINE:
            return "NEARLINE";
        default:
            return "UNKNOWN";
    }
}

size_t gfal2_space_generate_json(struct space_report *report, char *buffer, size_t s_buff)
{
    struct json_object *root = json_object_new_object();

    if (report->spacetoken) {
        json_object_object_add(root, "spacetoken", json_object_new_string(report->spacetoken));
    }
    if (report->owner) {
        json_object_object_add(root, "owner", json_object_new_string(report->owner));
    }

    json_object_object_add(root, "totalsize", json_object_new_int64(report->total));
    json_object_object_add(root, "unusedsize", json_object_new_int64(report->free));
    json_object_object_add(root, "usedsize", json_object_new_int64(report->used));

    if (report->largest_chunk) {
        json_object_object_add(root, "guaranteedsize", json_object_new_int64(*report->largest_chunk));
    }

    if (report->lifetime_assigned) {
        json_object_object_add(root, "totalsize", json_object_new_int(*report->lifetime_assigned));
    }
    if (report->lifetime_left) {
        json_object_object_add(root, "lifetimeleft", json_object_new_int(*report->lifetime_left));
    }
    if (report->retention != POLICY_UNKNOWN) {
        json_object_object_add(root, "retention",
            json_object_new_string(retention2str(report->retention)));
    }
    if (report->latency != LATENCY_UNKNOWN) {
        json_object_object_add(root, "accesslatency",
            json_object_new_string(accesslatency2str(report->latency)));
    }

    const char *serialized = json_object_to_json_string(root);
    strncpy(buffer, serialized, s_buff);

    json_object_put(root);

    return strlen(buffer);
}
