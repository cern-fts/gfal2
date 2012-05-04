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


#include "tests_params.h"
#include <cstdlib>

#include <transfer/gfal_transfer_types_internal.h>
#include <transfer/gfal_transfer_plugins.h>
#include <common/gfal_types.h>
#include <glibmm.h>
#include <cgreen/cgreen.h>	
#include <glib.h>


using namespace Gfal::Transfer;


int get_locked_errcode(){
	return EBUSY;
}

void create_params(){
	Params p;

		
}

void test_timeout(){
	Params p;
	long res = p.get_timeout();
	assert_true_with_message( res == GFALT_DEFAULT_TRANSFERT_TIMEOUT, "bad timeout value");
	long  r = rand();
	p.set_timeout(r);
	assert_true_with_message( p.get_timeout()  == r, "bad timeout get ");	
	p.lock(); // lock to block read access
	try{
		p.set_timeout(r);	
		assert_true_with_message(FALSE, "should send exception, locked ");
	}catch(Glib::Error & e){
		assert_true_with_message(e.code() == get_locked_errcode() , " must be a inval value");
	}catch (...){
		assert_true_with_message(FALSE, " unknown exception");
	}	
}

void test_timeout_c(){
	GError * tmp_err=NULL;
	
	gfalt_params_handle p = gfalt_params_handle_new(&tmp_err);
	assert_true_with_message( p != NULL && tmp_err==NULL, "bad initialization ");
	long res = gfalt_get_timeout(p, &tmp_err);
	assert_true_with_message( res == GFALT_DEFAULT_TRANSFERT_TIMEOUT && tmp_err==NULL, "bad timeout value %ld %ld ", res, tmp_err);
	long  r = rand();
	gfalt_set_timeout(p, r, &tmp_err);
	assert_true_with_message( gfalt_get_timeout(p, &tmp_err)  == r, "bad timeout get ");	
	((Params*)p)->lock(); // lock to block read access
	res = gfalt_set_timeout(p, r, &tmp_err);
	assert_true_with_message(tmp_err != NULL && tmp_err->code == get_locked_errcode() && res == -1 , " must be a inval value %ld %ld ", tmp_err->code, res);
}


void test_nbstreams(){
	Params p;
	unsigned long res = p.get_nbstream();
	assert_true_with_message( res == GFALT_DEFAULT_NB_STREAM, "bad nbstreams value");
	unsigned long  r = rand();
	p.set_nbstream(r);
	assert_true_with_message( p.get_nbstream()  == r, "bad nbstreams get ");	
	p.lock(); // lock to block read access
	try{
		p.set_nbstream(r);	
		assert_true_with_message(FALSE, "should send exception, locked ");
	}catch(Glib::Error & e){
		assert_true_with_message(e.code() == get_locked_errcode() , " must be a inval value");
	}catch (...){
		assert_true_with_message(FALSE, " unknown exception");
	}	
}

void test_nbstreams_c(){
	GError * tmp_err=NULL;
	gfalt_params_handle p = gfalt_params_handle_new(&tmp_err);
	assert_true_with_message( p != NULL && tmp_err==NULL, "bad initialization ");
	long res = gfalt_get_nbstreams(p, &tmp_err);
	assert_true_with_message( res == GFALT_DEFAULT_NB_STREAM && tmp_err==NULL, "bad nbstreams value %ld %ld ", res, tmp_err);
	long  r = rand();
	gfalt_set_nbstreams(p, r, &tmp_err);
	assert_true_with_message( gfalt_get_nbstreams(p, &tmp_err)  == r, "bad timeout get ");	
	((Params*)p)->lock(); // lock to block read access
	res = gfalt_set_nbstreams(p, r, &tmp_err);
	assert_true_with_message(tmp_err != NULL && tmp_err->code == get_locked_errcode() && res == -1 , " must be a inval value %ld %ld ", tmp_err->code, res);
}



void test_lock(){
	Params p;
	long res = p.get_timeout();
	assert_true_with_message( res == GFALT_DEFAULT_TRANSFERT_TIMEOUT, "bad timeout value");
	p.lock(); // verify lock
	try{
		p.set_timeout(res);	
		assert_true_with_message(FALSE, "should send exception, locked ");
	}catch(Glib::Error & e){
		assert_true_with_message(e.code() == get_locked_errcode() , " must be a inval value");
	}catch (...){
		assert_true_with_message(FALSE, " unknown exception");
	}		
	
	p.unlock();
	res = p.get_timeout();
	assert_true_with_message( res == GFALT_DEFAULT_TRANSFERT_TIMEOUT, "bad timeout value");	
}

