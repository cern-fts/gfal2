#pragma once
#ifndef _GRIDFTP_IFCE_INCLUDE_h
#define _GRIDFTP_IFCE_INCLUDE_h

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

// #include <gridftp-ifce.h> temporary disable because of the header bugs ...



#include <globus_gass_copy.h>
#include <globus_ftp_client.h>
#include <globus_ftp_client_restart_marker_plugin.h>
#include <globus_ftp_client_restart_plugin.h>
#include <globus_ftp_client_debug_plugin.h>


#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

#ifndef LONG64LONG64
#if defined(__ia64__) || defined(__x86_64)
#define LONG64 long
#elif defined(_WIN32)
#define LONG64 __i64
#else
#define LONG64 long long
#endif
#endif

#define ERROR_BUFFER_SIZE 1024

    /*
    GridFTP Operation supported:
            -Make directory
            -Delete file
            -Get file size
            -Checksum
            -Third-party tranfer
            -Url-to-Url copy
     */

    /**
     * dmutils_gridftp_check_gsi_url
     * Utility function to check of the url is gsiftp
     * Required parameters: All
     * Optional paramaters: None
     * @return 0 or -1
     */

    inline int dmutils_gridftp_check_gsi_url(const char* url) {
        return strncmp(url, "gsiftp:", 7) == 0 ? 0 : -1;
    }

    /**
     * dmutils_gridftp_delete
     * Delete a file using gridftp. 
     * Required parameters: All
     * Optional paramaters: None
     * @return GLOBUS_SUCCESS or GLOBUS_FALSE
     */
    globus_result_t dmutils_gridftp_delete(
            const char* file,
            char* errbuf,
            int errbufsz,
            int timeout);

    /**
     * dmutils_gridftp_make_dir
     * Creates a directory  using gridftp. 
     * Required parameters: All
     * Optional paramaters: None
     * @return GLOBUS_SUCCESS or GLOBUS_FALSE
     */
    globus_result_t dmutils_gridftp_make_dir(
            char* dir,
            char *errbuf,
            int errbufsz,
            int timeout);


    /**
     * dmutils_gridftp_get_filesize
     * Retrieve the size of a file using gridftp. 
     * Required parameters: All
     * Optional paramaters: None
     * @return GLOBUS_SUCCESS or GLOBUS_FALSE
     */
    globus_result_t dmutils_gridftp_get_filesize(
            char const* file,
            globus_off_t* size,
            char* errbuf,
            int errbufsz,
            int timeout);

    /**
     * dmutils_gridftp_checksum
     * Checksum using GridFTP. 
     * Required parameters: All
     * Optional paramaters: None
     * @return GLOBUS_SUCCESS or GLOBUS_FALSE
     */
    globus_result_t dmutils_gridftp_checksum(
            char const* file,
            char* cksm,
            char const* cksm_alg,
            char* errbuf,
            int errbufsz,
            int timeout);

    /**
     * dmutils_gridftp_copy
     * Copy a file using GridFTP. 
     * Required parameters: All
     * Optional paramaters: none
     * @return GLOBUS_SUCCESS or GLOBUS_FALSE
     */
    globus_result_t dmutils_gridftp_copy(
            const char* source_url,
            const char* dest_url,
            char *errbuf,
            int errbufsz,
            int timeout,
            int const nbstreams,
            int const tcp_bs,
            int const block_size,
            int verbose,
            globus_gass_copy_handle_t *ggc_handle,
            void (*ggc_callback)(void *callback_arg, globus_gass_copy_handle_t *handle, globus_object_t *error),
            void * monitor_external,
            void (*globus_gass_copy_performance_cb_t_etx)(void *user_arg,
            globus_gass_copy_handle_t *handle,
            globus_off_t total_bytes,
            float instantaneous_throughput,
            float avg_throughput),
            void * check_external,
	    int (*gridftp_copy_wait)(void * monitor, void * check, globus_gass_copy_handle_t *gfhp, int const timeout, void * iupdater),
	    void * iupdater);


    /**
     * dmutils_gridftp_error_to_string
     * Delete a file using gridftp. 
     * Required parameters: globus_object_t* errobj)
     * Optional paramaters: none
     * @return the string representation of globus_error
     */
    char* dmutils_gridftp_error_to_string(
            globus_object_t* errobj);





#ifdef __cplusplus
}
#endif // __cplusplus





#endif //_GRIDFTP_IFCE_INCLUDE_h

