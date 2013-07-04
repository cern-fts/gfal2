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
#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_constants.h>
#include "gfal_rfio_mock_test.h"


char defined_buff_read[2048];
int defined_buff_read_size;
char defined_buff_write[2048];
int defined_buff_write_size;

int rfio_mock_open(const char* path, int flag, ...){
	int i = mock(path, flag);
	if(i <= 0){
		errno=i;
		return -1;
	}
	return i;
}

off_t rfio_mock_lseek(int fd, off_t offset, int whence){
	off_t i = mock(fd, offset, whence);
	if(i < 0){
		errno=i;
		return -1;
	}
	return (off_t)i;	
	
}

ssize_t rfio_mock_read(int fd, void* buff, size_t size){
	int i = mock(fd, buff, size);
	if(i < 0){
		errno=i;
		return -1;
	}
	memcpy(buff, (void*) defined_buff_read, defined_buff_read_size);
	return (ssize_t)i;
}


ssize_t rfio_mock_write(int fd, const void* buff, size_t size){
	int i = mock(fd, buff, size);
	if(i < 0){
		errno=i;
		return -1;
	}
	memcpy(buff, (void*) defined_buff_read, defined_buff_read_size);
	return (ssize_t)i;
}


int rfio_mock_close(int fd){
	int i = mock(fd);
	if(i != 0){
		errno=i;
		return -1;
	}
	return i;
}





