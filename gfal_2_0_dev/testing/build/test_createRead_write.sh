#!/bin/bash
sfile=srm://grid-cert-03.roma1.infn.it/dpm/roma1.infn.it/home/dteam/generated/testfile001
lcg-del -v -l -D srmv2 $sfile > /dev/null &> /dev/null
./gfal_testrw $sfile > /dev/null
