#!/bin/bash
sfile=srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/testfile002
lcg-del -v -l -D srmv2 $sfile &> /dev/null # delete the file if exist
r=$( ./gfal_testread $sfile 2>&1 )	# must fail
echo $r | grep "No such file" > /dev/null
if [[ "$?" != "0" ]]; then
	exit -1
fi
