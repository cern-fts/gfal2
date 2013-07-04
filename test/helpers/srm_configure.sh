#!/bin/bash
## script to an auto configuration of a dpm storage element for the srm unit test
#
echo "$1"
if [ ["$1" = ""] -o ["$2" = ""] ]; then
	echo "Invalid Usage"
	echo "	$0 HOST PATH ...."
	exit -1
fi

export DPNS_HOST=$1

function create_on_grid {
	lcg-cp file:///etc/group "srm://$1:8446/$2" 
}
function change_right {
	dpns-chmod $1 $2
}

function create_fixed_size_file_on_grid {
	rm /tmp/tmptest &> /dev/null
	dd if=/dev/zero of=/tmp/tmptest bs=512 count=4 &> /dev/null
	lcg-cp file:///tmp/tmptest "srm://$1:8446/$2"
}

function create_directory {
	dpns-mkdir $1
}

TESTWRITE0011="$2/testwrite0011"
TESTNOREAD0011="$2/testnoread0011"
TESTNOWRITE0011="$2/testnowrite0011"
TESTSTAT0011="$2/teststat0011"
TESTUNACCESSMKDIR0011="$2/testunaccessmkdir0011"
TESTRMDIR0012="$2/testrmdir0012"
TESTOPEN0011="$2/testopen0011"
TESTOPEN0012="$TESTOPEN0011/testopen0012"

create_on_grid "$1" "$2/file75715ccc-1c54-4d18-8824-bdd3716a2b54" 
create_on_grid "$1" "$2/testread0011"
change_right 555 "$2/testread0011"
create_on_grid "$1" "$TESTWRITE0011"
change_right 666 "$TESTWRITE0011"
create_on_grid "$1" "$TESTNOREAD0011"
change_right 000 "$TESTNOREAD0011"
create_on_grid "$1" "$TESTNOWRITE0011"
change_right 555 "$TESTNOWRITE0011"
create_fixed_size_file_on_grid "$1" "$TESTSTAT0011"
change_right 664 "$TESTSTAT0011"
create_directory "$TESTUNACCESSMKDIR0011"
change_right 000 "$TESTUNACCESSMKDIR0011"
create_directory "$TESTRMDIR0012"
create_on_grid "$1" "$TESTRMDIR0012/testosef012"
create_directory "$TESTOPEN0011"
create_on_grid "$1" "$TESTOPEN0012"


