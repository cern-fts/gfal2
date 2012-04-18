

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


#include <posix/gfal_posix_api.h>




void test_posix_set_get_parameter(){
    char buff[2048];
    buff[0]='\0';
  
    int ret = gfal_set_parameter_int(NULL, "no_bdii", TRUE); // set a variable
    assert_true_with_message(ret == 0 && gfal_posix_check_error() == FALSE, " must be a valid return");
    
    ret = gfal_get_parameter_int(NULL, "no_bdii"); // verify the variable status 
    
    assert_true_with_message(ret == TRUE && gfal_posix_check_error() == FALSE, " must be the value set before");
    
    ret = gfal_set_parameter_int(NULL, "no_bdii", FALSE); // set a variable
    assert_true_with_message(ret == 0 && gfal_posix_check_error() == FALSE, " must be a valid return 2");
    
    ret = gfal_get_parameter_int(NULL, "no_bdii"); // verify the variable status 
    
    assert_true_with_message(ret == FALSE && gfal_posix_check_error() == FALSE, " must be the value set before 2");

    
  
}

