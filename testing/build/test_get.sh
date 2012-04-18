#!/bin/bash
sfile=srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/testfile002
lcg-del -v -l -D srmv2 $sfile > /dev/null &> /dev/null
touch /tmp/hello
echo "Hello World" > /tmp/hello
lcg-cp /tmp/hello $sfile > /dev/null
./gfal_testget $sfile 
if [[ "$?" != "0" ]]; then
	exit -1
fi
lcg-del -v -l -D srmv2 $sfile &> /dev/null									# delete the useless file
