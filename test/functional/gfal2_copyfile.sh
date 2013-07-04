#!/bin/bash
# test for the third party transfer
#


source `dirname $0`/gfal2_config_test.sh

export PATH=`dirname $0`:$PATH


tmp_filename
GRIDFTP_DPM_SRC=${GRIDFTP_PREFIX_DPM}/${GENERIC_VALID_FILENAME}
GRIDFTP_DPM_DST=${GRIDFTP_PREFIX_DPM}/$TMP_FILENAME
tmp_filename
GRIDFTP_DPM_SRCENOENT=${GRIDFTP_PREFIX_DPM}/testnonvalidfilejexistepas
GRIDFTP_DPM_DSTENOENT=${GRIDFTP_PREFIX_DPM}/$TMP_FILENAME
tmp_filename
GRIDFTP_DPM_SRCEACCESS=${GRIDFTP_PREFIX_DPM}/${GENERIC_NO_READACCESS_FILENAME}
GRIDFTP_DPM_DSTEACCESS=${GRIDFTP_PREFIX_DPM}/$TMP_FILENAME

function gfal_copyfile_exec {
	echo "gfalcopy from $1 to $2"
	gfal_copyfile $1 $2

}

# valid gridftp copy on dpm
set -e
gfal_copyfile_exec ${GRIDFTP_DPM_SRC} ${GRIDFTP_DPM_DST}
set +e

# enoent copy on dpm, must be a errno = 2
res=$(gfal_copyfile_exec ${GRIDFTP_DPM_SRCENOENT} ${GRIDFTP_DPM_DSTENOENT})
enoent_res=$(echo $res | grep "2")
if [[ "$enoent_res" == "" ]]; then
	echo " bad error code displayed : $res"
	exit 127
fi

# enoent copy on dpm, must be a eacess 
#gfal_copyfile_exec ${GRIDFTP_DPM_SRCEACCESS} ${GRIDFTP_DPM_DSTEACCESS}
#res=$()
#eaccess_res=$(echo $res | grep "2")
#if [[ "$eaccess_res" == "" ]]; then
#	echo " bad error code displayed : $res"
#	exit 127
#fi



exit 0
