#!/bin/bash
export GFAL_CONFIG_DIR=`pwd`/test/conf_test/


## export etics needed configs 


echo "## print environment"
env
echo "## end print environment"

$@
