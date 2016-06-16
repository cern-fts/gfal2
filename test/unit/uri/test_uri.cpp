#include <utils/uri/gfal2_uri.h>
#include <gtest/gtest.h>


TEST(gfalURI, regular_parsing)
{
    GError* tmp_err=NULL;

    gfal2_uri *parsed = gfal2_parse_uri(
            "gsiftp://dcache-door-desy09.desy.de:2811/pnfs/desy.de/dteam/gfal2-tests/testread0011",
            &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("gsiftp", parsed->scheme);
    ASSERT_STREQ("dcache-door-desy09.desy.de", parsed->host);
    ASSERT_EQ(2811, parsed->port);
    ASSERT_STREQ("/pnfs/desy.de/dteam/gfal2-tests/testread0011", parsed->path);
    ASSERT_EQ(NULL, parsed->query);

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
    GError* tmp_err=NULL;

    gfal2_uri *parsed = gfal2_parse_uri(
            "https://some.domain.com/path",
            &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("https", parsed->scheme);
    ASSERT_STREQ("some.domain.com", parsed->host);
    ASSERT_EQ(0, parsed->port);
    ASSERT_STREQ("/path", parsed->path);

    gfal2_free_uri(parsed);
}


TEST(gfalURI, ipv4)
{
    GError* tmp_err=NULL;
    gfal2_uri *parsed = gfal2_parse_uri(
        "gsiftp://192.168.1.1:1234/path",
        &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("gsiftp", parsed->scheme);
    ASSERT_STREQ("192.168.1.1", parsed->host);
    ASSERT_EQ(1234, parsed->port);
    ASSERT_STREQ("/path", parsed->path);

    gfal2_free_uri(parsed);
}


TEST(gfalURI, ipv6)
{
    GError* tmp_err=NULL;

    gfal2_uri *parsed = gfal2_parse_uri(
        "gsiftp://[2001:1458:301:a8ae::100:24]:1234/path",
        &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("gsiftp", parsed->scheme);
    ASSERT_STREQ("[2001:1458:301:a8ae::100:24]", parsed->host);
    ASSERT_EQ(1234, parsed->port);
    ASSERT_STREQ("/path", parsed->path);

    gfal2_free_uri(parsed);
}


TEST(gfalURI, userinfo)
{
    GError* tmp_err=NULL;

    gfal2_uri *parsed = gfal2_parse_uri(
        "gsiftp://user:patata@[2001:1458:301:a8ae::100:24]:1234/path",
        &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("gsiftp", parsed->scheme);
    ASSERT_STREQ("[2001:1458:301:a8ae::100:24]", parsed->host);
    ASSERT_EQ(1234, parsed->port);
    ASSERT_STREQ("/path", parsed->path);
    ASSERT_STREQ("user:patata", parsed->userinfo);

    gfal2_free_uri(parsed);
}


TEST(gfalURI, fragmentAndQuery)
{
    GError* tmp_err=NULL;

    gfal2_uri *parsed = gfal2_parse_uri(
        "gsiftp://host/path?a=b&c=d#fragment",
        &tmp_err);

    ASSERT_NE(parsed, (void*)NULL);

    ASSERT_STREQ("gsiftp", parsed->scheme);
    ASSERT_STREQ("host", parsed->host);
    ASSERT_STREQ("/path", parsed->path);
    ASSERT_STREQ("a=b&c=d", parsed->query);
    ASSERT_STREQ("fragment", parsed->fragment);

    gfal2_free_uri(parsed);
}


TEST(gfalURI, file)
{
    GError* tmp_err=NULL;

    // With authority
    gfal2_uri *parsed = gfal2_parse_uri("file:///path", &tmp_err);

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
    gfal2_free_uri(parsed);
}
