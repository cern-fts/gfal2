/*
 * Copyright (c) CERN 2013-2015
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

#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include <glib.h>
#include <dcap.h>

#include <gfal_plugins_api.h>


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
    int	(*lstat)(const char *, struct stat *);
	int	(*mkdir)(const char *, mode_t);
	int	(*open)(const char *, int, ...);
	DIR	*(*opendir)(const char *);
	ssize_t	(*read)(int, void *, size_t);

	ssize_t (*pread)(int fildes, void *buf, size_t nbytes, off_t offset);
	ssize_t (*pwrite)(int fildes, const void *buf, size_t nbytes, off_t offset);
	struct dirent	*(*readdir)(DIR *);
	int	(*rename)(const char *, const char *);
	int	(*rmdir)(const char *);
	ssize_t	(*setfilchg)(int, const void *, size_t);
	int	(*stat)(const char *, struct stat *);
	int	(*unlink)(const char *);
	ssize_t	(*write)(int, const void *, size_t);
};


extern struct dcap_proto_ops * (*gfal_dcap_internal_loader)(GError** err);
