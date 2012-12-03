#pragma once
/* unit test for posix getxattr func */


#include <cgreen/cgreen.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>


void gfal2_test_getxattr_guid_lfn_base();

void gfal2_test_getxattr_guid(const char* good_lfn, const char* guid, const char* enoent_lfn, const char* eaccess_lfn);

void gfal2_test_getxattr_status_srm_base();
