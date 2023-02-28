#
# Copyright (c) CERN 2023
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# This code sets the following variables:
#
# CRYPTOPP_LIBRARIES       = full path to the crytopp libraries
# CRYPTOPP_INCLUDE_DIRS    = include dir to be used when using the crytopp library
# CRYPTOPP_FOUND           = set to true if crytopp was found successfully


find_path(CRYPTOPP_INCLUDE_DIRS
        base64.h
        PATHS /usr/include/cryptopp
        NO_DEFAULT_PATH)

find_library(CRYPTOPP_LIBRARIES
        NAME cryptopp
        PATHS /usr/lib64
        NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(cryptopp DEFAULT_MSG
        CRYPTOPP_INCLUDE_DIRS CRYPTOPP_LIBRARIES)
