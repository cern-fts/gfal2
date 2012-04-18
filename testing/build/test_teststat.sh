#!/bin/bash
## simple test on an existing dir
edir="lfn:/grid/dteam"
./gfal_teststat $edir > /dev/null
if [ "$?" != "0" ]; then
	exit -1
fi
## test on missing dir
mdir="lfn:/grid/nonexistingdir59999"
r=$( ./gfal_teststat $mdir 2>&1 )
if [ "$?" == 0 ]; then
	exit -2
fi
if [[ $(echo $r | grep -c "No such file" ) != 1 ]]; then
	exit -3
fi
