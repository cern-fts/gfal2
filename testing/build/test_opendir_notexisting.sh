#!/bin/bash
sfile=lfn:/nothing	 # try to open a missing dir
./gfal_testdir $sfile &> /dev/null	# must fail
if [[ "$?" == "0" ]]; then
	exit -1
fi
