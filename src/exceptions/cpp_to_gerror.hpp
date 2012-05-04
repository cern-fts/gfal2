#pragma once
#ifndef CPP_TO_GERROR_HPP
#define CPP_TO_GERROR_HPP
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




#include <exception>

#include <glib.h>
#include <cerrno>

namespace Gfal{




#define CPP_GERROR_TRY do{ \
							try{

#define CPP_GERROR_CATCH(err)  } \
							catch(Glib::Error & e){ \
								g_set_error(err, e.domain(), e.code(), e.what().c_str()); \
							}catch(std::exception & e){ \
								g_set_error(err, 0, EPROTONOSUPPORT , e.what()); \
							}catch(...){ \
								g_set_error(err, 0, EIO, "Undefined Exception catched: Bug found !! "); \
							} \
							}while(0) 	
}


#endif /* CPP_TO_GERROR_HPP */ 

