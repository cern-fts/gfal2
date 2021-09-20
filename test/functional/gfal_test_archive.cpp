/*
 * Copyright (c) CERN 2020
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>

#include <gfal_api.h>
#include <uri/gfal2_uri.h>
#include <utils/exceptions/gerror_to_cpp.h>

#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>


class ArchiveTest: public testing::Test {

public:
    static const char* root;
    static const char* test_file;

    char surl[2048];
    gfal2_context_t handle;

    ArchiveTest() {
        GError* error = NULL;
        handle = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~ArchiveTest() {
        gfal2_context_free(handle);
    }

    virtual void SetUp() {
        setenv("XrdSecPROTOCOL", "gsi,unix", 1);
        uploadFile(surl, sizeof(surl));
    }

    virtual void TearDown() {
        clearFile(surl);
        unsetenv("XrdSecPROTOCOL");
    }

protected:
    void uploadFile(char* surl, size_t s_surl) {
        generate_random_uri(root, "archive", surl, s_surl);

        GError* error = NULL;
        int ret = generate_file_if_not_exists(handle, surl, test_file, &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
        g_clear_error(&error);
    }

    void clearFile(char* surl) {
        GError* error = NULL;
        int ret = gfal2_unlink(handle, surl, &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
        g_clear_error(&error);
    }
};

const char* ArchiveTest::root = NULL;
const char* ArchiveTest::test_file = "file:///etc/hosts";

// Poll a non-existing file
TEST_F(ArchiveTest, NoEntryPoll)
{
    GError* error = NULL;
    char inexistent[2048];

    generate_random_uri(root, "archive_enoent", inexistent, sizeof(inexistent));
    int ret = gfal2_archive_poll(handle, inexistent, &error);

    ASSERT_EQ(-1, ret);
    ASSERT_PRED_FORMAT3(AssertGfalErrno, -1, error, ENOENT);
}

// Poll invalid hostname
TEST_F(ArchiveTest, InvalidHostPoll)
{
    GError* error = NULL;
    char invalid_surl[2048];
    const char* format = "%s://invalid.%sfile.test";

    gfal2_uri* parsed = gfal2_parse_uri(root, &error);
    ASSERT_NE(parsed, (void *) NULL);

    if (root[strlen(root) - 1] != '/') {
        format = "%s://invalid.%s/file.test";
    }

    snprintf(invalid_surl, sizeof(invalid_surl), format,
             parsed->scheme, (root + strlen(parsed->scheme) + 3));

    g_clear_error(&error);
    gfal2_free_uri(parsed);

    int ret = gfal2_archive_poll(handle, invalid_surl, &error);

    ASSERT_EQ(-1, ret);
    ASSERT_PRED_FORMAT3(AssertGfalErrno, -1, error, ECOMM);
}

// Poll a single file for archiving
TEST_F(ArchiveTest, SingleFilePoll)
{
    GError* error = NULL;
    int interval = 1;
    int ret;

    do {
      printf ("Polling for archive: %s\n", surl);
      g_clear_error(&error);
      ret = gfal2_archive_poll(handle, surl, &error);

      if (error != NULL) {
          ASSERT_EQ(EAGAIN, error->code);

          if (error->code == EAGAIN) {
              g_clear_error(&error);
          }
      }

      printf("Waiting %ds before new archive poll...\n", interval);
      fflush(stdout);
      sleep(interval);
      interval <<= 1;

      if (interval > 300) {
        interval = 300;
      }

    } while (ret == 0);

    ASSERT_EQ(1, ret);
    ASSERT_PRED_FORMAT2(AssertGfalSuccess, 1, error);
}

// Poll a file list where some entries exist in the namespace
TEST_F(ArchiveTest, ListPoll)
{
    char surl_second[2048];
    char inexistent[2048];
    int interval = 1;
    int ret;

    uploadFile(surl_second, sizeof(surl_second));
    generate_random_uri(root, "archive_enoent", inexistent, sizeof(inexistent));

    char* surls[] = {
        inexistent,
        surl,
        surl_second
    };
    GError* errors[] = {
        NULL,
        NULL,
        NULL
    };

    memset(errors, 0x00, sizeof(errors));
    int nbfiles = sizeof(surls) / sizeof(char *);

    do {
        printf ("Polling for archive:\n");
        for (int i = 0 ; i < nbfiles; i++) {
            printf("\t%s\n", surls[i]);
            g_clear_error(&errors[i]);
        }

        ret = gfal2_archive_poll_list(handle, nbfiles, surls, errors);

        // Check that the first file does not exist
        ASSERT_PRED_FORMAT3(AssertGfalErrno, -1, errors[0], ENOENT);

        // Check that remaining files exist
        for (int i = 1; i < nbfiles; i++) {
            if (errors[1] != NULL) {
                ASSERT_EQ(EAGAIN, errors[i]->code);
                g_clear_error(&errors[i]);
            }
        }

        printf("Waiting %ds before new archive poll...\n", interval);
        fflush(stdout);
        sleep(interval);
        interval <<= 1;

        if (interval > 300) {
          interval = 300;
        }
    } while (ret == 0);

    ASSERT_EQ(2, ret);
    ASSERT_PRED_FORMAT3(AssertGfalErrno, -1, errors[0], ENOENT);
    for (int i = 1; i < nbfiles; i++) {
        ASSERT_PRED_FORMAT2(AssertGfalSuccess, 1, errors[i]);
    }
    clearFile(surl_second);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing parameters:\n");
        printf("\t%s [options] root://<host>:1094//<test_directory_path> [test_file]\n", argv[0]);
        return 1;
    }

    ArchiveTest::root = argv[1];

    if (argc == 3) {
        char* s_test_file = argv[2];

        if (strncmp(argv[2], "file://", 7) != 0) {
            s_test_file = new char[sizeof(argv[2]) + 8];
            strcpy(s_test_file, "file://");
            strcat(s_test_file, argv[2]);
        }
        ArchiveTest::test_file = s_test_file;
    }

    return RUN_ALL_TESTS();
}
