#include <utils/mds/gfal_mds_internal.h>
#include <gtest/gtest.h>
#include <fstream>


class MdsTestFixture : public ::testing::Test {
private:
    void generate_cache() {
        std::ofstream cache(MDS_CACHE_FILE, std::ios_base::out | std::ios_base::trunc);

        cache
            << "<?xml version=\"1.0\"?>" << std::endl
            << "<entry>" << std::endl
            << "    <endpoint>httpg://test.domain.com:8442/srm/managerv2</endpoint>" << std::endl
            << "    <sitename>TEST-PROD</sitename>" << std::endl
            << "    <type>SRM</type>" << std::endl
            << "    <version>2.2.0</version>" << std::endl
            << "</entry>" << std::endl;

        cache.flush();
    }

protected:
    gfal2_context_t context;

public:
    static const char *MDS_CACHE_FILE;

    MdsTestFixture() {
        generate_cache();
        GError *error = NULL;
        context = gfal2_context_new(&error);
        assert(context != NULL);

        int ret = gfal2_set_opt_string(context, "BDII", "CACHE_FILE", MDS_CACHE_FILE, NULL);
        assert(ret == 0);
    }

    ~MdsTestFixture() {
        unlink(MDS_CACHE_FILE);
        gfal2_context_free(context);
    }
};

const char *MdsTestFixture::MDS_CACHE_FILE = "/tmp/mds_cache.xml";


TEST_F(MdsTestFixture, test_cache_not_found)
{
    gfal_mds_endpoint endpoints[5];
    GError* err = NULL;
    int ret = gfal_mds_cache_resolve_endpoint(context, "noutfound.example.com", endpoints, 5, &err);
    ASSERT_EQ(err, (void*)NULL);
    ASSERT_EQ(ret, 0);
}


TEST_F(MdsTestFixture, test_cache_found)
{
    gfal_mds_endpoint endpoints[5];
    GError* err = NULL;
    int ret = gfal_mds_cache_resolve_endpoint(context, "test.domain.com", endpoints, 5, &err);
    ASSERT_EQ(err, (void*)NULL);
    ASSERT_EQ(ret, 1);

    ASSERT_EQ(endpoints[0].type, SRMv2);
    ASSERT_STREQ(endpoints[0].url, "httpg://test.domain.com:8442/srm/managerv2");
}
