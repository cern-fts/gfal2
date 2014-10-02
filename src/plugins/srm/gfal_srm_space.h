#pragma once
/*
 * @file gfal_srm_rmdir.h
 * @brief header file for the rmdir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 22/05/2011
 * */

#include <glib.h>
#include "gfal_srm_internal_layer.h"

ssize_t gfal_srm_space_getxattrG(plugin_handle handle, const char* path, const char* name , void* buff, size_t s_buff, GError** err);

