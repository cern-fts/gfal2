#include <iostream>
#include "../../src/plugins/xrootd/gfal_xrootd_plugin_utils.h"


int assert_equal(std::string a, std::string b)
{
  if (a != b) {
    std::cout << a << " != " << b << std::endl;
    return 1;
  }
  else {
    std::cout <<  a << " == " << b << std::endl;
    return 0;
  }
}


int main(int argc, char *argv[])
{
  int nerr = 0;

  gfal2_context_t context = gfal2_context_new(NULL);
  if (!context)
      abort();

  nerr += assert_equal("root://host.domain///", normalize_url(context, "root://host.domain"));
  nerr += assert_equal("root://host.domain///", normalize_url(context, "root://host.domain/"));
  nerr += assert_equal("root://host.domain///", normalize_url(context, "root://host.domain//"));
  nerr += assert_equal("root://host.domain///path/file", normalize_url(context, "root://host.domain/path/file"));
  nerr += assert_equal("root://host.domain///path/file", normalize_url(context, "root://host.domain//path/file"));
  nerr += assert_equal("root://host.domain///path/file", normalize_url(context, "root://host.domain///path/file"));

  // Now, try setting the user credentials
  gfal2_set_opt_string(context, "X509", "CERT", "/tmp/1234", NULL);
  nerr += assert_equal("root://host.domain///?xrd.gsiusrpxy=/tmp/1234", normalize_url(context, "root://host.domain"));
  gfal2_set_opt_string(context, "X509", "KEY", "/tmp/1234", NULL);
  nerr += assert_equal("root://host.domain///?xrd.gsiusrpxy=/tmp/1234", normalize_url(context, "root://host.domain"));

  gfal2_set_opt_string(context, "X509", "KEY", "/tmp/1234-key", NULL);
  nerr += assert_equal("root://host.domain///?xrd.gsiusrcrt=/tmp/1234&xrd.gsiusrkey=/tmp/1234-key", normalize_url(context, "root://host.domain"));

  std::cout << nerr << " failures" << std::endl;

  return nerr;
}
