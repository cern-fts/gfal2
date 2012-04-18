/*
 * Authors: Zsolt Molnar <zsolt.molnar@cern.ch, http://www.zsoltmolnar.hu>
 *
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Inspired by MinUnit -- a minimal unit testing framework for C
 * (http://www.jera.com/techinfo/jtns/jtn002.html)
 */
#ifndef GFAL_UNIT_TESTSUITE_H
#define GFAL_UNIT_TESTSUITE_H

#include <stdio.h>
#include <stdlib.h>

/** Test the logical condition. If it is logical false, the test execution
 * stops, error message is displayed. */
#define GFAL_TEST_ASSERT(condition) \
    do { \
        printf("\t Test at %s:%d... ", __FILE__, __LINE__); \
        if (!(condition)) { \
            printf("FAILED\n"); \
            exit(-1); \
        } \
        printf("OK\n"); \
    } while (0)

/** Test equality of two values. The test expects value "expected", compares
 * with "value". In case of inequality, the test execution stops, error message
 * is displayed. */
#define GFAL_TEST_EQUAL(expected, value) \
    do { \
        printf("\t Test at %s:%d... ", __FILE__, __LINE__); \
        if ((expected) != (value)) { \
            printf("FAILED. Expected: %d, Got: %d\n", (expected), (value)); \
            exit(-1); \
        } \
        printf("OK\n"); \
    } while (0)

/** Test equality of two string values. The test expects value "expected",
 * compares with "value". In case of inequality, the test execution stops,
 * error message is displayed. */
#define GFAL_TEST_EQUAL_STRING(expected, value) \
    do { \
        printf("\t Test at %s:%d... ", __FILE__, __LINE__); \
        if (strcmp(expected, value) != 0) { \
            printf("FAILED. Expected: \"%s\", Got: \"%s\"\n", (expected), (value)); \
            exit(-1); \
        } \
        printf("OK\n"); \
    } while (0)

#define GFAL_TEST_RUN(test) \
    do { \
        printf("\nRunning: %s...\n", #test); \
        test(); \
        __gfal_tests_run++; \
       } while (0)

/** Counts the executed test */
extern int __gfal_tests_run;

#endif /* #ifndef GFAL_UNIT_TESTSUITE_H */
