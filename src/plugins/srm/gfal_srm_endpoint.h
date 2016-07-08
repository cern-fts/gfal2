/*
 * Copyright (c) CERN 2013-2015
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

#pragma once

#include "gfal_srm.h"
#include <stdlib.h>


/**
 * Extract endpoint and srm_type from a surl
 *  determine the best endpoint associated with the surl and the param of the actual handle (no bdii check or not)
 *  see the diagram in doc/diagrams/surls_get_endpoint_activity_diagram.svg for more informations
 *  @return return 0 with endpoint and types set if success else -1 and set Error
 */
int gfal_srm_determine_endpoint(gfal_srmv2_opt *opts, const char *surl, char *buff_endpoint, size_t s_buff,
    enum gfal_srm_proto *srm_type, GError **err);

/** Returns 1 if the surl is a castor endpoint */
int is_castor_endpoint(plugin_handle handle, const char *surl);
