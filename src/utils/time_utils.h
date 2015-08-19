/*
 * Copyright (c) CERN 2013-2015
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
#ifndef _TIME_UTILS_H
#define _TIME_UTILS_H

/**
  similar to timeradd, timercmp, etc... function but for timerspec
*/


#define timespec_cmp(a, b, CMP)                                              \
  (((a)->tv_sec == (b)->tv_sec) ?                                             \
   ((a)->tv_nsec CMP (b)->tv_nsec) :                                          \
   ((a)->tv_sec CMP (b)->tv_sec))



#define timespec_add(a, b, result)                                           \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;                             \
    (result)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec;                          \
    if ((result)->tv_nsec > 1000000000) {                                     \
      ++(result)->tv_sec;                                                     \
      (result)->tv_nsec -= 1000000000;                                        \
    }                                                                         \
  } while (0)


#define timespec_sub(a, b, result)                                           \
  do {                                                                        \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                             \
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;                          \
    if ((result)->tv_nsec < 0) {                                     \
      --(result)->tv_sec;                                                     \
      (result)->tv_nsec += 1000000000;                                        \
    }                                                                         \
  } while (0)


#define timespec_clear(a)                                                    \
  do {                                                                        \
    (a)->tv_sec = 0;                                                          \
    (a)->tv_nsec = 0;                                                         \
  } while (0)




#define timespec_isset(a)                                                    \
  ( !((a)->tv_sec == 0 && (a)->tv_nsec == 0) )


#define timespec_copy(a,b)                                                    \
    do {                                                                      \
    (a)->tv_sec =  (b)->tv_sec;                                              \
    (a)->tv_nsec =  (b)->tv_nsec;                                            \
    } while (0)

#endif
