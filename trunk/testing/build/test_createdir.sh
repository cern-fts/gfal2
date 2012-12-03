#!/bin/bash
sfile=lfn:/grid/dteam/testcreatedir		# dir tested
./gfal_testcreatedir $sfile &> /dev/null	# must be a success
if [[ "$?" != "0" ]]; then
	exit -1
fi
