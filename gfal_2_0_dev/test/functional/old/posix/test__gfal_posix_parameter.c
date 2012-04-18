

/* unit test for the set/get parameters */


#include <cgreen/cgreen.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <glib.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>
#include <common/mds/gfal_common_mds.h>


#include <posix/gfal_posix_api.h>




void test_posix_set_get_parameter(){
    char buff[2048];
    buff[0]='\0';
  
    int ret = gfal_set_parameter_boolean("core", "no_bdii", TRUE); // set a variable
    assert_true_with_message(ret == 0 && gfal_posix_check_error() == FALSE, " must be a valid return %d %d ", ret , gfal_posix_code_error());
    
    ret = gfal_get_parameter_boolean("core", "no_bdii"); // verify the variable status 
    
    assert_true_with_message(ret == TRUE && gfal_posix_check_error() == FALSE, " must be the value set before");
    
    ret = gfal_set_parameter_boolean("core", "no_bdii", FALSE); // set a variable
    assert_true_with_message(ret == 0 && gfal_posix_check_error() == FALSE, " must be a valid return 2");
    
    ret = gfal_get_parameter_boolean("core", "no_bdii"); // verify the variable status 
    
    assert_true_with_message(ret == FALSE && gfal_posix_check_error() == FALSE, " must be the value set before 2");

    
}

void test_posix_set_get_false_parameter(){
    char buff[2048];
    buff[0]='\0';
  
    int ret = gfal_set_parameter_boolean(NULL, NULL, TRUE); // set a variable NULL
    assert_true_with_message(ret == -1 && gfal_posix_code_error() == EINVAL, " must be an error for the set");
	gfal_posix_clear_error();
	
    ret = gfal_set_parameter_boolean("lfc", NULL, TRUE); // set a variable NULL with namespace
    assert_true_with_message(ret == -1 && gfal_posix_code_error() == EINVAL, " must be an error for the set 2");
	gfal_posix_clear_error();
	
    ret = gfal_get_parameter_boolean("lfc", NULL); // get a variable NULL with namespace
    assert_true_with_message(ret == -1 && gfal_posix_code_error() == EINVAL, " must be an error for the get");
	gfal_posix_clear_error();
	
	ret = gfal_get_parameter_boolean("core", "unexisting_parameter"); // get an unexisting variable
    assert_true_with_message(ret == -1 && gfal_posix_code_error() == ENOENT, " must be an error for the get %d %d", gfal_posix_code_error(),ret);
	
	ret = gfal_set_parameter_boolean("core", "unexisting_parameter", TRUE); // set an unexisting variable
    assert_true_with_message(ret == -1 && gfal_posix_code_error() == ENOENT, " must be an error for the set %d %d", gfal_posix_code_error(), ret);	
	gfal_posix_clear_error();   
}


void test_posix_set_get_infosys_parameter(){

	char * old_infosys, * new_infosys, *param;
	old_infosys = getenv(bdii_env_var);
	assert_true_with_message(old_infosys != NULL, " must be a valid infosys var");
  
    int ret = gfal_set_parameter_string("core", "infosys", ""); // set a variable to empty
    assert_true_with_message(ret == 0 && gfal_posix_code_error() == 0, " must be a valid set %d %d", ret, gfal_posix_code_error());
	gfal_posix_clear_error();

	new_infosys = getenv(bdii_env_var);	
	assert_true_with_message(strcmp(new_infosys,"")==0, " must be the new infosys value");
	
    param = gfal_get_parameter_string("core", "infosys"); // set a variable NULL with namespace
    assert_true_with_message(ret == 0 && gfal_posix_code_error() == 0 && strcmp("",param) ==0, " must be a valid empty infosys");
    free(param);
	gfal_posix_clear_error();
	
    ret = gfal_set_parameter_string("core", "infosys", old_infosys); // set a variable to the old value 
    assert_true_with_message(ret == 0 && gfal_posix_code_error() == 0, " must be a valid set");
	gfal_posix_clear_error();
	
    param = gfal_get_parameter_string("core", "infosys"); // get the new value
    assert_true_with_message(ret == 0 && gfal_posix_code_error() == 0 && strcmp(old_infosys,param) ==0, " must be a valid old infosys");
    free(param);
	gfal_posix_clear_error();
	
	new_infosys = getenv(bdii_env_var);	// verify the set
	assert_true_with_message(strcmp(new_infosys, old_infosys)==0, " must be the old infosys value");
}


void test_posix_set_get_lfc_host_parameter(){

	const char * lfc_host = "LFC_HOST";
	char * old_lfchost, * new_lfchost, *param;
	old_lfchost = getenv(lfc_host);
	assert_true_with_message(old_lfchost != NULL, " must be a valid lfc host var");
  
    int ret = gfal_set_parameter_string("lfc", "host", ""); // set a variable to empty
    assert_true_with_message(ret == 0 && gfal_posix_code_error() == 0, " must be a valid set %d %d", ret, gfal_posix_code_error());
	gfal_posix_clear_error();

	new_lfchost = getenv(lfc_host);	
	assert_true_with_message(strcmp(new_lfchost,"")==0, " must be the new lfc value");
	
    param = gfal_get_parameter_string("lfc", "host"); // set a variable NULL with namespace
    assert_true_with_message(ret == 0 && gfal_posix_code_error() == 0 && strcmp("",param) ==0, " must be a valid empty lfc");
    free(param);
	gfal_posix_clear_error();
	
    ret = gfal_set_parameter_string("lfc", "host", old_lfchost); // set a variable to the old value 
    assert_true_with_message(ret == 0 && gfal_posix_code_error() == 0, " must be a valid set");
	gfal_posix_clear_error();
	
    param = gfal_get_parameter_string("lfc", "host"); // get the new value
    assert_true_with_message(ret == 0 && gfal_posix_code_error() == 0 && strcmp(old_lfchost,param) ==0, " must be a valid old lfc");
    free(param);
	gfal_posix_clear_error();
	
	new_lfchost = getenv(lfc_host);	// verify the set
	assert_true_with_message(strcmp(new_lfchost, old_lfchost)==0, " must be the old lfc value");
}




