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
