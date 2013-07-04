#!/bin/bash
# config files
#

#global config
export LFC_HOST=cvitblfc1.cern.ch
export LCG_GFAL_INFOSYS=certtb-bdii-top.cern.ch:2170
export GFAL_PLUGIN_LIST=libgfal_plugin_gridftp.so:libgfal_plugin_lfc.so:libgfal_plugin_srm.so:libgfal_plugin_rfio.so
export LD_LIBRARY_PATH=`pwd`/src:$LD_LIBRARY_PATH



function tmp_filename(){
	export TMP_FILENAME=$(mktemp -u test_gfal2_XXXXXXXXXXXXXXXXXXXXXXXXXXXXX)
}

export GENERIC_VALID_FILENAME=testread0011
export GENERIC_NO_READACCESS_FILENAME=testnoread0011

export GRIDFTP_PREFIX_DPM=gsiftp://cvitbdpm1.cern.ch/dpm/cern.ch/home/dteam/gfal2-tests/

export SRM_PREFIX_DPM=srm://cvitbdpm1.cern.ch/dpm/cern.ch/home/dteam/gfal2-tests/
