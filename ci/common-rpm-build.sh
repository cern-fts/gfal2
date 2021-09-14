#!/usr/bin/env bash
set -e

TIMESTAMP=`date +%y%m%d%H%M`
GITREF=`git rev-parse --short HEAD`
BRANCH=`git name-rev $GITREF --name-only`
RELEASE=r${TIMESTAMP}git${GITREF}

BUILD="devel"
DIST=$(rpm --eval "%{dist}" | cut -d. -f2)

# Special handling of FC rawhide
[[ "${DIST}" == "fc36" ]] && DIST="fc-rawhide"

if [[ $BRANCH == tags/* ]]; then
  RELEASE=
  BUILD="rc"
fi

REPO_FILE="dmc-${BUILD}-${DIST}.repo"

if [[ -f "ci/repo/${REPO_FILE}" ]]; then
  cp -v "ci/repo/${REPO_FILE}" "/etc/yum.repos.d/"
fi

RPMBUILD=${PWD}/build
SRPMS=${RPMBUILD}/SRPMS

cd packaging/
make srpm RELEASE=${RELEASE} RPMBUILD=${RPMBUILD} SRPMS=${SRPMS}

if [[ -f /usr/bin/dnf ]]; then
  dnf install -y epel-release || true
  dnf builddep -y ${SRPMS}/*
else
  yum-builddep -y ${SRPMS}/*
fi

rpmbuild --rebuild --define="_topdir ${RPMBUILD}" ${SRPMS}/*
