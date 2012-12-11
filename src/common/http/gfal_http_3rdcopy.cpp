#include <glib.h>
#include <davix.hpp>
#include <gridsite.h>
#include <unistd.h>
#include <stdsoap2.h>
#include "gfal_http_plugin.h"
#include "soapH.h"

#include "DelegationSoapBinding.nsmap"

const char * default_ca_path= "/etc/grid-security/certificates/";

/// Do the delegation
static char *gfal_http_delegate(const std::string& urlpp, Davix::DavixError** daverr)
{
  char                               *delegation_id = NULL;
  std::string                        *reqtxt  = NULL;
  char                               *certtxt = NULL;
  char                               *keycert = NULL;
  struct soap                        *soap_get = NULL, *soap_put = NULL;
  struct tns__getNewProxyReqResponse  getNewProxyReqResponse;
  struct tns__putProxyResponse        putProxyResponse;
  char                               *ucert = NULL, *ukey = NULL, *capath = NULL;
  int                                 lifetime;
  const char* url = urlpp.c_str();
  char        err_buffer[512];
  size_t      err_aux;

  // TODO: Get from the environment or something
  lifetime = 12 * 60 * 60; // 12h

  // Get certificate
  ucert = ukey = getenv("X509_USER_PROXY");
  if (!ucert) {
    ucert = getenv("X509_USER_CERT");
    ukey  = getenv("X509_USER_KEY");
  }

  if (!ucert || !ukey) {
    *daverr = new Davix::DavixError(http_module_name, Davix::StatusCode::CredentialNotFound,
                                    "Could not set the user's proxy or certificate");
    return NULL;
  }

  capath = getenv("X509_CA_PATH");
  if (!capath)
    capath = (char*)default_ca_path ;

  // Cert and key need to be in the same file
  if (strcmp(ucert, ukey) == 0) {
    keycert = strdup(ucert);
  }
  else {
    FILE *ifp, *ofp;
    int   fd;
    char  c;

    keycert = strdup("/tmp/.XXXXXX");

    fd = mkstemp(keycert);
    ofp = fdopen(fd, "w");

    ifp = fopen(ukey, "r");
    while ((c = fgetc(ifp)) != EOF) fputc(c, ofp);
    fclose(ifp);

    ifp = fopen(ukey, "r");
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
                              NULL, capath, NULL) == 0) {
    soap_call_tns__getNewProxyReq(soap_get,
                                  url,
                                  "http://www.gridsite.org/namespaces/delegation-1",
                                  getNewProxyReqResponse);

    if(soap_get->error == 0) {
      reqtxt        = getNewProxyReqResponse.getNewProxyReqReturn->proxyRequest;
      delegation_id = strdup(getNewProxyReqResponse.getNewProxyReqReturn->delegationID->c_str());

      // Generate proxy
      if (GRSTx509MakeProxyCert(&certtxt, stderr, (char*)reqtxt->c_str(),
                                ucert, ukey, lifetime) == GRST_RET_OK) {
        // Submit the proxy
        soap_put = soap_new();

        if (soap_ssl_client_context(soap_put, SOAP_SSL_DEFAULT, keycert, "",
                              NULL, capath, NULL) == 0) {
            soap_call_tns__putProxy(soap_put,
                                    url,
                                    "http://www.gridsite.org/namespaces/delegation-1",
                                    delegation_id, certtxt, putProxyResponse);
            if (soap_put->error) {
              // Could not PUT
              err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not PUT the proxy: ");
              soap_sprint_fault(soap_put, err_buffer + err_aux, sizeof(err_buffer) - err_aux);
              *daverr = new Davix::DavixError(http_module_name, Davix::StatusCode::AuthentificationError,
                                              err_buffer);
            }
        }
        else { // soap_put ssl error
          err_aux = snprintf(err_buffer, sizeof(err_buffer), "Connection error on proxy put: ");
          soap_sprint_fault(soap_put, err_buffer + err_aux, sizeof(err_buffer) - err_aux);
          *daverr = new Davix::DavixError(http_module_name, Davix::StatusCode::AuthentificationError,
                                          err_buffer);
        }

        soap_free(soap_put);
      }
      else {
        *daverr = new Davix::DavixError(http_module_name, Davix::StatusCode::AuthentificationError,
                                        "Could not generate the proxy");
      }
    }
    else { // Could not get ID
      err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not get proxy request: ");
      soap_sprint_fault(soap_get, err_buffer + err_aux, sizeof(err_buffer) - err_aux);
      *daverr = new Davix::DavixError(http_module_name, Davix::StatusCode::AuthentificationError,
                                      err_buffer);
    }
  }
  else { // soap_get ssl error
    err_aux = snprintf(err_buffer, sizeof(err_buffer), "Could not connect to get the proxy request: ");
    soap_sprint_fault(soap_get, err_buffer + err_aux, sizeof(err_buffer) - err_aux);
    *daverr = new Davix::DavixError(http_module_name, Davix::StatusCode::AuthentificationError,
                                    err_buffer);
  }

  // Clean soap_get
  soap_free(soap_get);
  free(keycert);
  free(certtxt);

  // Return delegation ID
  return delegation_id;
}



int gfal_http_3rdcopy(plugin_handle plugin_data, gfal2_context_t context, gfalt_params_t params,
                      const char* src, const char* dst, GError** err)
{
  GfalHttpInternal* davix = static_cast<GfalHttpInternal*>(plugin_data);
  Davix::DavixError* daverr = NULL;

  // Follow jumps until final source
  Davix::RequestParams requestParams(*davix->params);
  Davix::HttpRequest* request = NULL;
  std::string realSrc(src);

  requestParams.setTransparentRedirectionSupport(false);

  do {
    delete request;
    gfal_log(GFAL_VERBOSE_TRACE, "\t\t%s: Next hop = '%s'", __func__, realSrc.c_str());
    request = davix->context->createRequest(realSrc, &daverr);
    if (daverr) {
      davix2gliberr(daverr, err);
      delete daverr;
      return -1;
    }

    request->setRequestMethod("HEAD");
    request->setParameters(requestParams);
    request->executeRequest(&daverr);
    if (daverr) {
      davix2gliberr(daverr, err);
      delete daverr;
      return -1;
    }
  }
  while (request->getAnswerHeader("Location", realSrc));

  if (request->getRequestCode() > 299) {
    g_set_error(err, http_plugin_domain, EACCES,
                "HEAD failed with code '%d'", request->getRequestCode());
    return -1;
  }

  // Delegate on the final source
  std::string delegationEndpoint;
  if (!request->getAnswerHeader("Proxy-Delegation-Service", delegationEndpoint)) {
    g_set_error(err, http_plugin_domain, EACCES,
                "Can not determine the delegation endpoint for '%s; ",
                realSrc.c_str());
  }
  delete request;
  request = NULL;

  Davix::Uri srcUri(realSrc);
  delegationEndpoint = std::string("https://") + srcUri.getHost() + "/" + delegationEndpoint;
  gfal_log(GFAL_VERBOSE_TRACE, "\t\t%s: Delegation endpoint = '%s'", __func__, delegationEndpoint.c_str());

  char* delegation_id = gfal_http_delegate(delegationEndpoint, &daverr);
  if (delegation_id == NULL) {
    davix2gliberr(daverr, err);
    delete daverr;
    return -1;
  }

  gfal_log(GFAL_VERBOSE_TRACE, "\t\t%s: Got delegation ID '%s'", __func__, delegation_id);

  // Perform the actual copy
  // Note: Must be always HTTPS
  realSrc = std::string("https://") + srcUri.getHost() + "/" + srcUri.getPath() + "?" + srcUri.getQuery() + std::string("&delegation=") + delegation_id;

  request = davix->context->createRequest(realSrc, &daverr);
  if (daverr) {
    davix2gliberr(daverr, err);
    delete daverr;
    return -1;
  }

  request->setParameters(davix->params);
  request->addHeaderField("Destination", dst);
  request->setRequestMethod("COPY");

  gfal_log(GFAL_VERBOSE_TRACE, "\t\t%s: Using method 'COPY'", __func__);
  gfal_log(GFAL_VERBOSE_TRACE, "\t\t%s: Using final source '%s'", __func__, realSrc.c_str());
  gfal_log(GFAL_VERBOSE_TRACE, "\t\t%s: Using destination '%s'", __func__, dst);

  if (request->beginRequest(&daverr) < 0) {
    davix2gliberr(daverr, err);
    delete daverr;
    return -1;
  }

  // Read feedback
  int ret = 0;
  std::string line;
  char buffer;
  while (request->readBlock(&buffer, 1, &daverr) == 1) {
    if (buffer == '\n') {
      const char *strPtr = line.c_str();

      if (strncasecmp("success", strPtr, 7) == 0) {
        break;
      }
      else if (strncasecmp("aborted", strPtr, 7) == 0 ||
               strncasecmp("failed", strPtr, 6) == 0) {
        ret = -1;
        g_set_error(err, http_plugin_domain, EIO,
                    "%s", strPtr);
        break;
      }
      else {
        long long partial, total;
        if (sscanf(strPtr, "%lld/%lld", &partial, &total) == 2) {
            gfal_log(GFAL_VERBOSE_TRACE, "\t\t%s: %lld/%lld", __func__, partial, total);
        }
        else {
          gfal_log(GFAL_VERBOSE_TRACE, "\t\t%s: Got unexpected line: '%s'", __func__, strPtr);
        }
      }

      line.clear();
    }
    else {
      line.append(&buffer, 1);
    }
  }

  // Leave
  request->endRequest(&daverr);

  if (request->getRequestCode() > 299) {
    g_set_error(err, http_plugin_domain, EIO,
                "COPY method failed with code '%d'", request->getRequestCode());
    ret = -1;
  }

  return ret;
}



int gfal_http_3rdcopy_check(plugin_handle plugin_data,  const char* src, const char* dst, gfal_url2_check check)
{
  if (check != GFAL_FILE_COPY) return 0;

  return (strncmp(src, "https://", 8) == 0 && strncmp(dst, "https://", 8) == 0);
}
