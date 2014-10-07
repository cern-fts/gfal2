#! /bin/bash
# Copyright (c) Members of the EGEE Collaboration. 2004.
# See http://www.eu-egee.org/partners/ for details on the copyright holders.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Global settings for all the tests. 

# Exit in case of uninitialized variables
set -o nounset

# Exit the script in case of non-true return value
set -o errexit

function usage 
{
    echo
    echo "Usage:"
    echo
    echo "Prerequistes:"
    echo 
    echo " - BUILD_ROOT must be set to to the root of the build/checkout of the gfal2 project. Example:"
    echo
    echo " export BUILD_ROOT=/home/$USER/workspace"
    echo
}  

function checkVariable
{
    local VAR_NAME=$1
    local VAR_DEFAULT=$2
    local VAR_VALUE=$3

    # color codes, color howto: http://webhome.csc.uvic.ca/~sae/seng265/fall04/tips/s265s047-tips/bash-using-colors.html

    if [ ! -n "$VAR_VALUE" ] ;then
        echo
        echo -e "\e[1;31mERROR: $VAR_NAME is not set.\e[0m For example:"
        echo
        echo "export $VAR_NAME=$VAR_DEFAULT"
        echo
        usage
        exit 1
    fi
}

function execute_command {
    local line_number=$1
    local expected_return_code=$2
    local pattern_in_output=$3
    local command=$4

    set +o nounset
    local is_display=$5
    set -o nounset


    echo -e "\nExecuting command from line $line_number:\n$command\n\n"
    set +e
    OUTPUT=$(eval $command 2>&1)
    local return_code=$?
    
    if [ $return_code != $expected_return_code ]; then 
        echo -e "Command failed at line $line_number (return code: $return_code, expected: $expected_return_code)"
        echo -e "\nCommand output:\n$OUTPUT\n"
        exit 1
    fi
    
    match=`echo $OUTPUT | grep -c "$pattern_in_output"`
    
    if [ $match == 0 ]; then
        echo -e "Expected pattern \"$pattern_in_output\" cannot be found in the output.\n"
        echo -e "Command output:\n$OUTPUT\n"
        exit 1
    fi

    if [ -n "$is_display" ] ; then
         echo -e "Command output:\n$OUTPUT\n"
    fi

    set -e
}


checkVariable BUILD_ROOT "/home/user/workspace" $BUILD_ROOT
source $BUILD_ROOT/org.glite.data.project/bin/test-setup-gfal.sh

