#!/usr/bin/env bash
set -e

VERSION=`rpm --eval %{centos_ver}`

cp -v ci/repo/dmc-devel-el${VERSION}.repo /etc/yum.repos.d/

TIMESTAMP=`date +%y%m%d%H%M`
GITREF=`git rev-parse --short HEAD`
BRANCH=`git name-rev $GITREF --name-only`
RELEASE=r${TIMESTAMP}git${GITREF}

if [[ $BRANCH == tags/* ]]; then
  RELEASE=
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
