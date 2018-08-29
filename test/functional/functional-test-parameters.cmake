#
# compilation lines for test parameters

## STAT Tests

SET(MY_VO "dteam")
SET(MY_VO_STORM "${MY_VO}")
SET(TEST_ENVIRONMENT "PRODUCTION"
        CACHE STRING "Define the target for functional test")

## Global environment
SET(ftp_prefix "ftp://mirror.switch.ch/mirror/centos/")

IF(TEST_ENVIRONMENT STREQUAL "TESTBED_RC")

## Testbed environment
SET(file_base_path "/tmp/")
SET(srm_prefix_storm "srm://se01.esc.qmul.ac.uk:8444/srm/managerv2?SFN=/${MY_VO_STORM}/gfal2-tests/")
SET(srm_prefix_dcache "srm://dcache-se-cms.desy.de:8443/srm/managerv2?SFN=/pnfs/desy.de/${MY_VO}/gfal2-tests/")
SET(srm_prefix_castor "srm://srm-public.cern.ch:8443/srm/managerv2?SFN=/castor/cern.ch/grid/${MY_VO}/gfal2-tests/")
SET(lfc_prefix "lfn:/grid/${MY_VO}/gfal2-tests/")
SET(lfc_host_name "lfc-puppet01.cern.ch")
SET(gsiftp_prefix_dpm "gsiftp://dpmhead-rc.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(srm_prefix_dpm "srm://dpmhead-rc.cern.ch:8446/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(davs_prefix_dpm "davs://dpmhead-rc.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(davs_prefix_dcache "davs://prometheus.desy.de/VOs/${MY_VO}/gfal2-tests")
SET(root_prefix_dpm "root://dpmhead-rc.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(root_prefix_eos "root://eospps.cern.ch//eos/opstest/gfal2-tests")
SET(sftp_prefix "sftp://gfal2@arioch.cern.ch/home/gfal2/gfal2-tests")

ELSEIF(TEST_ENVIRONMENT STREQUAL "TESTBED_TRUNK")

## Testbed trunk environment
SET(file_base_path "/tmp/")
SET(srm_prefix_storm "srm://se01.esc.qmul.ac.uk:8444/srm/managerv2?SFN=/${MY_VO_STORM}/gfal2-tests/")
SET(srm_prefix_dcache "srm://vm-dcache-deploy6.desy.de:8443/data/${MY_VO}/gfal2-tests")
SET(srm_prefix_castor "srm://srm-public.cern.ch:8443/srm/managerv2?SFN=/castor/cern.ch/grid/${MY_VO}/gfal2-tests/")
SET(lfc_prefix "lfn:/grid/${MY_VO}/gfal2-tests/")
SET(lfc_host_name "lfc-puppet01.cern.ch")
SET(gsiftp_prefix_dpm "gsiftp://dpmhead-trunk.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(srm_prefix_dpm "srm://dpmhead-trunk.cern.ch:8446/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(davs_prefix_dpm "davs://dpmhead-trunk.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(davs_prefix_dcache "davs://prometheus.desy.de/VOs/${MY_VO}/gfal2-tests")
SET(root_prefix_dpm "root://dpmhead-trunk.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(root_prefix_eos "root://eospps.cern.ch//eos/opstest/tpc/gfal2-tests")
SET(sftp_prefix "sftp://gfal2@arioch.cern.ch/home/gfal2/gfal2-tests")

ELSE(TEST_ENVIRONMENT STREQUAL "TESTBED_RC")

## Production environment : default
SET(file_base_path "/tmp/")
SET(srm_prefix_storm "srm://se01.esc.qmul.ac.uk:8444/srm/managerv2?SFN=/${MY_VO_STORM}/gfal2-tests/")
SET(srm_prefix_dcache "srm://dcache-se-cms.desy.de:8443/srm/managerv2?SFN=/pnfs/desy.de/${MY_VO}/gfal2-tests/")
SET(srm_prefix_castor "srm://srm-public.cern.ch:8443/srm/managerv2?SFN=/castor/cern.ch/grid/${MY_VO}/gfal2-tests/")
SET(lfc_prefix "lfn:/grid/${MY_VO}/gfal2-tests/")
SET(lfc_host_name "lfc-puppet01.cern.ch")
SET(gsiftp_prefix_dpm "gsiftp://t2-dpm-01.na.infn.it/dpm/na.infn.it/home/${MY_VO}/gfal2-tests")
SET(srm_prefix_dpm "srm://ipnsedpm.in2p3.fr:8446/dpm/in2p3.fr/home/${MY_VO}/gfal2-tests/")
SET(sftp_prefix "sftp://gfal2@arioch.cern.ch/home/gfal2/gfal2-tests")
SET(root_prefix_dpm "root://dpmhead-rc.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(root_prefix_eos "root://eospps.cern.ch//eos/opstest/tpc/gfal2-tests")
# Need to find something better!
SET(davs_prefix_dpm "davs://dpmhead-rc.cern.ch/dpm/cern.ch/home/${MY_VO}/gfal2-tests")
SET(davs_prefix_dcache "davs://prometheus.desy.de/VOs/${MY_VO}/gfal2-tests")

ENDIF(TEST_ENVIRONMENT STREQUAL "TESTBED_RC")

## lfc parameters
SET(lfc_full_prefix "lfc://${lfc_host_name}/grid/${MY_VO}")

## local file parameters
SET(file_prefix "file://${file_base_path}")

IF(PLUGIN_FILE)
        test_del("FILE" "${file_prefix}")
        test_rename("FILE" "${file_prefix}")
        test_access("FILE" "${file_prefix}")
        test_stat_all("FILE" ${file_prefix})
        test_chmod_all("FILE" ${file_prefix} 0565 060 360 767)
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
        test_del("SRM_DPM"     "${srm_prefix_dpm}")
        test_del("SRM_STORM"   "${srm_prefix_storm}")
        test_del("SRM_DCACHE"  "${srm_prefix_dcache}")
        test_del("SRM_CASTOR"  "${srm_prefix_castor}")
        # stat tests
        test_stat_all("SRM_DPM" ${srm_prefix_dpm})
        test_stat_all("SRM_DCACHE" ${srm_prefix_dcache})
        test_stat_all("SRM_CASTOR" ${srm_prefix_castor})
        test_access("SRM_DPM" ${srm_prefix_dpm})
        # dCache says Operation not supported
        # test_access("SRM_DCACHE" ${srm_prefix_dcache})
        # Rename test
        test_rename("SRM_DPM" ${srm_prefix_dpm})
        test_rename("SRM_DCACHE" ${srm_prefix_dcache})
        test_rename("SRM_CASTOR" ${srm_prefix_castor})
        # checksum tests
        test_checksum_simple("SRM_DPM_ADLER32" ${srm_prefix_dpm} ADLER32)
        test_checksum_simple("SRM_DPM_MD5" ${srm_prefix_dpm} MD5)
        test_checksum_simple("SRM_DCACHE_ADLER32" ${srm_prefix_dcache} ADLER32)
        test_checksum_simple("SRM_CASTOR_ADLER32" ${srm_prefix_castor} ADLER32)
        # Mkdir
        test_mkdir_all("SRM_DPM" ${srm_prefix_dpm})
        test_mkdir_all("SRM_DCACHE" ${srm_prefix_dcache})
        test_mkdir_all("SRM_CASTOR" ${srm_prefix_castor})

        test_chmod_all("SRM_DPM" ${srm_prefix_dpm} 0575 070 370 777)
        test_rmdir_all("SRM_DPM" ${srm_prefix_dpm})

        test_readdir_full("SRM_DPM" ${srm_prefix_dpm} )

        test_rwt_all("SRM_DPM" ${srm_prefix_dpm} 4578)
        test_rwt_all("SRM_DPM_little" ${srm_prefix_dpm} 10)
        test_rwt_all("SRM_DPM_single" ${srm_prefix_dpm} 1)
        test_rwt_all("SRM_DCAP" ${srm_prefix_dcache} 4578)
        test_rwt_all("SRM_DCAP_little" ${srm_prefix_dcache} 10)
        test_rwt_all("SRM_DCAP_single" ${srm_prefix_dcache} 1)
        test_rwt_all("SRM_CASTOR" ${srm_prefix_castor} 4578)
        test_rwt_all("SRM_CASTOR_little" ${srm_prefix_castor} 10)
        test_rwt_all("SRM_CASTOR_single" ${srm_prefix_castor} 1)
        test_rwt_seq("SRM_DPM" ${srm_prefix_dpm} 100 4560)
        test_rwt_seq("SRM_DPM_unit" ${srm_prefix_dpm} 1 10)
        test_rwt_seq("SRM_DCAP" ${srm_prefix_dpm} 100 4560)

        test_rwt_seq("SRM_STORM" ${srm_prefix_storm} 100 4560)

        # Bringonline
        test_bringonline("SRM_DPM" ${srm_prefix_dpm})
        test_bringonline("SRM_DCACHE" ${srm_prefix_dcache})
        test_bringonline("SRM_CASTOR" ${srm_prefix_castor})
        # Note: STORM may not support tapes

#       test_chmod_all("SRM_DCACHE" ${srm_valid_dcache_chmod} 0565 060 360 767)  -> disabled, since unavailable on dcache
#       test_stat_all( "SRM_EOS" ${srm_valid_EOS_stat})

        test_xattr("DPM" ${srm_prefix_dpm})
        test_xattr("DCACHE" ${srm_prefix_dcache})
        test_xattr("CASTOR" ${srm_prefix_castor})

        test_space("DPM" ${srm_prefix_dpm})

        test_rd3_reorder_protocols("GRIDFTP_TO_SRM" ${gsiftp_prefix_dpm} ${srm_prefix_dcache})
        test_rd3_reorder_protocols("SRM_TO_GRIDFTP" ${srm_prefix_dcache} ${gsiftp_prefix_dpm})
        test_rd3_reorder_protocols("SRM_TO_SRM" ${srm_prefix_dcache} ${srm_prefix_dcache})
        test_rd3_reorder_protocols("XROOTD_TO_SRM" ${root_prefix_dpm} ${srm_prefix_dcache})

ENDIF(PLUGIN_SRM)

IF(PLUGIN_LFC)
        test_stat_all( "LFC" ${lfc_prefix})
        test_mkdir_all("LFC" ${lfc_prefix})
        test_chmod_all("LFC" ${lfc_prefix} 0565 060 0360 0767)
        test_rmdir_all("LFC" ${lfc_prefix})
        test_readdir_full("LFC" ${lfc_prefix} )
        # lfc full url style test
        test_stat_all( "LFC_FULL" ${lfc_full_prefix})
        test_mkdir_all("LFC_FULL" ${lfc_full_prefix})
        test_chmod_all("LFC_FULL" ${lfc_prefix} 0565 060 0360 0767)
        test_rmdir_all("LFC_FULL" ${lfc_full_prefix})
        test_readdir_full("LFC_FULL" ${lfc_full_prefix} )
        # Register
        test_register("LFC" ${gsiftp_prefix_dpm} ${lfc_full_prefix})
        # Xattr
        test_xattr("LFC" ${lfc_full_prefix} ${gsiftp_prefix_dpm})
ENDIF(PLUGIN_LFC)

IF(PLUGIN_GRIDFTP)
        test_del("GRIDFTP_DPM"   "${gsiftp_prefix_dpm}")
        test_rename("GRIDFTP_DPM" ${gsiftp_prefix_dpm})
        test_stat_all("GRIDFTP_DPM" ${gsiftp_prefix_dpm})
        test_access("GRIDFTP_DPM" ${gsiftp_prefix_dpm})
        test_checksum_simple("GRIDFTP_DPM__ADLER32" ${gsiftp_prefix_dpm} ADLER32)
        test_checksum_simple("GRIDFTP_DPM_MD5" ${gsiftp_prefix_dpm} MD5)
        test_checksum_simple("GRIDFTP_DPM_CRC32" ${gsiftp_prefix_dpm} CRC32)
        test_mkdir_all("GRIDFTP_DPM" ${gsiftp_prefix_dpm})
        test_chmod_all("GRIDFTP_DPM" ${gsiftp_prefix_dpm} 0565 060 0360 0767)
        test_rmdir_all("GRIDFTP_DPM" ${gsiftp_prefix_dpm})
        test_readdir_full("GRIDFTP_DPM" ${gsiftp_prefix_dpm})
        test_rwt_all("GRIDFTP_DPM" ${gsiftp_prefix_dpm} 4578)
        test_rwt_all("GRIDFTP_DPM_single" ${gsiftp_prefix_dpm} 1)
        test_rwt_seq("GRIDFTP_DPM" ${gsiftp_prefix_dpm} 100 4560)
        test_rwt_seq("GRIDFTP_DPM_unit" ${gsiftp_prefix_dpm} 1 10)
        test_rwt_seek("GRIDFTP_DPM" ${gsiftp_prefix_dpm} 100 4560)
        test_space("GRIDFTP_DPM" ${gsiftp_prefix_dpm})

        test_stat_all("FTP" ${ftp_prefix})
ENDIF(PLUGIN_GRIDFTP)

IF(PLUGIN_HTTP)
        test_del("DAVS_DPM" ${davs_prefix_dpm})
        test_del("DAVS_DCACHE" ${davs_prefix_dcache})
        test_stat_all("DAVS_DPM" ${davs_prefix_dpm})
        test_stat_all("DAVS_DCACHE" ${davs_prefix_dcache})
        test_checksum_simple("DAVS_DPM_ADLER32" ${davs_prefix_dpm} ADLER32)
        test_checksum_simple("DAVS_DACHE_ADLER32" ${davs_prefix_dcache} ADLER32)
        test_checksum_simple("DAVS_DPM_MD5"     ${davs_prefix_dpm} MD5)
        test_checksum_simple("DAVS_DCACHE_MD5"  ${davs_prefix_dcache} MD5)
        test_checksum_simple("DAVS_DPM_CRC32"   ${davs_prefix_dpm} CRC32)
        test_checksum_simple("DAVS_DCACHE_CRC32"   ${davs_prefix_dcache} CRC32)
        test_mkdir_all("DAVS_DPM" ${davs_prefix_dpm})
        test_mkdir_all("DAVS_DCACHE" ${davs_prefix_dcache})
        test_rename("DAVS_DPM" ${davs_prefix_dpm})
        test_rename("DAVS_DCACHE" ${davs_prefix_dcache})
        # chmod not supported
        test_rmdir_all("DAVS_DPM" ${davs_prefix_dpm})
        test_rmdir_all("DAVS_DCACHE" ${davs_prefix_dcache})
        test_readdir_full("DAVS_DPM" ${davs_prefix_dpm})
        test_readdir_full("DAVS_DCACHE" ${davs_prefix_dcache})
        test_rwt_all("DAVS_DPM" ${davs_prefix_dpm} 4578)
        test_rwt_all("DAVS_DCACHE" ${davs_prefix_dcache} 4578)
        test_rwt_all("DAVS_DPM_single" ${davs_prefix_dpm} 1)
        test_rwt_all("DAVS_DCACHE_single" ${davs_prefix_dcache} 1)
        # sequencial writes not supported
ENDIF(PLUGIN_HTTP)

IF(PLUGIN_XROOTD)
    test_del("XROOTD_DPM" ${root_prefix_dpm})
    test_del("XROOTD_EOS" ${root_prefix_eos})
    test_stat_all("XROOTD_DPM" ${root_prefix_dpm})
    test_stat_all("XROOTD_EOS" ${root_prefix_eos})
    test_access("XROOTD_DPM" ${root_prefix_dpm})
    test_access("XROOTD_EOS" ${root_prefix_eos})
    test_rename("XROOTD_DPM" ${root_prefix_dpm})
    test_rename("XROOTD_EOS" ${root_prefix_eos})
    # Checksum not supported yet in the XrdCl library
    # Chmod does not work in posix-style, so the test can not be used
    test_mkdir_all("XROOTD_DPM" ${root_prefix_dpm})
    test_mkdir_all("XROOTD_EOS" ${root_prefix_eos})
    test_rmdir_all("XROOTD_DPM" ${root_prefix_dpm})
    test_rmdir_all("XROOTD_EOS" ${root_prefix_eos})
    test_readdir_full("XROOTD_DPM" ${root_prefix_dpm})
    test_readdir_full("XROOTD_EOS" ${root_prefix_eos})
    test_rwt_all("XROOTD_DPM" ${root_prefix_dpm} 4578)
    test_rwt_all("XROOTD_EOS" ${root_prefix_eos} 4578)
    test_rwt_all("XROOTD_DPM_single" ${root_prefix_dpm} 1)
    test_rwt_all("XROOTD_EOS_single" ${root_prefix_eos} 1)
    test_rwt_seq("XROOTD_DPM" ${root_prefix_dpm} 100 4560)
    test_rwt_seq("XROOTD_EOS" ${root_prefix_eos} 100 4560)
    test_rwt_seq("XROOTD_DPM_single" ${root_prefix_dpm} 1 10)
    test_rwt_seq("XROOTD_EOS_single" ${root_prefix_eos} 1 10)
    test_rwt_seek("XROOTD_DPM" ${root_prefix_dpm} 100 4560)
    test_rwt_seek("XROOTD_EOS" ${root_prefix_eos} 100 4560)

    test_space("XROOTD_DPM" ${root_prefix_dpm})
    test_space("XROOTD_EOS" ${root_prefix_eos})

    # Copies
    IF (MAIN_TRANSFER)
        test_copy_file_no_checksum("XROOTD_DPM" ${root_prefix_dpm} ${root_prefix_dpm})
        test_copy_file_no_checksum("XROOTD_EOS" ${root_prefix_eos} ${root_prefix_eos})
        test_copy_bulk("XROOTD_DPM" ${root_prefix_dpm} ${root_prefix_dpm})
        test_copy_bulk("XROOTD_EOS" ${root_prefix_eos} ${root_prefix_eos})
    ENDIF (MAIN_TRANSFER)
ENDIF()

#IF (PLUGIN_SFTP)
#    test_del("SFTP" "${sftp_prefix}")
#    test_rename("SFTP" "${sftp_prefix}")
#    test_stat_all("SFTP" "${sftp_prefix}")
#    test_chmod_all("SFTP" "${sftp_prefix}" 0565 060 360 767)
#    test_mkdir_all("SFTP" "${sftp_prefix}")
#    test_rmdir_all("SFTP" "${sftp_prefix}")
#    test_readdir_full("SFTP" "${sftp_prefix}")
#    test_rwt_all("SFTP" "${sftp_prefix}" 4578)
#    test_rwt_all("SFTP" "${sftp_prefix}" 1)
#    test_rwt_all("SFTP" "${sftp_prefix}" 100000)
#    test_rwt_seq("SFTP" "${sftp_prefix}" 100 4560)
#    test_rwt_seek("SFTP" "${sftp_prefix}" 100 4560)
#ENDIF ()

IF (MAIN_TRANSFER)
        test_copy_file_full("GRIDFTP_TO_GRIDFTP"        ${gsiftp_prefix_dpm} ${gsiftp_prefix_dpm})
        test_copy_file_full("SRM_DPM_TO_DCACHE"         ${srm_prefix_dpm} ${srm_prefix_dcache})
        test_copy_file_full("SRM_DCACHE_TO_DPM"         ${srm_prefix_dcache} ${srm_prefix_dpm})
        test_copy_file_full("GRIDFTP_TO_SRM"            ${gsiftp_prefix_dpm} ${srm_prefix_dcache})
        test_copy_file_full("SRM_TO_GRIDFTP"            ${srm_prefix_dpm} ${gsiftp_prefix_dpm})
        test_copy_file_full("DAVS_TO_DAVS"              ${davs_prefix_dpm} ${davs_prefix_dpm})

        test_copy_file_full("FILE_TO_SRM"               ${file_prefix} ${srm_prefix_dpm})
        test_copy_file_full("SRM_TO_FILE"               ${srm_prefix_dcache}  ${file_prefix})
        test_copy_file_full("FILE_TO_FILE"              ${file_prefix}  ${file_prefix})
        test_copy_file_full("GRIDFTP_TO_FILE"           ${gsiftp_prefix_dpm}  ${file_prefix})
        test_copy_file_full("FILE_TO_GRIDFTP"           ${file_prefix}  ${gsiftp_prefix_dpm})

        test_copy_file_full("STORM_TO_STORM"            ${srm_prefix_storm}  ${srm_prefix_storm})
        test_copy_file_full("STORM_TO_SRM_DPM"          ${srm_prefix_storm}  ${srm_prefix_dpm})

        test_copy_file_full("SRM_DPM_TO_CASTOR"         ${srm_prefix_dpm} ${srm_prefix_castor})
        test_copy_file_full("SRM_CASTOR_TO_DPM"         ${srm_prefix_castor} ${srm_prefix_dpm})

        # particular cases, storm to gridftp (i.e. can't reuse turl)
        test_copy_file_full("STORM_TO_GRIDFTP"          ${srm_prefix_storm} ${gsiftp_prefix_dpm})
        test_copy_file_full("GRIDFTP_TO_STORM"          ${gsiftp_prefix_dpm} ${srm_prefix_storm})

        # bulk, only a subset, otherwise this takes too long
        test_copy_bulk("GSIFTP" ${gsiftp_prefix_dpm} ${gsiftp_prefix_dpm})
        test_copy_bulk("SRM" ${srm_prefix_dpm} ${srm_prefix_dcache})

        # Passive plugin, which only makes sense for GridFTP
        test_pasv("PASV" ${gsiftp_prefix_dpm} ${gsiftp_prefix_dpm})

        # Same for double credentials
        test_double_cred("GSIFTP" ${gsiftp_prefix_dpm} ${gsiftp_prefix_dpm})

ENDIF (MAIN_TRANSFER)
