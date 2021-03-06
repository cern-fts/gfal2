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
#ifndef CPP_TO_GERROR_HPP
#define CPP_TO_GERROR_HPP

#include <exception>

#include <glib.h>
#include <cerrno>

namespace Gfal{




#define CPP_GERROR_TRY do{ \
							try{

#define CPP_GERROR_CATCH(my_err_caught)  } \
        catch (Gfal::TransferException & e) {\
            gfalt_set_error(my_err_caught, e.domain(), e.code(), __func__, e.side.c_str(), e.note.c_str(), "%s", e.what()); \
        }catch(const Gfal::CoreException& e){ \
            gfal2_set_error(my_err_caught, e.domain(), e.code(), __func__, "%s", e.what()); \
        }catch(std::exception & e){ \
            gfal2_set_error(my_err_caught, gfal2_get_core_quark(), EPROTONOSUPPORT, __func__, "%s", e.what()); \
        }catch(...){ \
            gfal2_set_error(my_err_caught, gfal2_get_core_quark(), EIO, __func__, "Undefined Exception caught: Bug found !! "); \
        } \
        }while(0)
}


#endif /* CPP_TO_GERROR_HPP */

