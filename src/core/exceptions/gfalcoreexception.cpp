/*
 * Copyright @ Members of the EMI Collaboration, 2010.
 * See www.eu-emi.eu for details on the copyright holders.
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

#include "gfalcoreexception.hpp"

Gfal::CoreException::CoreException(GQuark scope, int code, const std::string& msg):
        _scope(scope), _msg(msg), _code(code)
{
}

Gfal::CoreException::CoreException(const GError* error):
    _scope(error->domain), _msg(error->message), _code(error->code)
{
}

Gfal::CoreException::~CoreException() throw ()
{

}

GQuark Gfal::CoreException::domain() const throw ()
{
    return _scope;
}

const char* Gfal::CoreException::what() const throw ()
{
    return _msg.c_str();
}

const std::string& Gfal::CoreException::what_str() const throw ()
{
    return _msg;
}

int Gfal::CoreException::code() const throw ()
{
    return _code;
}
