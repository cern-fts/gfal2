/*
 * Copyright (c) CERN 2013-2017
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
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
#include <gfal_api.h>
#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>
#include <utils/exceptions/gerror_to_cpp.h>
#include <list>


class PosixTest: public testing::Test {
public:
    static const char* root;
    std::list<std::string> filesToClean;
    gfal2_context_t context;

    PosixTest() {
        GError *error = NULL;
        context = gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
    }

    ~PosixTest() {
        gfal2_context_free(context);
    }

    void SetUp() {
    }

    void TearDown() {
        typedef std::list<std::string>::reverse_iterator iterator;

        for (iterator i = filesToClean.rbegin(); i != filesToClean.rend(); ++i) {
            GError *error = NULL;
            gfal2_unlink(context, i->c_str(), &error);
            g_clear_error(&error);
            gfal2_rmdir(context, i->c_str(), &error);
            g_clear_error(&error);
        }
        filesToClean.clear();
    }

    std::string GenerateFile(std::string base = std::string()) {
        if (base.empty()) {
            base = root;
        }

        GError *error = NULL;
        char file[2048];
        generate_random_uri(base.c_str(), "test_posix", file, sizeof(file));
        int ret = generate_file_if_not_exists(context, file, "file:///etc/hosts", &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
        filesToClean.push_back(file);
        return file;
    }

    std::string GenerateDir() {
        GError *error = NULL;
        char dir[2048];
        generate_random_uri(root, "test_posix_dir", dir, sizeof(dir));
        int ret = gfal2_mkdir(context, dir, 0775, &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
        filesToClean.push_back(dir);
        return dir;
    }
};

const char *PosixTest::root;


TEST_F(PosixTest, Access)
{
    int ret = gfal_access(root, R_OK | W_OK | X_OK);
    ASSERT_EQ(0, ret);
}


TEST_F(PosixTest, ChMod)
{
    std::string file = GenerateFile();
    struct stat buf;

    int ret = gfal_chmod(file.c_str(), 0701);
    ASSERT_EQ(0, ret);

    ret = gfal_stat(file.c_str(), &buf);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(buf.st_mode & ~S_IFMT, 0701);
}


TEST_F(PosixTest, Rename)
{
    std::string file = GenerateFile();
    int ret = gfal_access(file.c_str(), F_OK);
    ASSERT_EQ(0, ret);

    std::string newName = file + ".renamed";
    filesToClean.push_back(newName);
    ret = gfal_rename(file.c_str(), newName.c_str());
    ASSERT_EQ(0, ret);

    ret = gfal_access(newName.c_str(), F_OK);
    ASSERT_EQ(0, ret);

    ret = gfal_access(file.c_str(), F_OK);
    ASSERT_NE(0, ret);
    ASSERT_EQ(errno, ENOENT);
}


TEST_F(PosixTest, Stat)
{
    std::string file = GenerateFile();
    struct stat buf;
    int ret = gfal_stat(file.c_str(), &buf);
    ASSERT_EQ(0, ret);
}


TEST_F(PosixTest, LStat)
{
    std::string file = GenerateFile();
    struct stat buf;
    int ret = gfal_lstat(file.c_str(), &buf);
    ASSERT_EQ(0, ret);
}


TEST_F(PosixTest, OpenDir)
{
    std::string dir = GenerateDir();
    std::string file = GenerateFile(dir);
    int fnameIndex = file.rfind('/');

    DIR* fd = gfal_opendir(dir.c_str());
    ASSERT_NE((void*)NULL, fd);

    bool found = false;
    struct dirent *ent = gfal_readdir(fd);
    while (ent) {
        if (file.substr(fnameIndex + 1).compare(ent->d_name) == 0) {
            found = true;
        }
        ent = gfal_readdir(fd);
    }
    ASSERT_TRUE(found);

    gfal_closedir(fd);
}


TEST_F(PosixTest, OpenFile)
{
    std::string file = GenerateFile();

    int fd = gfal_open(file.c_str(), O_RDONLY);
    ASSERT_GT(fd, 0);

    char buffer[512] = {0};
    ssize_t readSize = gfal_read(fd, buffer, sizeof(buffer));
    ASSERT_GT(readSize, 0);

    off_t offset = gfal_lseek(fd, 2, SEEK_SET);
    ASSERT_EQ(offset, 2);
    readSize = gfal_read(fd, buffer + 50, sizeof(buffer) - 50);
    ASSERT_GT(readSize, 0);

    ASSERT_EQ(buffer[2], buffer[50]);
    ASSERT_EQ(buffer[3], buffer[51]);
    ASSERT_EQ(buffer[4], buffer[52]);

    gfal_close(fd);
}


TEST_F(PosixTest, Creat)
{
    char file[2048];
    generate_random_uri(root, "test_posix", file, sizeof(file));
    filesToClean.push_back(file);
    int fd = gfal_creat(file, 0775);
    ASSERT_GT(fd, 0);

    char buffer[512] = {0};
    ssize_t readSize = gfal_write(fd, buffer, sizeof(buffer));
    ASSERT_EQ(readSize, sizeof(buffer));

    gfal_close(fd);
}


TEST_F(PosixTest, SymLink)
{
    if (strncmp(root, "file://", 7) != 0) {
        SKIP_TEST(SymLink);
        return;
    }

    std::string file = GenerateFile();
    std::string link = file + ".link";
    filesToClean.push_back(link);
    int ret = gfal_symlink(file.c_str(), link.c_str());
    ASSERT_EQ(0, ret);

    struct stat buf;
    ret = gfal_lstat(link.c_str(), &buf);
    ASSERT_EQ(0, ret);
    ASSERT_TRUE(S_ISLNK(buf.st_mode));

    ret = gfal_stat(link.c_str(), &buf);
    ASSERT_EQ(0, ret);
    ASSERT_FALSE(S_ISLNK(buf.st_mode));

    char buffer[1024] = {0};
    // Initialize to non-null value
    memset(buffer, 'x', sizeof(buffer));
    ret = gfal_readlink(link.c_str(), buffer, sizeof(buffer));
    ASSERT_GT(ret, 0);
    ASSERT_STREQ(file.substr(file.size() - ret).c_str(), buffer);
}


TEST_F(PosixTest, SymLinkTruncate)
{
    if (strncmp(root, "file://", 7) != 0) {
        SKIP_TEST(SymLink);
        return;
    }

    std::string file = GenerateFile();
    std::string link = file + ".link";
    filesToClean.push_back(link);

    int ret = gfal_symlink(file.c_str(), link.c_str());
    ASSERT_EQ(0, ret);

    size_t read_size = file.length() - 8;
    char buffer[1024];
    GError* error;

    // Initialize to non-null value
    memset(buffer, 'x', sizeof(buffer));

    // To capture the error object, we need to get the Posix thread handle
    gfal2_context_t handle = gfal_posix_get_handle();
    ASSERT_NE(handle, (void *) NULL);

    ret = gfal2_readlink(handle, link.c_str(), buffer, read_size, &error);
    ASSERT_EQ(ret, read_size);
    ASSERT_STRNE(file.substr(file.size() - ret).c_str(), buffer);

    // Last read character should match the original
    ASSERT_EQ(buffer[read_size - 1], file[read_size + 6]);
    // Character at read_size should be the 'x' padding
    ASSERT_EQ(buffer[read_size], 'x');
    // Readlink should return an error about possible truncation
    ASSERT_PRED_FORMAT3(AssertGfalErrno, -1, error, ENOMEM);
}


TEST_F(PosixTest, Xattr)
{
    std::string file = GenerateFile();

    char attrValue[] = "hello there";
    int ret = gfal_setxattr(file.c_str(), "user.attr", attrValue, sizeof(attrValue), 0);
    ASSERT_EQ(0, ret);

    char buffer[64] = {0};
    ret = gfal_getxattr(file.c_str(), "user.attr", buffer, sizeof(buffer));
    ASSERT_EQ(sizeof(attrValue), ret);
    ASSERT_STREQ(buffer, attrValue);

    ret = gfal_listxattr(file.c_str(), buffer, sizeof(buffer));
    ASSERT_GE(ret, 10);

    bool found = false;
    int i = 0;
    while (i < ret) {
        if (strncmp(buffer + i, "user.attr", 8) == 0) {
            found = true;
            break;
        }
        i += strlen(buffer + i) + 1;
    }
    EXPECT_TRUE(found);
}


TEST_F(PosixTest, PWrite)
{
    char file[2048];
    generate_random_uri(root, "test_posix", file, sizeof(file));
    filesToClean.push_back(file);
    int fd = gfal_creat(file, 0775);
    ASSERT_GT(fd, 0);

    char buffer[512] = {0};
    ssize_t write_size = gfal_pwrite(fd, buffer, sizeof(buffer), 0);
    ASSERT_EQ(write_size, sizeof(buffer));

    gfal_close(fd);

    char read_buffer[512] = {0};
    fd = gfal_open(file, O_RDONLY);

    ssize_t read_size = gfal_read(fd, read_buffer, sizeof(read_buffer));
    ASSERT_EQ(read_size, write_size);
    ASSERT_TRUE(memcmp(buffer, read_buffer, read_size) == 0);

    gfal_close(fd);
}


TEST_F(PosixTest, PRead)
{
    std::string file = GenerateFile();

    int fd = gfal_open(file.c_str(), O_RDONLY);
    ASSERT_GT(fd, 0);

    char buffer[512] = {0};
    ssize_t readSize = gfal_pread(fd, buffer, sizeof(buffer), 0);
    ASSERT_GT(readSize, 0);

    readSize = gfal_pread(fd, buffer + 50, sizeof(buffer) - 50, 2);
    ASSERT_GT(readSize, 0);

    ASSERT_EQ(buffer[2], buffer[50]);
    ASSERT_EQ(buffer[3], buffer[51]);
    ASSERT_EQ(buffer[4], buffer[52]);

    gfal_close(fd);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 2) {
        printf("Missing base url\n");
        printf("\t%s [options] srm://host/base/path/\n", argv[0]);
        return 1;
    }

    PosixTest::root = argv[1];

    return RUN_ALL_TESTS();
}
