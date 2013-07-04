#pragma once
/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// set to 1 if mock need to be used else to 0
#ifndef USE_MOCK
#define USE_MOCK 0
#endif

// srm://grid05.lal.in2p3.fr:8446/dpm/lal.in2p3.fr/home/dteam/
// srm://grid05.lal.in2p3.fr:8446/srm/managerv2?SFN=/dpm/lal.in2p3.fr/home/dteam

// to define
#define TEST_SRM_DPM_ENDPOINT_PREFIX "srm://cvitbdpm1.cern.ch/dpm/cern.ch/home/dteam/gfal2-tests/"
#define TEST_SRM_DPM_FULLENDPOINT_PREFIX "srm://cvitbdpm1.cern.ch:8446/srm/managerv2?SFN=/dpm/cern.ch/home/dteam/gfal2-tests/" 
#define TEST_SRM_DPM_FULLENDPOINT_URL "httpg://cvitbdpm1.cern.ch:8446/srm/managerv2"
#define TEST_SRM_DPM_CORE_URL "cvitbdpm1.cern.ch:8446"
#define TEST_SRM_FILE_CONTENT "Hello world\n"
#define TEST_LFC_PREFIX "lfn:/grid/dteam"

#define TEST_GUID_VALID_FILE "guid:a5bf667d-dd72-45b9-9b8f-fc69b999f653"
// macros

#define TEST_FUSION_STRING(x,y) x y
#define TEST_XFUSION_STRING(x,y) TEST_FUSION_STRING(x,y)

#define TEST_CONCAT_SRM_PREFIX(x) TEST_XFUSION_STRING(TEST_SRM_DPM_ENDPOINT_PREFIX, x) 

#define TEST_CREATE_SRM_FILE(x) TEST_XFUSION_STRING(TEST_XFUSION_STRING("lcg-cp file:///etc/group ",x)," &> /dev/null")

#define TEST_CONCAT_LFC_PREFIX(x) TEST_XFUSION_STRING(TEST_LFC_PREFIX, x)

// mds part
#define TEST_MDS_VALID_ENDPOINT_URL "cvitbdpm1.cern.ch"
#define TEST_MDS_INVALID_ENDPOINT_URL "google.com"
#define TEST_MDS_VALID_ENDPOINT_RESU_1 "httpg://cvitbdpm1.cern.ch:8443/srm/managerv1"
#define TEST_MDS_VALID_ENDPOINT_RESU_2 "httpg://cvitbdpm1.cern.ch:8446/srm/managerv2"
 
 // constant to define for succefull unit test
#define TEST_LFC_VALID_ACCESS "lfn:/grid/dteam/hello001"		// this file must be a lfc file with read access and no write access
#define TEST_LFC_ONLY_READ_ACCESS "lfn:/grid/dteam/testread0011"
#define TEST_LFC_NOEXIST_ACCESS "lfn:/grid/dteam/PSGmarqueUNbut" // this file must never exist.
#define TEST_LFC_NO_READ_ACCESS "lfn:/grid/dteam/testnoread0011" // this file is un-accessible be read()
#define TEST_LFC_WRITE_ACCESS "lfn:/grid/dteam/testwrite0011"	// must be writed
#define TEST_LFC_NO_WRITE_ACCESS "lfn:/grid/dteam/testnowrite0011"
#define TEST_LFC_URL_SYNTAX_ERROR "jardiland.com"

// GUID constants
#define TEST_GUID_VALID_ACCESS "guid:4ff6feed-a6f4-4266-b783-9aff66ff54dc"
#define TEST_GUID_ONLY_READ_ACCESS "guid:4ff6feed-a6f4-4266-b783-9aff66ff54dc"
#define TEST_GUID_WRITE_ACCESS "guid:0e8d1f2b-6e11-4777-b6f4-3e3a41af9403"
#define TEST_GUID_NO_READ_ACCESS "guid:bf83c34a-97d4-4fc4-b2a0-439178efe568"
#define TEST_GUID_NO_WRITE_ACCESS "guid:98ec152b-9c2f-469e-9b9d-0eecda381f55"
#define TEST_GUID_NOEXIST_ACCESS "guid:ae595782-81be-40af-ac3c-8b1a084bfc82"

// srm part constant
#define TEST_SRM_VALID_SURL_EXAMPLE1  TEST_CONCAT_SRM_PREFIX("file75715ccc-1c54-4d18-8824-bdd3716a2b54") 
#define TEST_SRM_INVALID_SURL_EXAMPLE2 TEST_CONCAT_SRM_PREFIX("generated/2006-07-04/test2") 
// access
#define TEST_SRM_ONLY_READ_ACCESS TEST_CONCAT_SRM_PREFIX("testread0011")
#define TEST_SRM_ONLY_READ_HELLO TEST_CONCAT_SRM_PREFIX("testreadhello001")
#define TEST_SRM_NOEXIST_ACCESS TEST_SRM_INVALID_SURL_EXAMPLE2
#define TEST_SRM_NO_READ_ACCESS TEST_CONCAT_SRM_PREFIX("testnoread0011")
#define TEST_SRM_WRITE_ACCESS TEST_CONCAT_SRM_PREFIX("testwrite0011")
#define TEST_SRM_NO_WRITE_ACCESS TEST_CONCAT_SRM_PREFIX("testnowrite0011")

#define TEST_SRM_TURL_EXAMPLE1 "rfio://cvitbdpm1.cern.ch//storage/dteam/2011-10-26/file75715ccc-1c54-4d18-8824-bdd3716a2b54.43869.0"
#define TEST_SRM_IFCE_INVALID_PATH_REP "[SE][StatusOfPutRequest][SRM_INVALID_PATH] <none>"
// rfio
#define TEST_SRM_RFIO_EXAMPLE1 TEST_SRM_TURL_EXAMPLE1

// chmod 
#define TEST_LFC_MOD_READ_FILE "lfn:/grid/dteam/testchmodread0011"
#define TEST_LFC_MOD_UNEXIST_FILE "lfn:/grid/dteam/OpenSourceAndMicrosoft"
#define TEST_LFC_MOD_WRITE_FILE "lfn:/grid/dteam/testchmodwrite0011"

#define TEST_GUID_MODE_READ_FILE "guid:6a26c234-f241-44d8-b5f7-889c742d4378"

#define TEST_LFC_RENAME_VALID_DEST "lfn:/grid/dteam/testrename0012"
#define TEST_LFC_RENAME_VALID_SRC "lfn:/grid/dteam/testrename0011"

#define TEST_LFC_MOVABLE_DIR_SRC "lfn:/grid/dteam/testmovedir0011/"
#define TEST_LFC_MOVABLE_DIR_DEST "lfn:/grid/dteam/testmovedir0012/"

#define TEST_SRM_CHMOD_FILE_EXIST TEST_CONCAT_SRM_PREFIX("testchmod0011")
#define TEST_SRM_CHMOD_FILE_ENOENT TEST_CONCAT_SRM_PREFIX("testchmodenoent0011")

// rename
#define TEST_GFAL_LOCAL_FILE_RENAME_SRC "/tmp/testrename0011"
#define TEST_GFAL_LOCAL_FILE_RENAME_DEST "/tmp/testrename0012"


// stat
#define TEST_GFAL_LFC_FILE_STAT_OK "lfn:/grid/dteam/teststat0011"
#define TEST_GFAL_LFC_LINK_STAT_OK  "lfn:/grid/dteam/teststatlink0011"
#define TEST_GFAL_LFC_FILE_STAT_NONEXIST TEST_LFC_NOEXIST_ACCESS


#define TEST_GFAL_SRM_FILE_STAT_OK TEST_CONCAT_SRM_PREFIX("teststat0011")
#define TEST_GFAL_SRM_FILE_STAT_NONEXIST TEST_SRM_NOEXIST_ACCESS
#define TEST_GFAL_SRM_FILE_STAT_SIZE_VALUE 2048
#define TEST_GFAL_SRM_FILE_STAT_MODE_VALUE 0100664
#define TEST_GFAL_SRM_FILE_STAT_UID_VALUE 2
#define TEST_GFAL_SRM_FILE_STAT_GID_VALUE 2


#define TEST_GFAL_GUID_FILE_STAT_OK TEST_GUID_VALID_FILE 

#define TEST_GFAL_LFC_FILE_STAT_SIZE_VALUE 2048
#define TEST_GFAL_LFC_FILE_STAT_MODE_VALUE 0100664
#define TEST_GFAL_LFC_FILE_STAT_UID_VALUE 105
#define TEST_GFAL_LFC_FILE_STAT_GID_VALUE 105


#define TEST_GFAL_LFC_LINK_STAT_SIZE_VALUE 0
#define TEST_GFAL_LFC_LINK_STAT_MODE_VALUE 0120777
#define TEST_GFAL_LFC_LINK_STAT_UID_VALUE 105
#define TEST_GFAL_LFC_LINK_STAT_GID_VALUE 105

#define TEST_GFAL_LOCAL_STAT_COMMAND "dd if=/dev/zero of=/tmp/teststat0011 bs=512 count=4 &> /dev/null && chmod 0644 /tmp/teststat0011"
#define TEST_GFAL_LOCAL_LINK_COMMAND "ln -s /tmp/teststat0011 /tmp/teststatlink0011 &> /dev/null"
#define TEST_GFAL_LOCAL_STAT_OK "file:///tmp/teststat0011"
#define TEST_GFAL_LOCAL_LINK_OK "file:///tmp/teststatlink0011"
#define TEST_GFAL_LOCAL_STAT_NONEXIST "file:///tmp/ChristmasTree"

#define TEST_GFAL_LOCAL_FILE_STAT_SIZE_VALUE 2048
#define TEST_GFAL_LOCAL_FILE_STAT_MODE_VALUE 0100644


// mkdir

#define TEST_LFC_EEXIST_MKDIR "lfn:/grid/dteam/testmkdir0011"
#define TEST_LFC_BASE_FOLDER_URL_MKDIR1 "lfn:/grid/dteam/testmkdir0011/"
#define TEST_LFC_UNACCESS_MKDIR "lfn:/grid/dteam/testmkdirunaccess0011/testunacess0011"


#define TEST_LOCAL_MKDIR_EXIST_COMMAND "mkdir -p /tmp/testmkdir0011 &> /dev/null && mkdir -p -m 000 /tmp/testeaccesdir"
#define TEST_LOCAL_MKDIR_EXIST_FILE "file:///tmp/testmkdir0011"

#define TEST_LOCAL_BASE_FOLDER_URL_MKDIR1 "file:///tmp/"
#define TEST_LOCAL_UNACCESS_MKDIR "file:///tmp/testeaccesdir/santaclaus"

#define TEST_SRM_EEXIST_MKDIR TEST_CONCAT_SRM_PREFIX("testmkdir0011")
#define TEST_SRM_BASE_FOLDER_URL_MKDIR1 TEST_CONCAT_SRM_PREFIX("/testmkdirfolder/")
#define TEST_SRM_UNACCESS_MKDIR TEST_CONCAT_SRM_PREFIX("testunaccessmkdir0011/testunaccessmkdir0012")


// rmdir 

#define TEST_LFC_RMDIR_CREATED "lfn:/grid/dteam/testrmdir0011" // must be an unexistant dir, will be created and deleted, without slash
#define TEST_LFC_RMDIR_EEXIST "lfn:/grid/dteam/testrmdir0012" // must be a dir with content inside
#define TEST_LFC_RMDIR_CREATED_SLASH "lfn:/grid/dteam/testrmdir0011/" // must be an unexisting dir, will be created and deleted with slash


#define TEST_SRM_RMDIR_CREATED TEST_CONCAT_SRM_PREFIX("testrmdir0011") // same thing for srm
#define TEST_SRM_RMDIR_EEXIST TEST_CONCAT_SRM_PREFIX("testrmdir0012")
#define TEST_SRM_RMDIR_CREATED_SLASH TEST_CONCAT_SRM_PREFIX("testrmdir0011")

#define TEST_LOCAL_RMDIR_CREATED "file:///tmp/testrmdir0011"
#define TEST_LOCAL_RMDIR_EEXIST "file:///tmp/testrmdir0012"				// do not need to be created, created by system call
#define TEST_LOCAL_RMDIR_EEXIST_COMMAND "mkdir -p /tmp/testrmdir0012 && touch /tmp/testrmdir0012/testrmdir0013"
#define TEST_LOCAL_RMDIR_CREATED_SLASH "file:///tmp/testrmdir0011"


// opendir
#define TEST_LOCAL_OPENDIR_COMMAND "mkdir -m 000 -p /tmp/testopendireaccess"
#define TEST_LOCAL_OPENDIR_OPEN_INVALID "file:///windowsdoesnotcrash"
#define TEST_LOCAL_OPENDIR_OPEN_NOACCESS "file:///tmp/testopendireaccess"
#define TEST_LOCAL_OPENDIR_OPEN "file:///"

#define TEST_LFC_OPENDIR_OPEN_INVALID TEST_CONCAT_LFC_PREFIX("/cannotbethere")
#define TEST_LFC_OPENDIR_OPEN			TEST_CONCAT_LFC_PREFIX("/testopendir0011")
#define TEST_LFC_OPENDIR_OPEN_NOACCESS TEST_CONCAT_LFC_PREFIX("/testopendir0012")

#define TEST_SRM_OPENDIR_OPEN TEST_CONCAT_SRM_PREFIX("testopendir0011")
#define TEST_SRML_OPENDIR_ENOENT TEST_CONCAT_SRM_PREFIX("iamthequeenornot")
#define TEST_SRML_OPENDIR_EACCESS TEST_CONCAT_SRM_PREFIX("testopendireacces0011/testopendir0011")

// readdir
#define TEST_LOCAL_READDIR_CREATE_COMMAND "mkdir -p /tmp/testreaddir0011/testreaddir0012 &> /dev/null && \
mkdir -p /tmp/testreaddir0011/testreaddir0013 &> /dev/null &&  mkdir -p /tmp/testreaddir0011/testreaddir0014 &> /dev/null && \
mkdir -p /tmp/testreaddir0011/testreaddir0015 &> /dev/null"
#define TEST_LOCAL_READDIR_VALID "file:///tmp/testreaddir0011"
#define TEST_LOCAL_READDIR_1 "testreaddir0012"
#define TEST_LOCAL_READDIR_2 "testreaddir0013"
#define TEST_LOCAL_READDIR_3 "testreaddir0014"
#define TEST_LOCAL_READDIR_4 "testreaddir0015"


#define TEST_LFC_READDIR_VALID TEST_CONCAT_LFC_PREFIX("/testreaddir0011")
#define TEST_LFC_READDIR_1 "testreaddir0012"
#define TEST_LFC_READDIR_2 "testreaddir0013"
#define TEST_LFC_READDIR_3 "testreaddir0014"
#define TEST_LFC_READDIR_4 "testreaddir0015"

#define TEST_SRM_READDIR_VALID TEST_CONCAT_SRM_PREFIX("/testreaddir0011")
#define TEST_SRM_READDIR_1 "testreaddir0012"
#define TEST_SRM_READDIR_2 "testreaddir0013"
#define TEST_SRM_READDIR_3 "testreaddir0014"
#define TEST_SRM_READDIR_4 "testreaddir0015"


// open
#define TEST_LOCAL_OPEN_CREATE_COMMAND "echo  \"Hello world\" > /tmp/testopen0011 && touch /tmp/testopen0013 && chmod 000 /tmp/testopen0013"
#define TEST_LOCAL_WRITE_CREATE_COMMAND "rm -f /tmp/testwrite001"
#define TEST_LOCAL_OPEN_EXIST "file:///tmp/testopen0011"
#define TEST_LOCAL_OPEN_NOEXIST "file:///tmp/testopen0012"
#define TEST_LOCAL_OPEN_NOACCESS  "file:///tmp/testopen0013"
#define TEST_LOCAL_WRITE_VALID "file:///tmp/testwrite001"
#define TEST_LOCAL_READ_CONTENT "Hello world\n"

#define TEST_SRM_OPEN_EXIST TEST_SRM_ONLY_READ_ACCESS
#define TEST_SRM_OPEN_NOEXIST TEST_SRM_NOEXIST_ACCESS
#define TEST_SRM_OPEN_NOACCESS TEST_CONCAT_SRM_PREFIX("testopen0011/testopen0012")
#define TEST_SRM_OPEN_WRITE TEST_CONCAT_SRM_PREFIX("testwritetmp0011")

#define TEST_LFC_OPEN_EXIST TEST_LFC_ONLY_READ_ACCESS
#define TEST_LFC_OPEN_NOEXIST TEST_LFC_NOEXIST_ACCESS
#define TEST_LFC_OPEN_NOACCESS TEST_CONCAT_LFC_PREFIX("/testopen0011/testopen0012")
#define TEST_LFC_OPEN_WRITE TEST_CONCAT_LFC_PREFIX("/testwritetmp0011")

#define TEST_GUID_OPEN_EXIST TEST_GUID_VALID_FILE
#define TEST_GUID_OPEN_NONEXIST TEST_GUID_NOEXIST_ACCESS

// xattr
#define TEST_LFC_VALID_COMMENT TEST_CONCAT_LFC_PREFIX("/testcomment0011")
#define TEST_LFC_WRITEVALID_COMMENT TEST_CONCAT_LFC_PREFIX("/testcomment0012")
#define TEST_LFC_INVALID_COMMENT TEST_CONCAT_LFC_PREFIX("/testpsg0011")
#define TEST_LFC_COMMENT_CONTENT "Hello World"
#define TEST_LFC_VALID_TESTREAD_CHECKSUM ""
#define TEST_LFC_VALID_TESTREAD_CHKTYPE ""


#define TEST_SRM_CHECKSUM_VALID ""
#define TEST_SRM_CHKTYPE_VALID ""
