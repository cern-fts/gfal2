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

#include <algorithm>
#include "gfal_gtest_asserts.h"


testing::AssertionResult AssertGfalSuccess(
        const char* ret_expr, const char* error_expr,
        int ret, const GError* error)
{
    if (ret >= 0 && error == NULL)
        return testing::AssertionSuccess();

    if (ret >= 0 && error != NULL)
        return testing::AssertionFailure()
            << ret_expr << " >= 0, but " << error_expr << " is not NULL: ("
            << error->code << ") " << error->message;

    if (ret < 0 && error == NULL)
        return testing::AssertionFailure()
            << ret_expr << " < 0, but " << error_expr << " is NULL";

    return testing::AssertionFailure()
        << error_expr << " is not NULL: (" << error->code << ") " << error->message;
}


testing::AssertionResult AssertGfalErrno(
        const char* ret_expr, const char* error_expr, const char* errno_expr,
        int ret, const GError* error, int err)
{
    if (ret >= 0 && error == NULL)
        return testing::AssertionFailure()
            << "Operation succeeded, but errno " << err << " was expected";

    if (ret >= 0 && error != NULL)
        return testing::AssertionFailure()
            << "Return status is >= 0, but error has been set: ("
            << error->code << ") " << error->message
            << " (Was expecting errno " << err << ")";

    if (ret < 0 && error == NULL)
        return testing::AssertionFailure()
            << "Return status is < 0, but error has not been set "
            << "(was expecting errno " << err << ")";

    if (error->code != err)
        return testing::AssertionFailure()
            << "Expecting errno " << err << " but got " << error->code
            << " (" << error->message << ")";

    return testing::AssertionSuccess();
}


// Same as AssertGfalError, but accepts a list of possible error codes
// Accommodate plugins returning different error codes for the same function call (ideally, this is to be avoided)
testing::AssertionResult AssertGfalOneOfErrno(
        const char* ret_expr, const char* error_expr, const char* errno_expr,
        int ret, const GError* error, std::list<int> errcodes)
{
    for (auto it = errcodes.begin(); it != errcodes.end(); it++) {
        auto res = AssertGfalErrno(ret_expr, error_expr, errno_expr, ret, error, *it);

        if (res == testing::AssertionSuccess()) {
            return res;
        }
    }

    std::ostringstream oss;
    std::copy(errcodes.begin(), errcodes.end(), std::ostream_iterator<int>(oss, ", "));

    return testing::AssertionFailure()
        << "Expecting one of following errno [" << oss.str().substr(0, oss.str().size() - 2)
        << "] but got " << error->code << " (" << error->message << ")";
}
