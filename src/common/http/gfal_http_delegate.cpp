#include <glib.h>
#include <gridsite.h>
#include <stdsoap2.h>
#include "gfal_http_plugin.h"
#include "soapH.h"

#include "DelegationSoapBinding.nsmap"

const char * default_ca_path= "/etc/grid-security/certificates/";

/// Do the delegation
char *gfal_http_delegate(const std::string& urlpp, GError** err)
{
  char                               *delegation_id = NULL;
  std::string                        *reqtxt  = NULL;
  char                               *certtxt = NULL;
  char                               *keycert = NULL;
  struct soap                        *soap_get = NULL, *soap_put = NULL;
  struct tns__getNewProxyReqResponse  getNewProxyReqResponse;
  struct tns__putProxyResponse        putProxyResponse;
  std::string                         ucert, ukey, capath;
  int                                 lifetime;
  const char* url = urlpp.c_str();
  char        err_buffer[512];
  size_t      err_aux;

  // TODO: Get from the environment or something
  lifetime = 12 * 60 * 60; // 12h

  // Get certificate
  gfal_http_get_ucert(ucert, ukey);

  if (ucert.empty() || ukey.empty()) {
    *err = g_error_new(http_plugin_domain, EINVAL,
                       "Could not set the user's proxy or certificate");
    return NULL;
  }

  if (getenv("X509_CA_PATH"))
      capath = getenv("X509_CA_PATH");
  else
    capath = (char*)default_ca_path ;

  // Cert and key need to be in the same file
  if (ucert == ukey) {
    keycert = strdup(ucert.c_str());
  }
  else {
    FILE *ifp, *ofp;
    int   fd;
    char  c;

    keycert = strdup("/tmp/.XXXXXX");

    fd = mkstemp(keycert);
    ofp = fdopen(fd, "w");

    ifp = fopen(ukey.c_str(), "r");
    while ((c = fgetc(ifp)) != EOF) fputc(c, ofp);
    fclose(ifp);

    ifp = fopen(ukey.c_str(), "r");
    while ((c = fgetc(ifp)) != EOF) fputc(c, ofp);
    fclose(ifp);

    fclose(ofp);
  }

  // Initialize SSL
  ERR_load_crypto_strings ();
  OpenSSL_add_all_algorithms();

  // Request a new delegation ID
  soap_get = soap_new();
  soap_get->keep_alive = 1;

  if (soap_ssl_client_context(soap_get, SOAP_SSL_DEFAULT, keycert, "",
                              NULL, capath.c_str(), NULL) == 0) {
    soap_call_tns__getNewProxyReq(soap_get,
                                  url,
                                  "http://www.gridsite.org/namespaces/delegation-1",
                                  getNewProxyReqResponse);

    if(soap_get->error == 0) {
      reqtxt        = getNewProxyReqResponse.getNewProxyReqReturn->proxyRequest;
      delegation_id = strdup(getNewProxyReqResponse.getNewProxyReqReturn->delegationID->c_str());

      // Generate proxy
      if (GRSTx509MakeProxyCert(&certtxt, stderr, (char*)reqtxt->c_str(),
                                (char*)ucert.c_str(), (char*)ukey.c_str(), lifetime) == GRST_RET_OK) {
        // Submit the proxy
        soap_put = soap_new();

        if (soap_ssl_client_context(soap_put, SOAP_SSL_DEFAULT, keycert, "",
                              NULL, capath.c_str(), NULL) == 0) {
            soap_call_tns__putProxy(soap_put,
                                    url,
                                    "http://www.gridsite.org/namespaces/delegation-1",
                                    delegation_id, certtxt, putProxyResponse);
            if (soap_put->error) {
              // Could not PUT
              err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not PUT the proxy: ");
              soap_sprint_fault(soap_put, err_buffer + err_aux, sizeof(err_buffer) - err_aux);
              *err = g_error_new(http_plugin_domain, EACCES,
                                 "Could not send the proxy: %s", err_buffer);
            }
        }
        else { // soap_put ssl error
          err_aux = snprintf(err_buffer, sizeof(err_buffer), "Connection error on proxy put: ");
          soap_sprint_fault(soap_put, err_buffer + err_aux, sizeof(err_buffer) - err_aux);
          *err = g_error_new(http_plugin_domain, EACCES,
                             "Could not connect to the delegation endpoint: %s", err_buffer);
        }

        soap_free(soap_put);
      }
      else {
        *err = g_error_new(http_plugin_domain, EACCES,
                "Could not generate the proxy: %s", err_buffer);
      }
    }
    else { // Could not get ID
      err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not get proxy request: ");
      soap_sprint_fault(soap_get, err_buffer + err_aux, sizeof(err_buffer) - err_aux);
      *err = g_error_new(http_plugin_domain, EACCES,
              "Could not get the delegation id: %s", err_buffer);
    }
  }
  else { // soap_get ssl error
    err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not connect to get the proxy request: ");
    soap_sprint_fault(soap_get, err_buffer + err_aux, sizeof(err_buffer) - err_aux);
    *err = g_error_new(http_plugin_domain, EACCES,
            "Could not connect to the delegation endpoint: %s", err_buffer);
  }

  // Clean soap_get
  soap_free(soap_get);
  free(keycert);
  free(certtxt);

  // Return delegation ID
  return delegation_id;
}
