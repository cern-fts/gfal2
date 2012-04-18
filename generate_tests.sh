#/bin/bash
##
#
test_list=`grep -v "//" test/unit_test_cgreen.c  | grep -E "add_test(.*)" | sed "s/.*add_test(.*, \(.*\));/\1/g"`

test_script_dir=`dirname $0`/dist/usr/share/gfal2/tests/mocked

BDII_ENDPOINT="certtb-bdii-top.cern.ch:2170"
LFC_ENDPOINT="cvitblfc1.cern.ch"

for i in $test_list
do
filename="$test_script_dir/$(echo "$i" | sed -e 's/_/-/g')"
echo "#!/bin/bash " > $filename
echo "source /etc/profile" >> $filename
echo "export LCG_GFAL_INFOSYS=$BDII_ENDPOINT" >> $filename
echo "export LFC_HOST=$LFC_ENDPOINT" >> $filename
echo "export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64/:$LD_LIBRARY_PATH" >> $filename
echo "\`dirname \$0\`/test_verbose $i" >> $filename
chmod 775 $filename
done
