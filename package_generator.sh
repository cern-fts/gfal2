#!/bin/bash

package_list=(main_core main_devel main_meta main_doc plugin_devel plugin_dcap plugin_srm plugin_lfc plugin_rfio main_tests)

if [ -z "$SCONS_EXE" ]
then
	SCONS_EXE="scons"
fi


mkdir -p RPMS
for i in ${package_list[@]}
do
$SCONS_EXE -j 8 $i=yes "$@" package_generator
if [ "$?" != "0" ]
  then
	exit -1
fi
rm *.tar.gz
rm *.spec
mv *.rpm RPMS/
done

