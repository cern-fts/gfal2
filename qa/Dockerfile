#
# Copyright (c) CERN 2016
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

FROM centos:7

# Required repos
ADD "https://dmc-repo.web.cern.ch/dmc-repo/dmc-devel-el7.repo" "/etc/yum.repos.d/"
ADD "http://repository.egi.eu/sw/production/cas/1/current/repo-files/EGI-trustanchors.repo" "/etc/yum.repos.d/"

RUN yum install -y epel-release yum-utils

# Build requirements
RUN yum install -y gcc gcc-c++
RUN yum-builddep -y gfal2

# Run dependencies
RUN yum install -y cppcheck java-1.8.0-openjdk lcg-CA lcov rats unzip voms-clients voms-config-vo-dteam which

# Coverage requirements
ADD "http://repo1.maven.org/maven2/org/codehaus/sonar/runner/sonar-runner-dist/2.4/sonar-runner-dist-2.4.zip" /tmp
RUN unzip "/tmp/sonar-runner-dist-2.4.zip" -d "/tmp"
ADD sonar-runner.properties "/tmp/sonar-runner-2.4/conf"
ADD "https://raw.github.com/eriwen/lcov-to-cobertura-xml/master/lcov_cobertura/lcov_cobertura.py" /tmp
RUN chmod a+r /tmp/lcov_cobertura.py

# Required input
VOLUME ["/gfal2", "/.ssh", "/.globus"]

# Entry point
COPY "qa.sh" "sonar-project.properties" /
ENTRYPOINT ["/qa.sh"]
