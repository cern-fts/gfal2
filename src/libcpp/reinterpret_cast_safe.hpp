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


#ifndef _REINTERPRETER_CAST_SAFE_H
#define _REINTERPRETER_CAST_SAFE_H

#include <iostream>
#include <bstrexception.h>
#include <cerrno>

namespace Utilpp{
	
template <typename T>
T reinterpret_cast_void(void* ptr){
	if(ptr == NULL)
		throw new BStrException("NULL pointer unexpected", EFAULT);
	return reinterpret_cast<T>(ptr);
}

};

#endif /* _REINTERPRETER_CAST_SAFE_H */ 
