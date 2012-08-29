#!/bin/bash
export GFAL_PLUGIN_DIR=`pwd`/plugins/
export GFAL_CONFIG_DIR=`pwd`/test/conf_test/
export LD_LIBRARY_PATH=`pwd`/src:$LD_LIBRARY_PATH
$@
