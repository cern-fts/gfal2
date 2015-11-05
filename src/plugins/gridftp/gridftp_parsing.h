/*
* Copyright @ CERN, 2015
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

#ifndef GFAL2_GRIDFTP_PARSING_H
#define GFAL2_GRIDFTP_PARSING_H

#include <globus_types.h>

/// Parse a MLST line
globus_result_t parse_mlst_line(char *line, struct stat *stat_info, char *filename_buf, size_t filename_size);

/// Parse a STAT or LIST line
globus_result_t parse_stat_line(char* buffer, struct stat* fstat, char *filename_buf, size_t filename_size);

#endif //GFAL2_GRIDFTP_PARSING_H
