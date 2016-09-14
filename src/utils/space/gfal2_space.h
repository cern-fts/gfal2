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

#ifndef GFAL_SPACE_H_
#define GFAL_SPACE_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Utilities for space reporting via extended attributes
 */

enum space_latency {
    LATENCY_UNKNOWN = 0,
    LATENCY_ONLINE,
    LATENCY_NEARLINE
};

enum space_retention_policy {
    POLICY_UNKNOWN = 0,
    POLICY_REPLICA,
    POLICY_OUTPUT,
    POLICY_CUSTODIAL
};

struct space_report {
    uint64_t used;
    uint64_t free;
    uint64_t total;

    // Optional values
    uint64_t *largest_chunk;
    int *lifetime_assigned;
    int *lifetime_left;

    enum space_latency latency;
    enum space_retention_policy retention;

    char *owner;
    char *spacetoken;
};

/*
 * Generate a json containing the information in space_report.
 * Optional fields set to NULL are skipped.
 */
size_t gfal2_space_generate_json(struct space_report *report, char *buffer, size_t s_buff);

#ifdef __cplusplus
}
#endif

#endif // GFAL_SPACE_H_
