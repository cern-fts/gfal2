/*
 * Copyright (c) CERN 2022
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
#include <utils/uri/gfal2_parsing.h>


TEST(ParsingTest, CollapseSlashes)
{
    const char* path = "//eos/opstest/dteam/test";
    char* collapsed = gfal2_path_collapse_slashes(path);

    ASSERT_STREQ("//eos/opstest/dteam/test", path);
    ASSERT_STREQ("/eos/opstest/dteam/test", collapsed);
    g_free(collapsed);

    path = "//eos/opstest////dteam/test//";
    collapsed = gfal2_path_collapse_slashes(path);

    ASSERT_STREQ("//eos/opstest////dteam/test//", path);
    ASSERT_STREQ("/eos/opstest/dteam/test/", collapsed);
    g_free(collapsed);

    collapsed = gfal2_path_collapse_slashes("path//not/starting/in/slash///");
    ASSERT_STREQ("path/not/starting/in/slash/", collapsed);
    g_free(collapsed);
}

TEST(ParsingTest, CollapseSlashesNullInput)
{
    char* collapsed = gfal2_path_collapse_slashes(NULL);
    ASSERT_EQ(NULL, collapsed);
}

TEST(ParsingTest, CollapseSlashesEmptyInput)
{
    char* collapsed = gfal2_path_collapse_slashes("");
    ASSERT_STREQ("", collapsed);
    g_free(collapsed);
}

TEST(ParsingTest, CollapseSlashesStringOutput) {
    std::string path = "//eos/opstest/dteam/test";
    char *collapsed = gfal2_path_collapse_slashes(path.c_str());
    path = collapsed;

    ASSERT_STREQ("/eos/opstest/dteam/test", path.c_str());
    ASSERT_STREQ(collapsed, path.c_str());
    g_free(collapsed);
}

TEST(ParsingTest, CollapseSlashesEmptyStringOutput) {
    char* collapsed = gfal2_path_collapse_slashes("");
    std::string path = collapsed;

    ASSERT_TRUE(path.empty());
    g_free(collapsed);

    std::string sempty;
    collapsed = gfal2_path_collapse_slashes(sempty.c_str());
    sempty = collapsed;

    ASSERT_TRUE(sempty.empty());
    g_free(collapsed);
}
