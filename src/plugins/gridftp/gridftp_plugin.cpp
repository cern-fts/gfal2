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

#include "gridftpmodule.h"
#include "gridftp_namespace.h"
#include "gridftp_filecopy.h"
#include "gridftp_io.h"
#include "gridftp_plugin.h"
#include <exceptions/cpp_to_gerror.hpp>


extern "C"{


static bool is_gridftp_uri(const char* src)
{
    static const char gridftp_prefix[] = "gsiftp://";
    static const char ftp_prefix[] = "ftp://";
    return (
        strncmp(src, gridftp_prefix, sizeof(gridftp_prefix) - 1) == 0 ||
        strncmp(src, ftp_prefix, sizeof(ftp_prefix) - 1) == 0
    );
}



gboolean gridftp_check_url_transfer(plugin_handle handle, gfal2_context_t context, const char* src,
                                    const char* dst, gfal_url2_check type)
{
    g_return_val_if_fail(handle != NULL, FALSE);
    gboolean res = FALSE;

    if( src != NULL && dst != NULL){
        bool is_gridftp_transfer = is_gridftp_uri(src) && is_gridftp_uri(dst);
        if (type == GFAL_FILE_COPY || type == GFAL_BULK_COPY)
            res = is_gridftp_transfer;
    }
    return res;
}



int gridftp_check_url(plugin_handle handle, const char* src, plugin_mode check,
                      GError ** err)
{
    gboolean res = FALSE;
    if(is_gridftp_uri(src)) {
        switch(check){
            case GFAL_PLUGIN_ACCESS:
            case GFAL_PLUGIN_STAT:
            case GFAL_PLUGIN_LSTAT:
            case GFAL_PLUGIN_MKDIR:
            case GFAL_PLUGIN_CHMOD:
            case GFAL_PLUGIN_RMDIR:
            case GFAL_PLUGIN_OPENDIR:
            case GFAL_PLUGIN_UNLINK:
            case GFAL_PLUGIN_OPEN:
            case GFAL_PLUGIN_CHECKSUM:
            case GFAL_PLUGIN_RENAME:
                res = TRUE;
                break;
            default:
                break;

        }
    }
    return res;

}



plugin_handle gridftp_plugin_load(gfal2_context_t handle, GError ** err)
{
	GError * tmp_err=NULL;
	plugin_handle h = NULL;
	CPP_GERROR_TRY
		gfal2_log(G_LOG_LEVEL_DEBUG, " -> [gridftp_plugin] try to load ..");
		 h = static_cast<plugin_handle>(
					new GridFTPModule( new GridFTPFactory(handle) )
			);
		gfal2_log(G_LOG_LEVEL_DEBUG, " -> [gridftp_plugin] loaded ..");
	CPP_GERROR_CATCH(&tmp_err);

	G_RETURN_ERR(h, tmp_err, err);
}


void gridftp_plugin_unload(plugin_handle handle)
{
	if(handle){
		try{
			delete (static_cast<GridFTPModule*>(handle));
		}catch(...){
			gfal2_log(G_LOG_LEVEL_MESSAGE, " bug found plugin gridFTP throws error while loading");
		}
	}
}



const char *gridftp_plugin_name()
{
	return GFAL2_PLUGIN_VERSIONED("gridftp", VERSION);
}


/**
 * Map function for the gridftp interface
 * this function provide the generic PLUGIN interface for the gridftp plugin.
 **/
gfal_plugin_interface gfal_plugin_init(gfal2_context_t handle, GError** err)
{
    GError* tmp_err = NULL;

    gfal_plugin_interface ret;
    memset(&ret, 0, sizeof(gfal_plugin_interface));
    plugin_handle r = gridftp_plugin_load(handle, &tmp_err);

    ret.plugin_data = r;
    ret.check_plugin_url = &gridftp_check_url;
    ret.plugin_delete = &gridftp_plugin_unload;
    ret.getName = &gridftp_plugin_name;
    ret.accessG = &gfal_gridftp_accessG;
    ret.statG = & gfal_gridftp_statG;
    ret.lstatG = &gfal_gridftp_statG;
    ret.unlinkG = &gfal_gridftp_unlinkG;
    ret.mkdirpG = &gfal_gridftp_mkdirG;
    ret.chmodG = &gfal_gridftp_chmodG;
    ret.rmdirG = &gfal_gridftp_rmdirG;
    ret.opendirG = &gfal_gridftp_opendirG;
    ret.readdirG = &gfal_gridftp_readdirG;
    ret.readdirppG = &gfal_gridftp_readdirppG;
    ret.closedirG = &gfal_gridftp_closedirG;
    ret.openG = &gfal_gridftp_openG;
    ret.closeG = &gfal_gridftp_closeG;
    ret.readG = &gfal_gridftp_readG;
    ret.writeG = &gfal_gridftp_writeG;
    ret.lseekG = &gfal_gridftp_lseekG;
    ret.checksum_calcG = &gfal_gridftp_checksumG;
    ret.renameG = &gfal_gridftp_renameG;
    ret.check_plugin_url_transfer = &gridftp_check_url_transfer;
    ret.copy_file = &gridftp_plugin_filecopy;
    ret.copy_bulk = &gridftp_bulk_copy;

	G_RETURN_ERR(ret, tmp_err, err);
}

};
