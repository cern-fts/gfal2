#ifdef USE_GNUTLS
#include <gnutls/pkcs12.h>
#else
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#endif
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <status/davix_error.h>
#include "gfal_http_plugin.h"

// This function take a user certificate and a private key in x509 format and
// convert it into pkcs12 format. It returns -1 if a problem occurs, 0 otherwise
// Adapted from lcgdm-dav CNS API wrapper

#ifdef USE_GNUTLS

int convert_x509_to_p12(const char *privkey, const char *clicert, const char *p12cert, davix_error_t* err)
{
  gnutls_datum_t        data, keyid;
  struct stat           fstat;
  FILE                 *fhandler;
  gnutls_x509_crt_t     cert;
  gnutls_x509_privkey_t key;
  gnutls_pkcs12_t       p12;
  gnutls_pkcs12_bag_t   certbag, keybag;
  int                   r, index;
  unsigned char         buffer[10 * 1024];
  size_t                buffer_size;

  /* Init P12 */
  gnutls_pkcs12_init(&p12);

  /* Read the private key file */
  if (stat(privkey, &fstat) != 0) {
    davix_error_setup(err, http_module_name, errno,
                      "Could not stat the user's private key");
    return -1;
  }
  data.size = fstat.st_size;
  data.data = buffer;

  fhandler = fopen(privkey, "rb");
  if (!fhandler) {
    davix_error_setup(err, http_module_name, errno,
                      "Could not open the user's private key");
    return -1;
  }

  fread(data.data, sizeof(unsigned char), data.size, fhandler);

  gnutls_x509_privkey_init(&key);
  r = gnutls_x509_privkey_import(key, &data, GNUTLS_X509_FMT_PEM);
  if (r < 0) {
    davix_error_setup(err, http_module_name, EACCES,
                      gnutls_strerror(r));
    return -1;
  }

  buffer_size = sizeof(buffer);
  gnutls_x509_privkey_get_key_id (key, 0, buffer, &buffer_size);

  keyid.size = buffer_size;
  keyid.data = malloc(sizeof(unsigned char) * keyid.size);
  memcpy(keyid.data, buffer, keyid.size);

  index = gnutls_pkcs12_bag_init(&keybag);
  gnutls_pkcs12_bag_set_key_id(keybag, index, &keyid);


  buffer_size = sizeof(buffer);
  gnutls_x509_privkey_export_pkcs8 (key, GNUTLS_X509_FMT_DER,
                                    "", GNUTLS_PKCS8_USE_PKCS12_3DES,
                                    buffer, &buffer_size);
  data.size = buffer_size;
  gnutls_pkcs12_bag_set_data(keybag, GNUTLS_BAG_PKCS8_ENCRYPTED_KEY, &data);

  fclose(fhandler);

  /* Read the user certificate */
  if (stat(clicert, &fstat) != 0) {
    davix_error_setup(err, http_module_name, errno,
                      "Could not stat the user's certificate");
    return -1;
  }
  data.size = fstat.st_size;

  fhandler = fopen(clicert, "rb");
  if (!fhandler) {
    davix_error_setup(err, http_module_name, errno,
                      "Could not open the user's private key");
    return -1;
  }

  fread(data.data, sizeof(unsigned char), data.size, fhandler);

  gnutls_x509_crt_init(&cert);
  r = gnutls_x509_crt_import(cert, &data, GNUTLS_X509_FMT_PEM);
  if (r < 0) {
    davix_error_setup(err, http_module_name, EACCES,
                      gnutls_strerror(r));
    return -1;
  }

  index = gnutls_pkcs12_bag_init(&certbag);
  gnutls_pkcs12_bag_set_crt(certbag, cert);
  gnutls_pkcs12_bag_set_key_id(certbag, index, &keyid);

  fclose(fhandler);

  /* Generate P12 */
  gnutls_pkcs12_init(&p12);
  gnutls_pkcs12_set_bag(p12, certbag);
  gnutls_pkcs12_set_bag(p12, keybag);
  gnutls_pkcs12_generate_mac(p12, "");

  buffer_size = sizeof(buffer);
  r = gnutls_pkcs12_export (p12, GNUTLS_X509_FMT_DER, buffer, &buffer_size);

  if (r < 0) {
    davix_error_setup(err, http_module_name, EACCES,
                      gnutls_strerror(r));
    return -1;
  }

  fhandler = fopen(p12cert, "wb");
  if (!fhandler) {
    davix_error_setup(err, http_module_name, errno,
                      "Could not write the p12 file");
    return -1;
  }

  fwrite(buffer, buffer_size, 1, fhandler);

  fclose(fhandler);

  /* Release */
  gnutls_pkcs12_deinit(p12);
  gnutls_pkcs12_bag_deinit(certbag);
  gnutls_pkcs12_bag_deinit(keybag);
  gnutls_x509_crt_deinit(cert);
  free(keyid.data);

  return 0;
}

#else

int convert_x509_to_p12(const char *privkey, const char *clicert, const char *p12cert, davix_error_t* err)
{
  SSL_CTX* ctx;
  char errbuffer[128];

  OpenSSL_add_all_algorithms();
  ERR_load_ERR_strings();

  ctx = SSL_CTX_new(SSLv23_client_method());

  // Set CA's
  if (SSL_CTX_load_verify_locations(ctx, clicert, "/etc/grid-security/certificates") != 1) {
    ERR_error_string_n(ERR_get_error(), errbuffer, sizeof(errbuffer));
    davix_error_setup(err, http_module_name, EACCES, errbuffer);
    SSL_CTX_free(ctx);
    return -1;
  }

  // Load private key
  EVP_PKEY* key;
  FILE* keyfile = fopen(privkey, "r");
  if (!keyfile) {
    davix_error_setup(err, http_module_name, errno, "Could not open the user's private key");
    SSL_CTX_free(ctx);
    return -1;
  }

  key = PEM_read_PrivateKey(keyfile, NULL, NULL, NULL);
  fclose(keyfile);
  if (!key) {
    ERR_error_string_n(ERR_get_error(), errbuffer, sizeof(errbuffer));
    davix_error_setup(err, http_module_name, EACCES, errbuffer);
    SSL_CTX_free(ctx);
    return -1;
  }

  // Load certificate
  if (SSL_CTX_use_certificate_chain_file(ctx, clicert) != 1 ||
      SSL_CTX_use_PrivateKey(ctx, key) != 1) {
    ERR_error_string_n(ERR_get_error(), errbuffer, sizeof(errbuffer));
    davix_error_setup(err, http_module_name, EACCES, errbuffer);
    SSL_CTX_free(ctx);
    EVP_PKEY_free(key);
    return -1;
  }

  if (SSL_CTX_check_private_key(ctx) != 1) {
    davix_error_setup(err, http_module_name, EACCES, "User's certificate and key don't match");
    SSL_CTX_free(ctx);
    EVP_PKEY_free(key);
    return -1;
  }


  // Generate the p12 certificate
  SSL*    ssl = SSL_new(ctx);
  PKCS12 *p12;
  FILE   *p12file;
  int     bytes = 0;

  if ((p12 = PKCS12_new()) == NULL){
    davix_error_setup(err, http_module_name, EACCES,
                      "Could not create the PKCS12 structure");
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    EVP_PKEY_free(key);
    return -1;
  }
  p12 = PKCS12_create("", NULL, key , SSL_get_certificate(ssl),
                      ctx->extra_certs,
                      0, 0, 0, 0, 0);

  if ( p12 == NULL) {
    davix_error_setup(err, http_module_name, EACCES,
                      "Error generating a valid PKCS12 certificate");
    PKCS12_free(p12);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    EVP_PKEY_free(key);
    return -1;
  }

  if (! (p12file = fopen(p12cert, "w"))){
    davix_error_setup(err, http_module_name, errno,
                      "Could not write the p12 file");
    PKCS12_free(p12);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    EVP_PKEY_free(key);
    return -1;
  }

  bytes = i2d_PKCS12_fp(p12file, p12);
  fclose(p12file);
  if (bytes <= 0){
    davix_error_setup(err, http_module_name, errno,
                      "Error writing the p12 file");
    PKCS12_free(p12);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    EVP_PKEY_free(key);
    return -1;
  }

  PKCS12_free(p12);
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  EVP_PKEY_free(key);

  return 0;
}

#endif
