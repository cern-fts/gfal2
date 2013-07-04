#!/bin/bash
## script to an auto configuration for the unit tests, with gfalFS
#

TEST_SRM_ENDPOINT="srm://cvitbdpm1.cern.ch/dpm/cern.ch/home/dteam/gfal2-tests/"
TEST_LFC_ENDPOINT="lfn:/grid/dteam/"

TEST_SRM_FILE_CONTENT="Hello world"

## srm vars
TEST_SRM_ONLY_READ_ACCESS="testread0011"
TEST_SRM_ONLY_READ_HELLO="testreadhello001"
TEST_SRM_NOEXIST_ACCESS=$TEST_SRM_INVALID_SURL_EXAMPLE2
TEST_SRM_NO_READ_ACCESS="testnoread0011"
TEST_SRM_WRITE_ACCESS="testwrite0011"
TEST_SRM_NO_WRITE_ACCESS="testnowrite0011"

TEST_SRM_CHMOD_FILE_EXIST="testchmod0011"
TEST_SRM_CHMOD_FILE_ENOENT="testchmodenoent0011"

TEST_SRM_MOD_READ_FILE="testchmodread0011"
TEST_SRM_MOD_WRITE_FILE="testchmodwrite0011"

TEST_GFAL_SRM_FILE_STAT_OK="teststat0011"

TEST_SRM_OPENDIR_OPEN="testopendir0011"


TEST_SRM_READDIR_VALID="testreaddir0011"
TEST_SRM_READDIR_1="testreaddir0012"
TEST_SRM_READDIR_2="testreaddir0013"
TEST_SRM_READDIR_3="testreaddir0014"
TEST_SRM_READDIR_4="testreaddir0015"

TEST_SRM_RENAME_VALID_DEST="testrename0012"
TEST_SRM_RENAME_VALID_SRC="testrename0011"

## lfc vars
TEST_LFC_ONLY_READ_HELLO="hello001"
TEST_LFC_ONLY_READ_ACCESS="testread0011"
TEST_LFC_NO_READ_ACCESS="testnoread0011"
TEST_LFC_WRITE_ACCESS="testwrite0011"	
TEST_LFC_NO_WRITE_ACCESS="testnowrite0011"

TEST_LFC_VALID_COMMENT="testcomment0011"
TEST_LFC_WRITEVALID_COMMENT="testcomment0012"
TEST_LFC_INVALID_COMMENT="testpsg0011"
TEST_LFC_COMMENT_CONTENT="Hello World"

TEST_GFAL_LFC_FILE_STAT_OK="teststat0011"
TEST_GFAL_LFC_LINK_STAT_OK="teststatlink0011"

TEST_LFC_OPEN_NOACCESS="testopen0011/testopen0012"
TEST_LFC_OPEN_NOACCESS_BASE="testopen0011"



TEST_LFC_MOD_READ_FILE="testchmodread0011"
TEST_LFC_MOD_WRITE_FILE="testchmodwrite0011"


TEST_LFC_OPENDIR_OPEN="testopendir0011"
TEST_LFC_OPENDIR_OPEN_NOACCESS="testopendir0012"

TEST_LFC_READDIR_VALID="testreaddir0011"
TEST_LFC_READDIR_1="testreaddir0012"
TEST_LFC_READDIR_2="testreaddir0013"
TEST_LFC_READDIR_3="testreaddir0014"
TEST_LFC_READDIR_4="testreaddir0015"

# write files 
function create_on_grid {
	echo "$1" > $2 
}

function create_directory {
	mkdir $1 
}

function create_directory_rec {
	mkdir -p $1 
}

function change_right {
	chmod $1 $2
}

function create_stat_basic {
	dd if=/dev/urandom of=/tmp/teststattmp bs=512 count=4;
	cp /tmp/teststattmp $1;
	chmod 664 $1;
}

function set_comment_on_file {
	setfattr -n "user.comment" -v "$1" $2
}

function create_symlink {
	ln -s $1 $2 
}

## CONFIGURE SRM ENV
#
mkdir -p /tmp/mount_point_gfal
gfalFS_umount  /tmp/mount_point_gfal
gfalFS  /tmp/mount_point_gfal $TEST_SRM_ENDPOINT
cd /tmp/mount_point_gfal

# main files for access
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_ONLY_READ_ACCESS
change_right 555 $TEST_SRM_WRITE_ACCESS
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_ONLY_READ_HELLO
# created but impossible to access
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_NO_READ_ACCESS
change_right 000 $TEST_SRM_NO_READ_ACCESS
# main file for write access
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_WRITE_ACCESS
change_right 777 $TEST_SRM_WRITE_ACCESS
# main file for no write access
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_NO_WRITE_ACCESS
change_right 555 $TEST_SRM_NO_WRITE_ACCESS
# main filefor chmod
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_CHMOD_FILE_EXIST

## main file for stats calls
create_stat_basic "$TEST_GFAL_SRM_FILE_STAT_OK"

## create comments files
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_LFC_VALID_COMMENT
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_LFC_WRITEVALID_COMMENT

## opendir / readdir
create_directory $TEST_SRM_OPENDIR_OPEN

create_directory $TEST_SRM_READDIR_VALID
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_READDIR_VALID/$TEST_SRM_READDIR_1
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_READDIR_VALID/$TEST_SRM_READDIR_2
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_READDIR_VALID/$TEST_SRM_READDIR_3
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_READDIR_VALID/$TEST_SRM_READDIR_4

## chmod
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_MOD_READ_FILE
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_MOD_WRITE_FILE


# rename
create_on_grid "$TEST_SRM_FILE_CONTENT" $TEST_SRM_RENAME_VALID_SRC


cd ~/
gfalFS_umount  /tmp/mount_point_gfal


### CONFIGURE LFC ENV
## FILES CAN NOT BE CREATED ON LFC CURRENTLY


mkdir -p /tmp/mount_point_gfal
gfalFS  /tmp/mount_point_gfal $TEST_LFC_ENDPOINT
cd /tmp/mount_point_gfal

## lfc access
change_right 555 $TEST_LFC_ONLY_READ_ACCESS
change_right 000 $TEST_LFC_NO_READ_ACCESS
change_right 777 $TEST_LFC_WRITE_ACCESS
change_right 444 $TEST_LFC_NO_WRITE_ACCESS

create_directory_rec $TEST_LFC_OPEN_NOACCESS
chmod 000 $TEST_LFC_OPEN_NOACCESS_BASE

# comments
set_comment_on_file "$TEST_LFC_COMMENT_CONTENT" $TEST_LFC_VALID_COMMENT

## stats

create_symlink $TEST_GFAL_LFC_FILE_STAT_OK $TEST_GFAL_LFC_LINK_STAT_OK

# mkdir
## opendir / readdir
create_directory $TEST_LFC_OPENDIR_OPEN
create_directory $TEST_LFC_OPENDIR_OPEN_NOACCESS
change_right 000 $TEST_LFC_OPENDIR_OPEN_NOACCESS


create_directory $TEST_LFC_READDIR_VALID

cd ~/
gfalFS_umount  /tmp/mount_point_gfal
