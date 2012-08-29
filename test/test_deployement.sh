#!/bin/bash
#
#

set -e

echo "## start test deployement"

gfal_dir=$(dirname $0)/
gfal_dir=$(readlink -f "$gfal_dir/../")

echo "## gfal dir $gfal_dir "

echo "## create gfal2 test dir execution : "
gfal_test_dir=$(mktemp -td gfal2.XXXXXXXXXXXXXXXX)

echo "## gfal test dir $gfal_test_dir "
mkdir -p $gfal_test_dir/build
cd $gfal_test_dir/build
echo "## configure project ..."
cmake -D MAIN_TRANSFER=TRUE -D FUNCTIONAL_TESTS=TRUE -D UNIT_TESTS=TRUE -D CMAKE_INSTALL_PREFIX=/usr -D ONLY_TESTS=TRUE $gfal_dir

echo "## build tests..."
make


echo "## test execution .... "
source $gfal_dir/setup_test_env.sh

ctest
