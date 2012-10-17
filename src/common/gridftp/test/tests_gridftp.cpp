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


#include "tests_gridftp.h"
#include <common/gridftp/gridftp_plugin_loader.h>
#include <common/gridftp/gridftp_plugin_main.h>
#include <common/gridftp/gridftpinterface.h>
#include <common/gridftp/gridftpmodule.h>
#include <cgreen/cgreen.h>

gfal_handle init_gfal_handle(){
	GError * tmp_err=NULL;
	gfal_handle h = gfal_initG(&tmp_err);
	assert_true_with_message(tmp_err== NULL && h ," initialize gfal failed ");
	return h;
}

plugin_handle init_gridftp_plugin_test(gfal_handle h){
	GError * tmp_err=NULL;

	
	plugin_handle p = plugin_load(h, &tmp_err);	
	assert_true_with_message(  p && tmp_err ==NULL ," must be a good init");
	return p;	
}

void load_gridftp(){
	GError * tmp_err=NULL;
    core_init();
	gfal_handle h = gfal_initG(&tmp_err);
	assert_true_with_message(tmp_err== NULL && h ," initialize gfal failed ");
	
	plugin_handle p = plugin_load(h, &tmp_err);
	assert_true_with_message(  p && tmp_err ==NULL ," must be a good init");
	plugin_unload(p);
	
	gfal_handle_freeG(h);
}


void handle_creation(){
	GError * tmp_err=NULL;
	gfal_handle h = gfal_initG(&tmp_err);
	assert_true_with_message(tmp_err== NULL && h ," initialize gfal failed ");
	
	GridFTPFactoryInterface* f = new GridFTPFactory(h);
	GridftpModule* copy = new GridftpModule(  f);	
	assert_true_with_message(copy != NULL, " must be a valid ");
	// create and delete properly
	GridFTP_session* sess = f->gfal_globus_ftp_take_handle("gsiftp://fsdfdsfsd/fsdfds");
	f->gfal_globus_ftp_release_handle(sess);
	
	// wild delete for exception clean recovery
	sess = f->gfal_globus_ftp_take_handle("gsiftp://fsdfdsfsd/fsdfds");
	delete sess;
	delete copy;
	gfal_handle_freeG(h);
}


void gridftp_parseURL(){
	// check null handle, must not segfault
	plugin_url_check2(NULL, "gsiftp://myurl.com/mypath/myfile", "gsiftp://myurl.com/mypath/myfile", GFAL_FILE_COPY);

	gfal_handle a= init_gfal_handle()	;
	plugin_handle p = init_gridftp_plugin_test(a);
	// check with URL null, must not segfault and return false
	bool res = plugin_url_check2(p, NULL, NULL, GFAL_FILE_COPY);	
	assert_true_with_message(res == FALSE, " must be a bad URL ");
	
	res = plugin_url_check2(p, "gsiftp://myurl.com/mypath/myfile", "gsiftp://myurl.com/mypath/myfile", GFAL_FILE_COPY);
	assert_true_with_message(res == TRUE, " must two good URL ");
	res = plugin_url_check2(p, "myurl.com/mypath/myfile", "gsiftp://myurl.com/mypath/myfile", GFAL_FILE_COPY);
	assert_true_with_message(res == FALSE, " first URL should be bad ");
	res = plugin_url_check2(p,  "gsiftp://myurl.com/mypath/myfile", "myurl.com/mypath/myfile", GFAL_FILE_COPY);
	assert_true_with_message(res == FALSE, " first URL shoudl be bad ");
	plugin_unload(p);
	
	gfal_handle_freeG(a);
}


