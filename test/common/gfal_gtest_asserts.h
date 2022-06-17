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

#pragma once

#include <list>
#include <glib.h>
#include <gtest/gtest.h>


#define SKIP_TEST(NAME) do { std::cout << "[  SKIPPED ] " << #NAME << std::endl; } while (0);


testing::AssertionResult AssertGfalSuccess(
        const char* ret_expr, const char* error_expr,
        int ret, const GError* error
);


testing::AssertionResult AssertGfalErrno(
        const char* ret_expr, const char* error_expr, const char* errno_expr,
        int ret, const GError* error, int err
);


testing::AssertionResult AssertGfalOneOfErrno(
        const char* ret_expr, const char* error_expr, const char* errcodes_expr,
        int ret, const GError* error, std::list<int> errcodes
);
