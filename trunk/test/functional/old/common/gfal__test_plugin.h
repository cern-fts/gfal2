#pragma once

/* unit test for common_plugin */

#include <stdio.h>
#include "lfc/lfc_ifce_ng.h"

struct lfc_ops* find_lfc_ops(gfal_handle handle, GError** err);

void test_mock_lfc(gfal_handle handle, GError** err);


void gfal2_test_plugin_access_file();

void gfal2_test_plugin_url_checker();

void gfal2_test__plugin_stat();

void gfal2_test__plugin_lstat();
