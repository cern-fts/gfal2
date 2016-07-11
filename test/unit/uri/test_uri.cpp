#include <utils/uri/gfal2_uri.h>
#include <gtest/gtest.h>


TEST(gfalURI, regular_parsing)
{
    const char *URI = "gsiftp://dcache-door-desy09.desy.de:2811/pnfs/desy.de/dteam/gfal2-tests/testread0011";

    GError* tmp_err=NULL;

    gfal2_uri *parsed = gfal2_parse_uri(URI, &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("gsiftp", parsed->scheme);
    ASSERT_STREQ("dcache-door-desy09.desy.de", parsed->host);
    ASSERT_EQ(2811, parsed->port);
    ASSERT_STREQ("/pnfs/desy.de/dteam/gfal2-tests/testread0011", parsed->path);
    ASSERT_EQ(NULL, parsed->query);

    char *rebuilt = gfal2_join_uri(parsed);
    ASSERT_STREQ(URI, rebuilt);

    g_free(rebuilt);
    gfal2_free_uri(parsed);
}


TEST(gfalURI, malformed)
{
    GError* tmp_err=NULL;

    gfal2_uri *parsed = gfal2_parse_uri(
            "malformed",
            &tmp_err);

    ASSERT_EQ(parsed->scheme, (void*)NULL);

    gfal2_free_uri(parsed);
}


TEST(gfalURI, no_port)
{
    const char *URI = "https://some.domain.com/path";

    GError* tmp_err=NULL;

    gfal2_uri *parsed = gfal2_parse_uri(URI, &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("https", parsed->scheme);
    ASSERT_STREQ("some.domain.com", parsed->host);
    ASSERT_EQ(0, parsed->port);
    ASSERT_STREQ("/path", parsed->path);

    char *rebuilt = gfal2_join_uri(parsed);
    ASSERT_STREQ(URI, rebuilt);

    g_free(rebuilt);
    gfal2_free_uri(parsed);
}


TEST(gfalURI, ipv4)
{
    const char *URI = "gsiftp://192.168.1.1:1234/path";
    GError* tmp_err=NULL;
    gfal2_uri *parsed = gfal2_parse_uri(URI, &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("gsiftp", parsed->scheme);
    ASSERT_STREQ("192.168.1.1", parsed->host);
    ASSERT_EQ(1234, parsed->port);
    ASSERT_STREQ("/path", parsed->path);

    char *rebuilt = gfal2_join_uri(parsed);
    ASSERT_STREQ(URI, rebuilt);

    g_free(rebuilt);
    gfal2_free_uri(parsed);
}


TEST(gfalURI, ipv6)
{
    const char *URI ="gsiftp://[2001:1458:301:a8ae::100:24]:1234/path";
    GError* tmp_err=NULL;

    gfal2_uri *parsed = gfal2_parse_uri(URI, &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("gsiftp", parsed->scheme);
    ASSERT_STREQ("[2001:1458:301:a8ae::100:24]", parsed->host);
    ASSERT_EQ(1234, parsed->port);
    ASSERT_STREQ("/path", parsed->path);

    char *rebuilt = gfal2_join_uri(parsed);
    ASSERT_STREQ(URI, rebuilt);

    g_free(rebuilt);
    gfal2_free_uri(parsed);
}


TEST(gfalURI, userinfo)
{
    const char *URI ="gsiftp://user:patata@[2001:1458:301:a8ae::100:24]:1234/path";
    GError* tmp_err=NULL;

    gfal2_uri *parsed = gfal2_parse_uri(URI, &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("gsiftp", parsed->scheme);
    ASSERT_STREQ("[2001:1458:301:a8ae::100:24]", parsed->host);
    ASSERT_EQ(1234, parsed->port);
    ASSERT_STREQ("/path", parsed->path);
    ASSERT_STREQ("user:patata", parsed->userinfo);

    char *rebuilt = gfal2_join_uri(parsed);
    ASSERT_STREQ(URI, rebuilt);

    g_free(rebuilt);
    gfal2_free_uri(parsed);
}


TEST(gfalURI, fragmentAndQuery)
{
    const char *URI = "gsiftp://host/path?a=b&c=d#fragment";
    GError* tmp_err=NULL;

    gfal2_uri *parsed = gfal2_parse_uri(URI, &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("gsiftp", parsed->scheme);
    ASSERT_STREQ("host", parsed->host);
    ASSERT_STREQ("/path", parsed->path);
    ASSERT_STREQ("a=b&c=d", parsed->query);
    ASSERT_STREQ("fragment", parsed->fragment);

    char *rebuilt = gfal2_join_uri(parsed);
    ASSERT_STREQ(URI, rebuilt);

    g_free(rebuilt);
    gfal2_free_uri(parsed);
}


TEST(gfalURI, file)
{
    const char *URI = "file:///path";
    GError* tmp_err=NULL;

    // With authority
    gfal2_uri *parsed = gfal2_parse_uri(URI, &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("file", parsed->scheme);
    ASSERT_STREQ("", parsed->host);
    ASSERT_STREQ("/path", parsed->path);

    gfal2_free_uri(parsed);

    // Without authority
    parsed = gfal2_parse_uri("file:/path", &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("file", parsed->scheme);
    ASSERT_STREQ(NULL, parsed->host);
    ASSERT_STREQ("/path", parsed->path);

    char *rebuilt = gfal2_join_uri(parsed);
    ASSERT_STREQ(URI, rebuilt);

    g_free(rebuilt);
    gfal2_free_uri(parsed);
}

TEST(gfalURI, special)
{
    const char *URI = "davs://arioch.cern.ch/dpm/cern.ch/home/dteam/xrd-f1460564827990430820aBc0!@%23f%25%5e_-+=:%20.dat";
    GError* tmp_err = NULL;

    gfal2_uri *parsed = gfal2_parse_uri(URI, &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("davs", parsed->scheme);
    ASSERT_STREQ("arioch.cern.ch", parsed->host);
    ASSERT_STREQ("/dpm/cern.ch/home/dteam/xrd-f1460564827990430820aBc0!@%23f%25%5e_-+=:%20.dat", parsed->path);
    ASSERT_EQ(NULL, parsed->query);
    ASSERT_EQ(NULL, parsed->fragment);

    char *rebuilt = gfal2_join_uri(parsed);
    ASSERT_STREQ(URI, rebuilt);

    g_free(rebuilt);

    gfal2_urldecode(parsed->path);
    ASSERT_STREQ("/dpm/cern.ch/home/dteam/xrd-f1460564827990430820aBc0!@#f%^_-+=: .dat", parsed->path);

    gfal2_free_uri(parsed);
}
