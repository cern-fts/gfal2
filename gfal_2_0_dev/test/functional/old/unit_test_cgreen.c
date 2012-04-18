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
 
 





/**
 * Unit tests for gfal based on the cgreen library
 * @author : Devresse Adrien
 * @version : 0.0.1
 */

#include <cgreen/cgreen.h>
#include <stdio.h>
#include <stdlib.h>
#include "unit_test_constants.h"
#include "common/gfal_common_errverbose.h"
#include "common/gfal__test_verbose.h"
#include "common/gfal__test_plugin.h"
//#include "common/voms/gfal__test_voms.h"
#include "common/gfal__test_common_srm.h"
#include "common/gfal__test_common_srm_access.h"
#include "common/mds/gfal__test_common_mds.h"
#include "common/lfc/gfal__test_common_lfc.h"
#include "common/gfal__test_common_srm_no_glib.h"
#include "common/gfal__test_common_dir_file_descriptor.h"
#include "posix/test__gfal_posix_access.h"
#include "posix/test__gfal_posix_chmod.h"
#include "posix/test__gfal_posix_mkdir.h"
#include "posix/test__gfal_posix_stat.h"
#include "posix/test__gfal_posix_open.h"
#include "posix/test__gfal_posix_read.h"
#include "posix/test__gfal_posix_write.h"
#include "posix/test__gfal_posix_opendir.h"
#include "posix/test__gfal_posix_parameter.h"
#include "posix/test__gfal_posix_xattr.h"
#include "common/rfio/test__gfal_rfio_plugin.h"

#include "externals/test_skiplist.h"

TestSuite * verbose_suite (void)
{
   TestSuite *s1 = create_test_suite();
  // verbose test case /
   add_test(s1, gfal2_test_verbose_set_get);
   return s1;
 }
 
 
TestSuite * plugin_suite (void)
{
	TestSuite *s2 = create_test_suite();
	// verbose test case /
	add_test(s2, gfal2_test_get_cat_type);
	add_test(s2, gfal2_test_plugin_access_file);
	add_test(s2, gfal2_test_plugin_url_checker);
	add_test(s2, gfal2_test__plugin_stat);
	add_test(s2, gfal2_test__plugin_lstat);
	return s2;
 }
 

 
 
TestSuite* srm_Suite(){
	TestSuite *tc_srm = create_test_suite();
	add_test(tc_srm, gfal2_test_create_srm_handle);
	add_test(tc_srm, gfal2_test__gfal_convert_full_surl);
	add_test(tc_srm, gfal2_test_gfal_get_hostname_from_surl);
	add_test(tc_srm, gfal2_test_gfal_full_endpoint_checkG);
	add_test(tc_srm, gfal2_test_gfal_get_endpoint_and_setype_from_bdiiG);
	add_test(tc_srm, gfal2_test_gfal_select_best_protocol_and_endpointG);
	add_test(tc_srm, gfal2_test_gfal_srm_determine_endpoint_full_endpointG);
	add_test(tc_srm, gfal2_test_gfal_auto_get_srm_endpoint_full_endpoint_with_no_bdiiG);
	add_test(tc_srm, gfal2_test_gfal_srm_determine_endpoint_not_fullG);
	add_test(tc_srm, gfal2_test_gfal_srm_getTURLS_one_success);
	add_test(tc_srm, gfal2_test_gfal_srm_getTURLS_bad_urls);
	add_test(tc_srm, gfal2_test_gfal_srm_getTURLS_pipeline_success);
	add_test(tc_srm, gfal2_test_srm_get_checksum);
	return tc_srm;

}


TestSuite* no_glib_suite(){
	TestSuite* tc_srm_no_glib = create_test_suite();
	add_test(tc_srm_no_glib, gfal2_test_srm_api_no_glib_full);
	return tc_srm_no_glib;
}

TestSuite* mds_suite(){
	TestSuite *tc_mds= create_test_suite();
	add_test(tc_mds, gfal2_test_check_bdii_endpoints_srm);
	add_test(tc_mds, gfal2_gfal2_test_check_bdii_endpoints_srm_ng);
	return tc_mds;
}

TestSuite* lfc_suite(){
	TestSuite *tc_lfc= create_test_suite();
	add_test(tc_lfc, gfal2_test_gfal_common_lfc_resolve_sym);
	add_test(tc_lfc, gfal2_test_gfal_common_lfc_define_env);
	add_test(tc_lfc, gfal2_test_gfal_common_lfc_init);
	add_test(tc_lfc,  gfal2_test__gfal_common_lfc_statg);	
	add_test(tc_lfc, gfal2_test__gfal_common_lfc_rename);
	add_test(tc_lfc, gfal2_test_gfal_common_lfc_access);
	add_test(tc_lfc, gfal2_test_gfal_common_lfc_no_exist);
	add_test(tc_lfc, gfal2_test_gfal_common_lfc_check_filename);
	add_test(tc_lfc, gfal2_test_gfal_common_lfc_getSURL);
	add_test(tc_lfc, gfal2_gfal2_test_gfal_common_lfc_access_guid_file_exist);
	add_test(tc_lfc, gfal2_test_common_lfc_getcomment);
	add_test(tc_lfc, gfal2_test_common_lfc_setcomment);
	add_test(tc_lfc, gfal2_test_common_lfc_checksum);
	return tc_lfc;
}

TestSuite* filedesc_suite(){
	TestSuite *tc_filedesc = create_test_suite();
	add_test(tc_filedesc, gfal2_test__dir_file_descriptor_low);
	add_test(tc_filedesc, gfal2_test__dir_file_descriptor_high);
	return tc_filedesc;
}

TestSuite* posix_chmod_suite(void){
	TestSuite* tc_chmod = create_test_suite();	
	add_test(tc_chmod, gfal2_test__gfal_posix_chmod_read_lfn);
	add_test(tc_chmod, gfal2_test__gfal_posix_chmod_read_guid);
	add_test(tc_chmod, gfal2_test__gfal_posix_chmod_read_local);
	add_test(tc_chmod, gfal2_test__gfal_posix_chmod_write_lfn);
	add_test(tc_chmod, gfal2_test__gfal_posix_chmod_srm);
	return tc_chmod;
}

TestSuite* posix_mkdir_suite(void){
	TestSuite* tc_mkdir = create_test_suite();
	add_test(tc_mkdir, gfal2_test__mkdir_posix_lfc_simple);
	add_test(tc_mkdir, gfal2_test__mkdir_posix_local_simple);
	add_test(tc_mkdir, gfal2_test__mkdir_posix_srm_simple);
	return tc_mkdir;
}

TestSuite* posix_stat_suite(void){
	TestSuite* tc_stat= create_test_suite();
	add_test(tc_stat, gfal2_test__gfal_posix_stat_lfc);
	add_test(tc_stat, gfal2_test__gfal_posix_stat_guid);
	add_test(tc_stat, gfal2_test__gfal_posix_stat_local);
	add_test(tc_stat, gfal2_test__gfal_posix_stat_srm);
	add_test(tc_stat, gfal2_test__gfal_posix_lstat_lfc);
	add_test(tc_stat, gfal2_test__gfal_posix_lstat_guid);
	add_test(tc_stat, gfal2_test__gfal_posix_lstat_local);
	add_test(tc_stat, gfal2_test__gfal_posix_lstat_srm);
	return tc_stat;
}

TestSuite* posix_open_suite(void){
	TestSuite* tc_open = create_test_suite();
	add_test(tc_open, gfal2_test_open_posix_all_simple);
	add_test(tc_open, gfal2_test_open_posix_local_simple);
	add_test(tc_open, gfal2_test_open_posix_lfc_simple);
	add_test(tc_open, gfal2_test_open_posix_srm_simple);
	add_test(tc_open, gfal2_test_open_posix_guid_simple);
	return tc_open;
}

TestSuite* posix_read_suite(void){
	TestSuite* tc_read = create_test_suite();
	add_test(tc_read, gfal2_test_read_posix_local_simple);
	add_test(tc_read, gfal2_test_read_posix_srm_simple);
	add_test(tc_read, gfal2_test_read_posix_lfc_simple);
	add_test(tc_read, gfal2_test_read_posix_guid_simple);
	add_test(tc_read, gfal2_test_pread_posix_local_simple);
	add_test(tc_read, gfal2_test_pread_posix_srm_simple);
	add_test(tc_read, gfal2_test_pread_posix_lfc_simple);
	add_test(tc_read, gfal2_test_pread_posix_guid_simple);
	return tc_read;
}

TestSuite* posix_write_suite(void){
	TestSuite* tc_write = create_test_suite();
	add_test(tc_write, gfal2_test_write_posix_local_simple);
	add_test(tc_write, gfal2_test_pwrite_posix_local_simple);
	add_test(tc_write, gfal2_test_write_posix_srm_simple);
	add_test(tc_write, gfal2_test_pwrite_posix_srm_simple);
	
	add_test(tc_write, gfal2_test_write_posix_lfc_simple);
	return tc_write;
}


TestSuite* posix_rfio_plugin_suite(){
	TestSuite* tc_rfio = create_test_suite();
	add_test(tc_rfio, gfal2_test_load_plugin);
	return tc_rfio;	
}

TestSuite* posix_opendir_suite(){
      TestSuite* tc_opendir = create_test_suite();
      add_test(tc_opendir, gfal2_test__opendir_posix_local_simple);
      add_test(tc_opendir, gfal2_test__opendir_posix_lfc_simple);
      add_test(tc_opendir, gfal2_test__readdir_posix_local_simple);
      add_test(tc_opendir, gfal2_test__readdir_posix_lfc_simple);
      add_test(tc_opendir, gfal2_test__opendir_posix_srm_simple_mock);
      add_test(tc_opendir, gfal2_test__readdir_posix_srm_simple_mock);
      return tc_opendir;	
}


TestSuite* posix_parameter_suite(){
  TestSuite* tc_parameter = create_test_suite();  
  add_test(tc_parameter, test_posix_set_get_parameter);
  add_test(tc_parameter, test_posix_set_get_false_parameter);
  add_test(tc_parameter, test_posix_set_get_infosys_parameter);
  add_test(tc_parameter, test_posix_set_get_lfc_host_parameter);
  return tc_parameter;  
}

TestSuite* posix_xattr_suite(){
  TestSuite* tc_xattr = create_test_suite();  
  add_test(tc_xattr, gfal2_test_getxattr_guid_lfn_base);
  add_test(tc_xattr, gfal2_test_getxattr_status_srm_base);
  return tc_xattr;  
}

TestSuite* skip_list_tests(){
  TestSuite* test_skiplist = create_test_suite();  
  add_test(test_skiplist, test_gskiplist_create_delete);
  add_test(test_skiplist, test_gskiplist_insert_len);
  add_test(test_skiplist, test_gskiplist_insert_search_remove);
  add_test(test_skiplist, test_gskiplist_insert_get_clean);
  add_test(test_skiplist, test_gskiplist_insert_multi);
  return test_skiplist;  
}

TestSuite* posix_access_suite (void)
{
  TestSuite* tc_access = create_test_suite();
  add_test(tc_access, gfal2_test_access_posix_guid_exist);
  add_test(tc_access, gfal2_test_access_posix_guid_read);
  add_test(tc_access, gfal2_test_access_posix_guid_write);
  add_test(tc_access, gfal2_test_access_posix_lfn_exist);
  add_test(tc_access, gfal2_test_access_posix_lfn_read);
  add_test(tc_access, gfal2_test_access_posix_lfn_write);
  add_test(tc_access, gfal2_test_access_posix_srm_exist);
  add_test(tc_access, gfal2_test_access_posix_srm_read);
  add_test(tc_access, gfal2_test_access_posix_srm_write);
/*
//  TestSuite* tc_rename = create_test_suite();
// add_test(tc_rename, test__gfal_posix_rename_plugin);
 // add_test(tc_rename, test__gfal_posix_move_dir_plugin);
 // add_test(tc_rename, test__gfal_posix_rename_url_check);
//  add_test(tc_rename, test__gfal_posix_rename_local);
 // suite_add_tcase(s, tc_rename);
 // TestSuite* tc_opendir = create_test_suite();
  //add_test(tc_opendir, gfal2_test__opendir_posix_local_simple);
 // add_test(tc_opendir, gfal2_test__opendir_posix_lfc_simple);
 // add_test(tc_opendir, gfal2_test__readdir_posix_local_simple);
 // add_test(tc_opendir, gfal2_test__readdir_posix_lfc_simple);
//  add_test(tc_opendir, gfal2_test__opendir_posix_srm_simple_mock);
 // add_test(tc_opendir, gfal2_test__readdir_posix_srm_simple_mock);
 // add_test(tc_opendir, test__readdir_posix_srm_empty_mock);
 // suite_add_tcase(s, tc_opendir);
 // TestSuite* tc_open = create_test_suite();
 // add_test(tc_open, gfal2_test_open_posix_all_simple);
 // add_test(tc_open, gfal2_test_open_posix_local_simple);
 // add_test(tc_open, gfal2_test_open_posix_lfc_simple);
 // add_test(tc_open, gfal2_test_open_posix_srm_simple);
//  add_test(tc_open, gfal2_test_open_posix_guid_simple);
  suite_add_tcase(s, tc_open);*/
  return tc_access;
}




int main (int argc, char** argv)
{
	//fprintf(stderr, " tests : %s ", getenv("LD_LIBRARY_PATH"));
	TestSuite *global = create_test_suite();
	add_suite(global, verbose_suite());
	add_suite(global, plugin_suite());
	add_suite(global, srm_Suite());
	//add_suite(global, no_glib_suite());
	add_suite(global, lfc_suite());
	add_suite(global, mds_suite());
	add_suite(global, posix_access_suite());
	add_suite(global, posix_chmod_suite());
	add_suite(global, posix_mkdir_suite());
	add_suite(global, posix_stat_suite());
	add_suite(global, posix_open_suite());
	add_suite(global, posix_rfio_plugin_suite());
	add_suite(global, posix_read_suite());
	add_suite(global, posix_write_suite());
	add_suite(global, posix_opendir_suite());
	add_suite(global, posix_parameter_suite());
	add_suite(global, posix_xattr_suite());
	add_suite(global, skip_list_tests());
	//add_suite(global, filedesc_suite());
    if (argc > 1){
        return run_single_test(global, argv[1], create_text_reporter());
    }
	return run_test_suite(global, create_text_reporter());
}

