#pragma once
/*
 * 
 *  convenience function for the mocks or the lfc interface
 * 
 */

#include <cgreen/cgreen.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "gfal_constants.h"


extern char defined_buff_read[2048];
extern int defined_buff_read_size;
extern char defined_buff_write[2048];
extern int defined_buff_write_size;

off_t rfio_mock_lseek(int fd, off_t offset, int whence);

ssize_t rfio_mock_read(int fd, void* buff, size_t size);

ssize_t rfio_mock_write(int fd, const void* buff, size_t size);

int rfio_mock_close(int fd);

int rfio_mock_open(const char* path, int flag, ...);




