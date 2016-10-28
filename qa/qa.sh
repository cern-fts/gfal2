#!/usr/bin/bash
set -x

export HOME=/
export SONAR_USER_HOME=/tmp

VOMS=${VOMS:=dteam}
PASSWD=$1

if [[ -z "$PASSWD" ]]; then
    echo "Missing password"
    exit 1
fi

mkdir -p /tmp/build
pushd /tmp/build

# Build
CFLAGS=--coverage CXXFLAGS=--coverage cmake "/gfal2" \
    -DUNIT_TESTS=ON -DFUNCTIONAL_TESTS=ON -DPLUGIN_MOCK=ON
make -j

# Clear counters
lcov --directory . --zerocounters

# Run
echo ${PASSWD} | voms-proxy-init --pwstdin --voms ${VOMS}

export GFAL_PLUGIN_DIR="/tmp/build/plugins"
export GFAL_CONFIG_DIR="/gfal2/test/conf_test"
ctest -T test

# Extract coverage
lcov --directory . --capture --output-file="/tmp/coverage.info"
python /tmp/lcov_cobertura.py "/tmp/coverage.info" -b "/gfal2" -e ".+usr.include." -o "/tmp/coverage.xml"

# Run sonar
cp /sonar-project.properties .
export SONAR_RUNNER_OPTS="-Duser.timezone=+01:00 -Djava.security.egd=file:///dev/urandom"
/tmp/sonar-runner-2.4/bin/sonar-runner -X

popd

