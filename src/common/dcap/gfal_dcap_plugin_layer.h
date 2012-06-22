/* 
* Copyright @ Members of the EMI Collaboration, 2010.
* See www.eu-emi.eu for details on the copyright holders.
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


/*
 *  gfal_dcap_plugin_layer.h
 *  file for the external call, abstraction layer for mock purpose
 *  author: Devresse Adrien
 * 
 **/


#include <glib.h>
#include <common/gfal_common_internal.h>
#include <common/gfal_common_errverbose.h>
#include <common/gfal_common_plugin.h>
#include <common/gfal_types.h>

#include <dcap.h>

struct dcap_proto_ops {
	int*	(*geterror)();
	const char*(*strerror)(int);
	int	(*access)(const char *, int);
	int	(*chmod)(const char *, mode_t);
	int	(*close)(int);
	int	(*closedir)(DIR *);
	void (*debug_level)(unsigned int);
	void (*active_mode)();
	off_t	(*lseek)(int, off_t, int);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
	off64_t	(*lseek64)(int, off64_t, int);
#endif
	int	(*lstat)(const char *, struct stat *);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
	int	(*lstat64)(const char *, struct stat64 *);
#endif
	int	(*mkdir)(const char *, mode_t);
	int	(*open)(const char *, int, ...);
	DIR	*(*opendir)(const char *);
	ssize_t	(*read)(int, void *, size_t);
	
	ssize_t (*pread)(int fildes, void *buf, size_t nbytes, off_t offset);
	ssize_t (*pwrite)(int fildes, const void *buf, size_t nbytes, off_t offset);
	struct dirent	*(*readdir)(DIR *);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
	struct dirent64	*(*readdir64)(DIR *);
#endif
	int	(*rename)(const char *, const char *);
	int	(*rmdir)(const char *);
	ssize_t	(*setfilchg)(int, const void *, size_t);
	int	(*stat)(const char *, struct stat *);
#if ! defined(linux) || defined(_LARGEFILE64_SOURCE)
	int	(*stat64)(const char *, struct stat64 *);
#endif
	int	(*unlink)(const char *);
	ssize_t	(*write)(int, const void *, size_t);
};


extern struct dcap_proto_ops * (*gfal_dcap_internal_loader)(GError** err);

