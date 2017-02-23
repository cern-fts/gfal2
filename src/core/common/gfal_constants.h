/*
 * Copyright (c) CERN 2013-2017
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#ifndef GFAL_CONSTANTS_H_
#define GFAL_CONSTANTS_H_

#if !defined(__GFAL2_H_INSIDE__) && !defined(__GFAL2_BUILD__)
#   warning "Direct inclusion of gfal2 headers is deprecated. Please, include only gfal_api.h or gfal_plugins_api.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/** Maximum number of plugins */
#define MAX_PLUGIN_LIST 15

/** Module name size */
#define GFAL_MODULE_NAME_SIZE 	1024

/** GFAL error level for gfal_errmsg */
#define GFAL_ERRMSG_LEN			  2048

/** default buffer size for address */
#define GFAL_URL_MAX_LEN 2048

/** environment variable for personnalized plugin directory  */
#define GFAL_PLUGIN_DIR_ENV "GFAL_PLUGIN_DIR"
/** default directory name for gfal 2 plugin search */
#define GFAL_PLUGIN_DIR_SUFFIX "gfal2-plugins"
/** plugin entry point */
#define GFAL_PLUGIN_INIT_SYM "gfal_plugin_init"

/**  environment variable for personnalized configuration directory */
#define GFAL_CONFIG_DIR_ENV "GFAL_CONFIG_DIR"
/**  folder name under /etc for the configuration files */
#define GFAL_CONFIG_DIR_SUFFIX "gfal2.d"

/* xattr standard keys for getxattr / setxattr */
/** replicas listing */
#define GFAL_XATTR_REPLICA "user.replicas"
/** guid information */
#define GFAL_XATTR_GUID "user.guid"
/** file comment */
#define GFAL_XATTR_COMMENT "user.comment"
/** file checksum type */
#define GFAL_XATTR_CHKSUM_TYPE "user.chksumtype"
/** file checksum */
#define GFAL_XATTR_CHKSUM_VALUE "user.checksum"

/**
 * File availability status
 * This key can be used to check or set the stage status of a file
 */
#define GFAL_XATTR_STATUS "user.status"

/** String value to use/compare for extended attribute user.status
 * user.status possible value, similar to SRM meaning of status ( brings_online )
 */
#define GFAL_XATTR_STATUS_ONLINE "ONLINE"
#define GFAL_XATTR_STATUS_NEARLINE "NEARLINE "
#define GFAL_XATTR_STATUS_NEARLINE_ONLINE "ONLINE_AND_NEARLINE"
#define GFAL_XATTR_STATUS_UNKNOW "UNKNOW"
#define GFAL_XATTR_STATUS_LOST "LOST"
#define GFAL_XATTR_STATUS_UNAVAILABLE "UNAVAILABLE"

/** space reporting */
#define GFAL_XATTR_SPACETOKEN "spacetoken"


#ifdef __cplusplus
}
#endif

#endif /* GFAL_CONSTANTS_H_ */
