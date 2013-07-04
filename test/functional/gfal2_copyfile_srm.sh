#!/bin/bash
# test for the third party transfer
#


source `dirname $0`/gfal2_config_test.sh

export PATH=`dirname $0`:$PATH


tmp_filename
SRM_DPM_SRC=${SRM_PREFIX_DPM}/${GENERIC_VALID_FILENAME}
SRM_DPM_DST=${SRM_PREFIX_DPM}/$TMP_FILENAME
tmp_filename
SRM_DPM_SRCENOENT=${SRM_PREFIX_DPM}/testnonvalidfilejexistepas
SRM_DPM_DSTENOENT=${SRM_PREFIX_DPM}/$TMP_FILENAME

function gfal_copyfile_exec {
	echo "gfalcopy from $1 to $2"
	gfal_copyfile $1 $2

}

# valid gridftp copy on dpm
set -e
gfal_copyfile_exec ${SRM_DPM_SRC} ${SRM_DPM_DST}
set +e

# enoent copy on dpm, must be a errno = 2
res=$(gfal_copyfile_exec ${SRM_DPM_SRCENOENT} ${SRM_DPM_DSTENOENT})
enoent_res=$(echo $res | grep "2")
if [[ "$enoent_res" == "" ]]; then
	echo " bad error code displayed : $res"
	exit 127
fi
exit 0
