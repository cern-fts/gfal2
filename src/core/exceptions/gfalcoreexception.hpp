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
#ifndef GFALCOREEXCEPTION_H
#define GFALCOREEXCEPTION_H

#include <exception>
#include <glib.h>
#include <string>


namespace Gfal{

class CoreException: public std::exception
{
	public:
        CoreException(GQuark scope, int code, const std::string & msg);
        CoreException(const GError* error);
		virtual ~CoreException() throw();

		virtual GQuark domain() const throw ();
		virtual const char* what() const throw();
		virtual const std::string& what_str() const throw();
		virtual int code() const throw();

	private:
		GQuark _scope;
		std::string _msg;
		int _code;
};

class TransferException: public CoreException {
public:
    std::string side;
    std::string note;

    TransferException(GQuark scope, int code, const std::string & msg,
            const std::string & side, const std::string & note = std::string()):
                CoreException(scope, code, msg), side(side), note(note)
    {
    }

    virtual ~TransferException() throw()
    {
    }
};

}

#endif /* GFALCOREEXCEPTION_H */
