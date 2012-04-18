#
# compilation lines for examples

## STAT Tests

SET(srm_prefix_dpm "srm://cvitbdpm1.cern.ch/dpm/cern.ch/home/dteam/gfal2-tests/")
SET(srm_valid_dpm_stat "${srm_prefix_dpm}/testread0011")

SET(srm_prefix_dcache "srm://cork.desy.de:8443/srm/managerv2?SFN=/pnfs/desy.de/data/dteam/testgfal2")
SET(srm_valid_dcache_stat "${srm_prefix_dcache}/testread0011")

#SET(srm_prefix_EOS "srm://srm.pic.es/pnfs/pic.es/data/dteam")
#SET(srm_valid_EOS_stat "${srm_prefix_castor}/testread0011")

SET(lfc_prefix "lfn:/grid/dteam/")
SET(lfc_stat_ok "${lfc_prefix}/testread0011")

SET(dcap_prefix "dcap://cork.desy.de/pnfs/desy.de/data/dteam/testgfal2")
SET(dcap_stat_ok "${dcap_prefix}/testread0011")

IF(PLUGIN_SRM)
	stat_test_all( "SRM_DPM" ${srm_valid_dpm_stat})
	stat_test_all( "SRM_DCACHE" ${srm_valid_dcache_stat})
#	stat_test_all( "SRM_EOS" ${srm_valid_EOS_stat})
ENDIF(PLUGIN_SRM)

IF(PLUGIN_LFC)
	stat_test_all( "LFC" ${lfc_stat_ok})
ENDIF(PLUGIN_LFC)


IF(PLUGIN_DCAP)
	stat_test_all( "DCAP" ${dcap_stat_ok})
ENDIF(PLUGIN_DCAP)
