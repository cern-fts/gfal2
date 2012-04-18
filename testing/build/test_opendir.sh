#!/bin/bash
sfile=lfn:/grid/dteam	# dir tested
./gfal_testdir $sfile &> /dev/null	# must be a success
if [[ "$?" != "0" ]]; then
	exit -1
fi
