#pragma once

#include <glib.h>
#include <gtest/gtest.h>

testing::AssertionResult AssertGfalSuccess(
        const char* ret_expr, const char* error_expr,
        int ret, const GError* error
);

testing::AssertionResult AssertGfalErrno(
        const char* ret_expr, const char* error_expr, const char* errno_expr,
        int ret, const GError* error, int err
);
