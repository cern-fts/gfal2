#!/bin/bash
# Build source for coverage tests

if [ "$#" -eq 1 ]; then
    BUILD_DIR=`readlink -f $1`
else
    BUILD_DIR=~+/build
fi

SOURCE_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
RUNNING_DIR=`pwd`

set -x

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}"

# Build
CFLAGS=--coverage CXXFLAGS=--coverage cmake "${SOURCE_DIR}" \
    -DUNIT_TESTS=ON -DFUNCTIONAL_TESTS=ON
make -j2

# Run
export GFAL_PLUGIN_DIR="${BUILD_DIR}/plugins"
export GFAL_CONFIG_DIR="${SOURCE_DIR}/test/conf_test"
ctest -T test

# Extract coverage
lcov --directory . --capture --output-file="${RUNNING_DIR}/coverage.info"

if [ ! -f "/tmp/lcov_cobertura.py" ]; then
    wget "https://raw.github.com/eriwen/lcov-to-cobertura-xml/master/lcov_cobertura/lcov_cobertura.py" -O "/tmp/lcov_cobertura.py"
fi

python /tmp/lcov_cobertura.py "${RUNNING_DIR}/coverage.info" -b "${SOURCE_DIR}" -e ".+usr.include." -o "${RUNNING_DIR}/coverage.xml"

# Done
popd
echo "Done extracting coverage information"
