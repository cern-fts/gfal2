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
ADD "https://grid-deployment.web.cern.ch/grid-deployment/dms/dmc/repos/dmc-ci-el7.repo" "/etc/yum.repos.d/"
ADD "http://repository.egi.eu/sw/production/cas/1/current/repo-files/EGI-trustanchors.repo" "/etc/yum.repos.d/"

# Epel and upgrade
RUN yum install -y epel-release
RUN yum clean all && yum update -y

# Build requirements
RUN yum install -y yum-utils
RUN yum groupinstall -y 'Development Tools'
RUN yum-builddep -y gfal2

# Run dependencies
RUN yum install -y voms-clients voms-config-vo-dteam lcg-CA

# Coverage requirements
RUN yum install -y java-1.8.0-openjdk lcov which
ADD "http://repo1.maven.org/maven2/org/codehaus/sonar/runner/sonar-runner-dist/2.4/sonar-runner-dist-2.4.zip" /tmp
RUN unzip "/tmp/sonar-runner-dist-2.4.zip" -d "/tmp"
ADD sonar-runner.properties "/tmp/sonar-runner-2.4/conf" 
ADD "https://raw.github.com/eriwen/lcov-to-cobertura-xml/master/lcov_cobertura/lcov_cobertura.py" /tmp

# Required input
VOLUME ["/gfal2", "/.ssh", "/.globus"]

# Entry point
ADD qa.sh /qa.sh
ADD sonar-project.properties /sonar-project.properties 
ENTRYPOINT ["/qa.sh"]

