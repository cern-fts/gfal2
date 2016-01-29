#
# compilation lines for test parameters

## STAT Tests

SET(MY_VO "dteam")
SET(MY_VO_STORM "${MY_VO}")
SET(TEST_ENVIRONMENT "PRODUCTION"
        CACHE STRING "Define the target for functional test")

## Global environment
SET(ftp_prefix "ftp://root.cern.ch/root/")

IF(TEST_ENVIRONMENT STREQUAL "TESTBED_RC")

## Testbed environment
SET(file_base_path "/tmp/")
SET(srm_prefix_storm "srm://storm-01.roma3.infn.it:8444/srm/manager?SFN=/${MY_VO_STORM}/gfal2-tests/")
SET(dcap_prefix "gsidcap://dcache-door-desy09.desy.de:22128/pnfs/desy.de/${MY_VO}/gfal2-tests/")
SET(srm_prefix_dcache "srm://grid002.ft.uam.es/pnfs/ft.uam.es/data/${MY_VO}/gfal2-tests/")
SET(lfc_prefix "lfn:/grid/${MY_VO}/gfal2-tests/")
SET(lfc_host_name "lfc-puppet02.cern.ch")
SET(gsiftp_prefix_dpm "gsiftp://dpmhead-rc.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(srm_prefix_dpm "srm://dpmhead-rc.cern.ch:8446/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(davs_prefix_dpm "davs+3rd://dpmhead-rc.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(root_prefix_dpm "root://dpmhead-rc.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")

ELSEIF(TEST_ENVIRONMENT STREQUAL "TESTBED_TRUNK")

## Testbed trunk environment
SET(file_base_path "/tmp/")
SET(srm_prefix_storm "srm://storm-01.roma3.infn.it:8444/srm/manager?SFN=/${MY_VO_STORM}/gfal2-tests/")
SET(dcap_prefix "gsidcap://vm-dcache-deploy6.desy.de/data/${MY_VO}/gfal2-tests/")
SET(srm_prefix_dcache "srm://vm-dcache-deploy6.desy.de:8443/data/${MY_VO}/gfal2-tests")
SET(lfc_prefix "lfn:/grid/${MY_VO}/gfal2-tests/")
SET(lfc_host_name "lfc-puppet02.cern.ch")
SET(gsiftp_prefix_dpm "gsiftp://dpmhead-trunk.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(srm_prefix_dpm "srm://dpmhead-trunk.cern.ch:8446/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(davs_prefix_dpm "davs+3rd://dpmhead-trunk.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(root_prefix_dpm "root://dpmhead-trunk.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")

ELSE(TEST_ENVIRONMENT STREQUAL "TESTBED_RC")

## Production environment : default
SET(file_base_path "/tmp/")
SET(srm_prefix_storm "srm://storm-01.roma3.infn.it:8444/srm/manager?SFN=/${MY_VO_STORM}/gfal2-tests/")
SET(dcap_prefix "gsidcap://dcache-door-desy09.desy.de:22128/pnfs/desy.de/${MY_VO}/gfal2-tests/")
SET(srm_prefix_dcache "srm://grid002.ft.uam.es/pnfs/ft.uam.es/data/${MY_VO}/gfal2-tests/")
SET(lfc_prefix "lfn:/grid/${MY_VO}/gfal2-tests/")
SET(lfc_host_name "lfc-puppet02.cern.ch")
SET(gsiftp_prefix_dpm "gsiftp://marsedpm.in2p3.fr/dpm/in2p3.fr/home/${MY_VO}/gfal2-tests/")
SET(srm_prefix_dpm "srm://marsedpm.in2p3.fr:8446/srm/managerv2?SFN=/dpm/in2p3.fr/home/${MY_VO}/gfal2-tests/")

# Need to find something better!
SET(davs_prefix_dpm "davs+3rd://dpmhead-rc.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(root_prefix_dpm "root://marsedpm.in2p3.fr/dpm/in2p3.fr/home/${MY_VO}/gfal2-tests")

ENDIF(TEST_ENVIRONMENT STREQUAL "TESTBED_RC")

#dpm parameters
SET(srm_valid_dpm_stat "${srm_prefix_dpm}/testread0011")
SET(srm_valid_dpm_bigfile "${srm_prefix_dpm}/testbig")
SET(srm_valid_dir_root "${srm_prefix_dpm}")
SET(srm_valid_dpm_src_file "${srm_valid_dpm_stat}")

# dcache parameters
SET(srm_valid_dcache_bigfile "${srm_prefix_dcache}/testbig")
SET(srm_valid_dcache_stat "${srm_prefix_dcache}/testread0011")
SET(srm_valid_dcache_dir_root "${srm_prefix_dcache}")
SET(srm_valid_dcache_src_file "${srm_valid_dcache_stat}")

# storm parameters

SET(srm_valid_storm_stat "${srm_prefix_storm}/testread00111")
SET(srm_valid_storm_dir_root "${srm_prefix_storm}")
SET(srm_valid_storm_src_file "${srm_prefix_storm}")

## gsiftp parameters
#
SET(gsiftp_valid_dpm_stat "${gsiftp_prefix_dpm}/testread0011")
SET(gsiftp_valid_dpm_src_file "${gsiftp_valid_dpm_stat}")
SET(gsiftp_valid_dir_root "${gsiftp_prefix_dpm}")

## https parameters
#
SET(davs_valid_dpm_stat     "${davs_prefix_dpm}/testread0011")
SET(davs_valid_dpm_src_file "${davs_valid_dpm_stat}")
SET(davs_valid_dir_root     "${davs_prefix_dpm}")

## xrootd parameters
SET(root_valid_dpm_stat     "${root_prefix_dpm}/testread0011")
SET(root_valid_dpm_src_file "${root_valid_dpm_stat}")
SET(root_valid_dir_root     "${root_prefix_dpm}")

## lfc parameters
SET(lfc_stat_ok "${lfc_prefix}/testread0011")
SET(lfc_chmod_ok "${lfc_prefix}/test_change_right")
SET(lfc_valid_dir_root "${lfc_prefix}")
SET(lfc_full_prefix "lfc://${lfc_host_name}/grid/${MY_VO}")
SET(lfc_full_stat_ok "${lfc_full_prefix}/testread0011")
SET(lfc_full_chmod_ok "${lfc_full_prefix}/test_change_right")
SET(lfc_full_valid_dir_root "${lfc_full_prefix}")

# dcap dcache parameters
SET(dcap_stat_ok "${dcap_prefix}/testread0011")
SET(dcap_valid_dir_root "${dcap_prefix}")

#local file parameters
SET(file_prefix "file://${file_base_path}")
FILE(WRITE "${file_base_path}/testread_0011" "hello world agdlkmgfmklmklklmvc;!:c;:!;:!xc;!:vx!;:bvx!;:!;:o=)=)à=àdg:;;:!:!;!:;b")
SET(file_stat_ok "${file_prefix}/testread_0011")
SET(file_valid_chmod "${file_prefix}/test_change_right")

IF(PLUGIN_FILE)
        test_del_nonex("FILE" "file://${file_base_path}")
        test_del("FILE" "file://${file_base_path}")
        test_mkdir_unlink("FILE" "file://${file_base_path}")
        test_rename("FILE" "file://${file_base_path}")
        test_access("FILE" "file://${file_base_path}")
        test_stat_all("FILE" ${file_prefix})
        test_chmod_all("FILE" file://${file_base_path} 0565 060 360 767)
        test_mkdir_all("FILE" ${file_prefix})
        test_rmdir_all("FILE" ${file_prefix})
        test_readdir_full("FILE" ${file_prefix})
        test_rwt_all("FILE" ${file_prefix} 4578)
        test_rwt_all("FILE" ${file_prefix} 1)
        test_rwt_all("FILE" ${file_prefix} 100000)
        test_rwt_seq("FILE" ${file_prefix} 100 4560)
        test_rwt_seek("FILE" ${file_prefix} 100 4560)
        test_checksum_simple("FILE_MD5" ${file_prefix} MD5)
        test_checksum_simple("FILE_ADLER32" ${file_prefix} ADLER32)
        test_checksum_simple("FILE_CRC32" ${file_prefix} CRC32)

        gfal_test_posix("FILE" ${file_prefix})
ENDIF(PLUGIN_FILE)

IF(PLUGIN_SRM)
        # del dir test
        test_del_nonex("SRM_DPM"    "${srm_prefix_dpm}")
        test_del_nonex("SRM_STORM"  "${srm_prefix_storm}")
        test_del_nonex("SRM_DCACHE" "${srm_prefix_dcache}")
        test_del("SRM_DPM"     "${srm_prefix_dpm}")
        test_del("SRM_STORM"   "${srm_prefix_storm}")
        test_del("SRM_DCACHE"  "${srm_prefix_dcache}")
        # mkdir tests
        test_mkdir_unlink("SRM_DPM"     "${srm_prefix_dpm}")
        test_mkdir_unlink("SRM_STORM"   "${srm_prefix_storm}")
        test_mkdir_unlink("SRM_DCACHE"  "${srm_prefix_dcache}")
        # stat tests
        test_stat_all("SRM_DPM" ${srm_prefix_dpm})
        test_stat_all("SRM_DCACHE" ${srm_prefix_dcache})
        test_access("SRM_DPM" ${srm_prefix_dpm})
        # dCache says Operation not supported
        # test_access("SRM_DCACHE" ${srm_prefix_dcache})
        # Rename test
        test_rename("SRM_DPM" ${srm_prefix_dpm})
        test_rename("SRM_DCACHE" ${srm_prefix_dcache})
        # checksum tests
        test_checksum_simple("SRM_DPM_ADLER32" ${srm_prefix_dpm} ADLER32)
        test_checksum_simple("SRM_DPM_MD5" ${srm_prefix_dpm} MD5)
        test_checksum_simple("SRM_DCACHE_ADLER32" ${srm_prefix_dcache} ADLER32)
       # test_checksum_simple("SRM_DCACHE_MD5" ${srm_valid_dcache_stat} MD5) Dcache does not support dynamic checksum calculation
        test_mkdir_all("SRM_DPM" ${srm_prefix_dpm})
        test_mkdir_all("SRM_DCACHE" ${srm_prefix_dcache})
        test_chmod_all("SRM_DPM" ${srm_valid_dir_root} 0575 070 370 777)
        test_rmdir_all("SRM_DPM" ${srm_valid_dir_root})
        test_readdir_full("SRM_DPM" ${srm_valid_dir_root} )     
        test_rwt_all("SRM_DPM" ${srm_valid_dir_root} 4578)
        test_rwt_all("SRM_DPM_little" ${srm_valid_dir_root} 10)
        test_rwt_all("SRM_DPM_single" ${srm_valid_dir_root} 1)  
        test_rwt_all("SRM_DCAP" ${srm_valid_dcache_dir_root} 4578)
        test_rwt_all("SRM_DCAP_little" ${srm_valid_dcache_dir_root} 10) 
        test_rwt_all("SRM_DCAP_single" ${srm_valid_dcache_dir_root} 1)          
        test_rwt_seq("SRM_DPM" ${srm_valid_dir_root} 100 4560)  
        test_rwt_seq("SRM_DPM_unit" ${srm_valid_dir_root} 1 10)                 
        test_rwt_seq("SRM_DCAP" ${srm_valid_dir_root} 100 4560) 

        test_rwt_seq("SRM_STORM" ${srm_prefix_storm} 100 4560)
        
        # Bringonline
        test_bringonline("SRM_DPM" ${srm_prefix_dpm})
        test_bringonline("SRM_DCACHE" ${srm_prefix_dcache})
        # Note: STORM may not support tapes
                
#       test_chmod_all("SRM_DCACHE" ${srm_valid_dcache_chmod} 0565 060 360 767)  -> disabled, since unavailable on dcache
#       test_stat_all( "SRM_EOS" ${srm_valid_EOS_stat})

        test_xattr("DPM" ${srm_prefix_dpm})
        test_xattr("DCACHE" ${srm_prefix_dcache})
ENDIF(PLUGIN_SRM)


IF(PLUGIN_LFC)
        test_stat_all( "LFC" ${lfc_prefix})
        test_mkdir_all("LFC" ${lfc_prefix})
        test_chmod_all("LFC" ${lfc_valid_dir_root} 0565 060 0360 0767)
        test_rmdir_all("LFC" ${lfc_valid_dir_root})
        test_readdir_full("LFC" ${lfc_valid_dir_root} )
        # lfc full url style test
        test_stat_all( "LFC_FULL" ${lfc_full_prefix})
        test_mkdir_all("LFC_FULL" ${lfc_full_prefix})
        test_chmod_all("LFC_FULL" ${lfc_valid_dir_root} 0565 060 0360 0767)
        test_rmdir_all("LFC_FULL" ${lfc_full_valid_dir_root})
        test_readdir_full("LFC_FULL" ${lfc_full_valid_dir_root} )
        # Register
        test_register("LFC" ${gsiftp_prefix_dpm} ${lfc_full_prefix})
ENDIF(PLUGIN_LFC)


IF(PLUGIN_DCAP)
        test_stat_all( "DCAP" ${dcap_prefix})

        test_mkdir_all("DCAP" ${dcap_prefix})
        # disable not supported by most of the server, impossible to test
        # test_chmod_all("DCAP" ${dcap_prefix} 0565 000 0320 0767)
        # disable not supported by most of the server, impossible to test
        #test_readdir_full("DCAP" ${dcap_valid_dir_root} )      
        #test_rmdir_all("DCAP" ${dcap_valid_dir_root})
        test_rwt_all("DCAP" ${dcap_valid_dir_root} 4578)        
        test_rwt_seq("DCAP" ${dcap_valid_dir_root} 100 4560)    
ENDIF(PLUGIN_DCAP)


IF(PLUGIN_GRIDFTP)
        test_del_nonex("GRIDFTP_DPM"  "${gsiftp_prefix_dpm}")
        test_del("GRIDFTP_DPM"   "${gsiftp_prefix_dpm}")
        test_mkdir_unlink("GRIDFTP" "${gsiftp_prefix_dpm}")
        test_rename("GRIDFTP" ${gsiftp_prefix_dpm})
        test_stat_all( "GRIDFTP" ${gsiftp_valid_dir_root})
        test_access("GRIDFTP" ${gsiftp_valid_dir_root})
        test_checksum_simple("GRIDFTP_ADLER32" ${gsiftp_valid_dir_root} ADLER32)
        test_checksum_simple("GRIDFTP_MD5" ${gsiftp_valid_dir_root} MD5)
        test_checksum_simple("GRIDFTP_CRC32" ${gsiftp_valid_dir_root} CRC32)
        test_mkdir_all("GRIDFTP" ${gsiftp_prefix_dpm})  
        test_chmod_all("GRIDFTP" ${gsiftp_prefix_dpm} 0565 060 0360 0767)
        test_rmdir_all("GRIDFTP" ${gsiftp_valid_dir_root})
        test_readdir_full("GRIDFTP" ${gsiftp_valid_dir_root})
        test_rwt_all("GRIDFTP" ${gsiftp_valid_dir_root} 4578)
        test_rwt_all("GRIDFTP_single" ${gsiftp_valid_dir_root} 1)               
        test_rwt_seq("GRIDFTP" ${gsiftp_valid_dir_root} 100 4560)
        test_rwt_seq("GRIDFTP_unit" ${gsiftp_valid_dir_root} 1 10)
        test_rwt_seek("GRIDFTP" ${gsiftp_valid_dir_root} 100 4560)

        test_stat_all("FTP" ${ftp_prefix})
ENDIF(PLUGIN_GRIDFTP)

IF(PLUGIN_HTTP)
        test_del_nonex("DAVS_DPM" ${davs_valid_dir_root})
        test_del("DAVS_DPM" ${davs_valid_dir_root})
        test_mkdir_unlink("DAVS_DPM" ${davs_valid_dir_root})
        test_stat_all("DAVS" ${davs_valid_dir_root})
        test_checksum_simple("DAVS_ADLER32" ${davs_valid_dir_root} ADLER32)
        test_checksum_simple("DAVS_MD5"     ${davs_valid_dir_root} MD5)
        test_checksum_simple("DAVS_CRC32"   ${davs_valid_dir_root} CRC32)
        test_mkdir_all("DAVS" ${davs_valid_dir_root})
        test_rename("DAVS" ${davs_valid_dir_root})
        # chmod not supported
        test_rmdir_all("DAVS" ${davs_valid_dir_root})
        test_readdir_full("DAVS" ${davs_valid_dir_root})
        test_rwt_all("DAVS" ${davs_valid_dir_root} 4578)
        test_rwt_all("DAVS_single" ${davs_valid_dir_root} 1)
        # sequencial writes not supported
ENDIF(PLUGIN_HTTP)

IF(PLUGIN_XROOTD)
    test_del_nonex("XROOTD_DPM" ${root_valid_dir_root})
    test_del("XROOTD_DPM" ${root_valid_dir_root})
    test_mkdir_unlink("XROOTD_DPM" ${root_valid_dir_root})
    test_stat_all("XROOTD" ${root_valid_dir_root})
    test_access("XROOTD" ${root_valid_dir_root})
    test_rename("XROOTD" ${root_valid_dir_root})
    # Checksum not supported yet in the XrdCl library
    # Chmod does not work in posix-style, so the test can not be used
    test_mkdir_all("XROOTD" ${root_valid_dir_root})
    test_rmdir_all("XROOTD" ${root_valid_dir_root})
    test_readdir_full("XROOTD" ${root_valid_dir_root})
    test_rwt_all("XROOTD" ${root_valid_dir_root} 4578)
    test_rwt_all("XROOTD_single" ${root_valid_dir_root} 1)
    test_rwt_seq("XROOTD" ${root_valid_dir_root} 100 4560)
    test_rwt_seq("XROOTD_single" ${root_valid_dir_root} 1 10)

    # Copies
    IF (MAIN_TRANSFER)
        test_copy_file_no_checksum("XROOTD" ${root_valid_dir_root} ${root_valid_dir_root})
    ENDIF (MAIN_TRANSFER)
ENDIF()

IF (MAIN_TRANSFER)
        test_copy_file_full("GRIDFTP_DPM"               ${gsiftp_prefix_dpm} ${gsiftp_prefix_dpm})   
        test_copy_file_full("SRM_DPM"                   ${srm_valid_dir_root} ${srm_valid_dir_root})
        test_copy_file_full("SRM_DPM_TO_DCACHE"         ${srm_valid_dir_root} ${srm_valid_dcache_dir_root})
        test_copy_file_full("SRM_DCACHE_TO_SRM"         ${srm_valid_dcache_dir_root} ${srm_valid_dir_root})
        test_copy_file_full("GSIFTP_DPM_TO_SRM_DCACHE"  ${gsiftp_prefix_dpm} ${srm_valid_dcache_dir_root})
        test_copy_file_full("SRM_DCACHE"                ${srm_valid_dcache_dir_root} ${srm_valid_dcache_dir_root})
        test_copy_file_full("SRM_TO_GRIDFTP"            ${srm_valid_dir_root} ${gsiftp_prefix_dpm})
        test_copy_file_full("GRIDFTP_TO_SRM"            ${gsiftp_prefix_dpm} ${srm_valid_dir_root})
        test_copy_file_full("DAVS_TO_DAVS"              ${davs_valid_dir_root} ${davs_valid_dir_root})

        # global transfer tests for storage compatibility

        # storm <-> storm
        test_copy_file_full("STORM_TO_STORM" ${srm_prefix_storm}  ${srm_prefix_storm})
        # storm -> dpm
        test_copy_file_full("STORM_TO_SRM_DPM" ${srm_prefix_storm}  ${srm_valid_dir_root})
        # storm -> dcache
        test_copy_file_full("STORM_TO_SRM_DCACHE" ${srm_prefix_storm}  ${srm_valid_dcache_dir_root})

        # local transfer
        #

        # local <-> DPM
        test_copy_file_simple("FILE_TO_SRM_DPM" ${file_prefix} ${srm_valid_dir_root})
        test_copy_file_simple("SRM_DPM_TO_FILE" ${srm_valid_dir_root}  ${file_prefix})

        # local <-> dcache
        test_copy_file_simple("FILE_TO_SRM_DCACHE" ${file_prefix} ${srm_valid_dcache_dir_root})
        test_copy_file_simple("SRM_DCACHE_TO_FILE" ${srm_valid_dcache_dir_root}  ${file_prefix})
        test_copy_file_simple("FILE_TO_FILE" ${file_prefix}  ${file_prefix})

        # gsiftp dpm <-> local
        test_copy_file_simple("GSIFTP_TO_FILE" ${gsiftp_prefix_dpm}  ${file_prefix})
        test_copy_file_simple("FILE_TO_GSIFTP" ${file_prefix}  ${gsiftp_prefix_dpm})

        # local <-> storm
        test_copy_file_simple("STORM_TO_FILE" ${srm_prefix_storm}  ${file_prefix})
        test_copy_file_simple("FILE_TO_STORM" ${file_prefix}  ${srm_prefix_storm})

        # Passive plugin, which only makes sense for GridFTP
        test_pasv("PASV" ${gsiftp_prefix_dpm} ${gsiftp_prefix_dpm})

ENDIF (MAIN_TRANSFER)
