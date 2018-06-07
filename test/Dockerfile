FROM gitlab-registry.cern.ch/dmc/gfal2:latest

RUN yum install -y cmake ca-policy-egi-core voms-clients voms-config-vo-dteam
RUN chmod a+rw /etc/gfal2.d/*

# We need to mount /etc/passwd so the current user id can be resolved from
# inside the container. And this is needed because of a bug in XRootD
# (See RQF0573851)
VOLUME [".ssh", ".globus", "/usr/share/gfal2/tests/Testing"]

ADD "gfal2-tests.sh" "/gfal2-tests.sh"
ENTRYPOINT ["/gfal2-tests.sh"]

