#include <gtest/gtest.h>
#include <fcntl.h>
#include <stdio.h>
#include <gfal_api.h>
#include <stdlib.h>
#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>
#include <utils/exceptions/gerror_to_cpp.h>


class RwSeqTest: public testing::Test {
public:
    static const char* root;
    static long block_size, file_size;

    char surl[2048];
    gfal2_context_t context;

    RwSeqTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~RwSeqTest() {
        gfal2_context_free(context);
    }

    virtual void SetUp() {
        generate_random_uri(root, "rwseq_test", surl, sizeof(surl));
    }

    virtual void TearDown() {
        GError* error = NULL;
        gfal2_unlink(context, surl, &error);
        g_clear_error(&error);
    }
};
const char* RwSeqTest::root;
long RwSeqTest::block_size = 0;
long RwSeqTest::file_size = 0;


TEST_F(RwSeqTest, WriteReadSeq)
{
    // Create
    char buffer[file_size];
    long i;
    for (i = 0; i < file_size; ++i)
        buffer[i] = i;

    GError* error = NULL;
    int fd = gfal2_open(context, surl, O_WRONLY | O_CREAT, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, fd, error);

    int ret;
    long j = 0, n = 0;
    i = 0;
    while (i < file_size) {
        j = ((i + block_size) < file_size) ? block_size : file_size - i;

        ret = gfal2_write(context, fd, buffer + i, j, &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

        i += j;
    }

    ret = gfal2_close(context, fd, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, fd, error);

    // Read
    char read_buffer[file_size];
    fd = gfal2_open(context, surl, O_RDONLY, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, fd, error);

    i = j = n = 0;
    while (i < file_size) {
        j = ((i + block_size) < file_size) ? block_size : file_size - i;

        ret = gfal2_read(context, fd, read_buffer + i, j, &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, fd, error);

        i += (long) ret;
    }

    ret = gfal2_close(context, fd, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, fd, error);

    // Compare
    EXPECT_EQ(0, memcmp(buffer, read_buffer, file_size));
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc != 4) {
        printf("Missing base url and/or sizes\n");
        printf("\t%s [options] srm://host/base/path/ <block size> <file size>\n", argv[0]);
        return 1;
    }

    RwSeqTest::root = argv[1];
    RwSeqTest::block_size = atol(argv[2]);
    RwSeqTest::file_size = atol(argv[3]);

    if (RwSeqTest::file_size / 2 < RwSeqTest::block_size) {
        fprintf(stderr, "block size should be less than half the file size\n");
        exit(1);
    }

//    gfal_set_verbose(GFAL_VERBOSE_TRACE | GFAL_VERBOSE_VERBOSE | GFAL_VERBOSE_DEBUG);

    return RUN_ALL_TESTS();
}
