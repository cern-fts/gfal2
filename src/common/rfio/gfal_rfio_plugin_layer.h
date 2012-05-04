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


/*
 * @file gfal_rfio_plugin_layer.c
 * @brief header file for the external call, abstraction layer for mock purpose
 * @author Devresse Adrien
 * @version 0.1
 * @date 30/06/2011
 * 
 **/
 
#include <glib.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>

extern  struct rfio_proto_ops * (*gfal_rfio_internal_loader)(GError** err);

struct rfio_proto_ops {
	int	(*geterror)();
	int	(*access)(const char *, int);
	int	(*chmod)(const char *, mode_t);
	int	(*close)(int);
	int	(*closedir)(DIR *);
	char* (*serror_r)(char* buff, size_t buff_size);
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

