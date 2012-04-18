#!/bin/bash
## help tool for generic packaging
# @author: Devresse Adrien

## vars
BASE_TMP=/tmp
FINAL_DIR="RPMS/"

TARBALL_FILTER="--exclude='.git' --exclude='.svn' --exclude='RPMS' --exclude='*.rpm'"
FOO="HELLO"

set -e


create_tmp_dir() {
	export TMP_DIR="$BASE_TMP/$RANDOM$RANDOM$RANDOM_$$"
	mkdir -p $TMP_DIR
}

# var, value, specfile, try to do a recursive resolution of a spec attribute
do_rec_resolution(){
	RES_VAR=""
	local TMP_VAR="$(echo \"$2\" | grep % | sed -e 's@.*%{\(.*\)}.*@\1@g' )"
	if [[ "$TMP_VAR" != "" ]]; then
		local TMP_VAR2="$( grep "%define $TMP_VAR" $3 | sed "s@.*%define $TMP_VAR \(.*\)@\1@g" )"
		if [[ "x$TMP_VAR2" == "x" ]]; then
			local TMP_VAR2="$( grep -i "$TMP_VAR" $3 | sed "s@$TMP_VAR\:[ \t]*\(.*\)@\1@I" ) "	
		fi
		do_rec_resolution $1 $TMP_VAR2 $3
		export RES_VAR="$(echo $2 | sed "s@%{$TMP_VAR}@$RES_VAR@g" )"
		do_rec_resolution $1 $RES_VAR $3
	else
		export RES_VAR=$2
	fi
	export $1="$RES_VAR"
}

get_attrs_spec(){
	export PKG_VERSION="$( grep "Version:" $1 | sed 's@Version:\(.*\)@\1@g' )"
	export PKG_NAME="$( grep "Name:" $1 | sed 's@Name:\(.*\)@\1@g' )"
	do_rec_resolution "PKG_VERSION" $PKG_VERSION $1
	do_rec_resolution "PKG_NAME" $PKG_NAME $1
	export SRC_NAME="$PKG_NAME-$PKG_VERSION.tar.gz"
	echo "res : $SRC_NAME $PKG_VERSION $PKG_NAME"
}

# src_dir, tarbal_filepath
create_tarball(){
	create_tmp_dir
	SRC_FOLDER="/$TMP_DIR/$PKG_NAME-$PKG_VERSION"
	mkdir -p "$SRC_FOLDER"
	echo "copy files..."
	cp -r $1/* $SRC_FOLDER/
	CURRENT_DIR=$PWD
	cd $TMP_DIR
	echo "copy files..."
	eval "tar -cvzf $2 $TARBALL_FILTER $PKG_NAME-$PKG_VERSION"
	echo "tarball result : $2 $TARBALL_FILTER "
	cd $CURRENT_DIR
	rm -rf $TMP_DIR
}

# specfiles_dir 
create_rpmbuild_env(){
	create_tmp_dir
	export RPM_BUILD_DIR="$TMP_DIR"	
	mkdir -p $RPM_BUILD_DIR/RPMS $RPM_BUILD_DIR/SOURCES $RPM_BUILD_DIR/BUILD $RPM_BUILD_DIR/SRPMS $RPM_BUILD_DIR/SPECS $RPM_BUILD_DIR/tmp
	cp $1/* $RPM_BUILD_DIR/SPECS/
	
}

# specfiles_dir 
delete_rpmbuild_env(){
	rm -rf $RPM_BUILD_DIR/
	
}


# specfile
rpm_build_src_package(){
	echo "Begin the rpmbuild source call for spec file $1 ...."
	echo "%_topdir $RPM_BUILD_DIR" >> ~/.rpmmacro
	local OLD_DIR=$PWD
	local MACRO_TOPDIR="s  \"_topdir $RPM_BUILD_DIR\""
	cd $RPM_BUILD_DIR
	ls $PWD/SOURCES/
	rpmbuild -bs --nodeps --define "_topdir $RPM_BUILD_DIR" SPECS/$1
	cd $OLD_DIR
	echo "End the rpmbuild source call...."	
}



## main
if [[ "$1" == "" || "$2" == "" ]]; then
	echo "Usage $0 [spec_dir] [src_dir] "
	exit -1
fi

create_rpmbuild_env $1
mkdir -p SRPMS

# list spec file
for i in $1/*.spec 
do
	echo " create RPMS for spec file : $i"
	get_attrs_spec $i
	echo "Source : $SRC_NAME"
	echo "Version : $PKG_VERSION"
	echo "Name: $PKG_NAME"	
	echo "create source tarball..."
	create_tarball $2 "$RPM_BUILD_DIR/SOURCES/$SRC_NAME"
	echo "TARBALL: $RPM_BUILD_DIR/SOURCES/$SRC_NAME"
	rpm_build_src_package `basename $i` 
done
mkdir -p  $FINAL_DIR
cp $RPM_BUILD_DIR/SRPMS/* $FINAL_DIR
## clean everything
delete_rpmbuild_env
	
