#pragma once
#ifndef _GFAL_CONSTANTS_H
#define _GFAL_CONSTANTS_H

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
 

 
/**
  * @file gfal_constants.h 
  * the global constant declarations of gfal core
  * @author Devresse Adrien , Baud Jean-Philippe 
*/

#ifdef __cplusplus
extern "C"
{
#endif 




#define MAX_PLUGIN_LIST 64

/* used to check wether there is enough space on the SE */
#define GFAL_NEWFILE_SIZE         1024
#define GFAL_MODULE_NAME_SIZE 	1024
#define GFAL_OPEN_MAX             1024
#define GFAL_DEFAULT_PLUGIN_TYPE "lfc"
#define GFAL_FQAN_MAX             64
#define GFAL_GUID_LEN             37
#define GFAL_VO_MAXLEN            255
#define GFAL_FQAN_MAXLEN          511
#define GFAL_HOSTNAME_MAXLEN      255
#define GFAL_PATH_MAXLEN          1024
#define GFAL_ERRMSG_LEN           1024
#define GFAL_BDII_TIMEOUT         60
#define GFAL_SIZE_MARGIN          1048576     // 1MB
#define GFAL_SRM_MAXRETRIES       10
#define GFAL_MAX_PLUGIN_ID_SIZE 512

/* GFAL error level for gfal_errmsg */
#define GFAL_ERRMSG_LEN			  1024
#define GFAL_ERRLEVEL_ERROR       0
#define GFAL_ERRLEVEL_WARN        1
#define GFAL_ERRLEVEL_INFO        2

#define GFAL_VERBOSE_NORMAL     0x00    /*!< \def GFAL_VERBOSE_NORMAL only errors are printed */
#define GFAL_VERBOSE_VERBOSE    0x01    /*!< \def GFAL_VERBOSE_VERBOSE a bit more verbose information is printed */
#define GFAL_VERBOSE_DEBUG      0x02    /*!< \def GFAL_VERBOSE_DEBUG  extra information is printed */
#define GFAL_VERBOSE_TRACE		0x08	/*! < \def GFAL_VERBOSE_TRACE execution trace */


#define GFAL_GUID_PREFIX "guid:"

// default buffer size for address
#define GFAL_URL_MAX_LEN 2048

#define GFAL_MODULEID_LOCAL "local_file_module"
#define GFAL_MODULEID_SRM 2
#define GFAL_EXTERNAL_MODULE_OFFSET 10

#define GFAL_PLUGIN_DIR_ENV "GFAL_PLUGIN_DIR" /**<  environment variable for personnalized plugin directory  */
#define GFAL_PLUGIN_DIR_SUFFIX "gfal2-plugins" /**< default directory name for gfal 2 plugin search */
#define GFAL_PLUGIN_INIT_SYM "gfal_plugin_init"
#define GFAL_MAX_PLUGIN_LIST 2048


// xattr standard keys for getxattr / setxattr
#define GFAL_XATTR_REPLICA "user.replicas" // global key of replicas for the extended attributes 
#define GFAL_XATTR_GUID "user.guid" // global key for the guid of a file 
#define GFAL_XATTR_COMMENT "user.comment" // global key for the comments of a file 
#define GFAL_XATTR_CHKSUM_TYPE "user.chksumtype"
#define GFAL_XATTR_CHKSUM_VALUE "user.checksum"
#define GFAL_XATTR_STATUS "user.status" 

// xattr standard values 
// list of standard values return by getxattr for status key
#define GFAL_XATTR_STATUS_ONLINE "ONLINE"
#define GFAL_XATTR_STATUS_NEARLINE "NEARLINE "
#define GFAL_XATTR_STATUS_NEARLINE_ONLINE "ONLINE_AND_NEARLINE"
#define GFAL_XATTR_STATUS_UNKNOW "UNKNOW"
#define GFAL_XATTR_STATUS_LOST "LOST"
#define GFAL_XATTR_STATUS_UNAVAILABLE "UNAVAILABLE"



// plugins entry points for the new system of plugins
//  int plugin_url_transfer_check(plugin_handle handle, const char* src, const char* dst, gfal_transfer_type type )
// return true if url is compatible with this plugin else false
#define GFAL_PLUGIN_URL_TRANSFER_CHECK "plugin_url_transfer_check"


// parameters list for core
#define GFAL_NO_BDII_OPTION "no_bdii" // set bdii value, can be True or False


#ifdef __cplusplus
}
#endif 

#endif
