#!/bin/bash
# Build source for coverage tests
if [ "$#" -ne 2 ]; then
    echo "Wrong number of arguments, required source and build dir"
    exit 1;
fi

SOURCE_DIR=`readlink -f $1`
BUILD_DIR=`readlink -f $2`

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
lcov --directory . --capture --output-file="${BUILD_DIR}/coverage.info"

if [ ! -f "/tmp/lcov_cobertura.py" ]; then
    wget "https://raw.github.com/eriwen/lcov-to-cobertura-xml/master/lcov_cobertura/lcov_cobertura.py" -O "/tmp/lcov_cobertura.py"
fi

python /tmp/lcov_cobertura.py "${BUILD_DIR}/coverage.info" -b "${SOURCE_DIR}" -e ".+usr.include." -o "${BUILD_DIR}/coverage.xml"

# Done
popd
echo "Done extracting coverage information"
