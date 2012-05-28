/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
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
 
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char* generate_random_uri(const char* uri_dir, const char* prefix, char* buff, size_t s_buff){
	snprintf(buff, s_buff, "%s/%s_%ld%ld",uri_dir, prefix, (long) time(NULL), (long) rand());
	return buff;
}

#include "gfal_lib_test.h"




