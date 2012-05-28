
/*
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

#include <errno.h>
#include <utils/uri_util.h>
#include <string.h>
 
static GQuark scope_uri(){
	return g_quark_from_static_string("Gfal::Uri_util");
}

int gfal_hostname_from_uri(const char * uri, char* buff_hostname, size_t s_buff, GError ** err){
	 g_return_val_if_fail (uri != NULL && buff_hostname != NULL && s_buff != NULL, -1);
	 
	char* p1, *p2; 
	p1= strstr(uri, "://");
	if( p1 != NULL){
		p2 = ++p1;		
		while(*p1 != '\0' && *p1 == '/') // end of initial separators
			p1++;
		if( *p1 != '\0' && *(p1+1) != '\0'){
			p2 =p1;
			while(*p2 != '\0' && *p2!= '/')
				p2++;
			return g_strlcpy(buff_hostname, p1, MIN(p2-p1+1,s_buff));
		}
	}
	g_set_error(err, scope_uri(), EINVAL, "Incorrect URI, no hostname");	
	return -1;
}


