#pragma once
#ifndef GFALCOREEXCEPTION_H
#define GFALCOREEXCEPTION_H

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


#include <glibmm.h>
#include <exceptions/cpp_to_gerror.hpp>

namespace Gfal{

class CoreException: public Glib::Error
{
	public:
        CoreException(GQuark scope, const std::string & msg, int mode);
		CoreException(const Glib::Quark & scope, const std::string & msg, int mode);
		virtual ~CoreException() throw();

		virtual const char* message_only() const throw();

	
	private:
		/* add your private declarations */
};

}

#endif /* GFALCOREEXCEPTION_H */ 
