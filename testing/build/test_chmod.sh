#!/bin/bash
orgfile="srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/filetest27"
sfile="/grid/dteam/filetest27"	# dir tested
lfnfile="lfn:$sfile"
right=755
lcg-del -v -D srmv2 $orgfile &> /dev/null					## del existing files
lcg-del -a $lfnfile &> /dev/null
touch /tmp/test														## create new file
echo "Hello World" > /tmp/test	
lcg-cp file:///tmp/test $orgfile	> /dev/null								# copy on grid and test
lcg-rf -l $lfnfile $orgfile		>/dev/null
./gfal_testchmod lfn:$sfile $right &> /dev/null	# must be a success
if [[ "$?" != "0" ]]; then
	exit -1
fi
lfc-ls -l $sfile | grep rwxr-xr-x > /dev/null				# test if rights are correct
if [[ "$?" != "0" ]]; then
	exit -2
fi
lcg-del -a $lfnfile &> /dev/null										# delete the useless file
