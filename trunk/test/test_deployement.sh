#!/bin/bash
#
#

set -e

PATH_TESTS=/usr/tests/functional/gfal2
CERT_PATH=$HOME/user-cert.pem
KEY_PATH=$HOME/user-key.pem

echo "## start test deployement"

if [[ ! -a "$CERT_PATH" ]] || [[ ! -a "$KEY_PATH" ]]; then
echo "cert or private key does not exist ! at $CERT_PATH and $KEY_PATH, FATAL"
exit 1
fi


gfal_dir=$(dirname $0)/
gfal_dir=$(readlink -f "$gfal_dir/../")

echo "## gfal dir $gfal_dir "

echo "## create gfal2 test dir execution : "
gfal_test_dir=$(mktemp -td gfal2.XXXXXXXXXXXXXXXX)

echo "## gfal test dir $gfal_test_dir "
mkdir -p $gfal_test_dir/build
cd $gfal_test_dir/build
echo "## configure project ..."
cmake -D MAIN_TRANSFER=TRUE -D FUNCTIONAL_TESTS=TRUE -D UNIT_TESTS=TRUE -D CMAKE_INSTALL_PREFIX=/usr -D ONLY_TESTS=TRUE $gfal_dir

echo "## build tests..."
make

echo "## import test environment"
source $gfal_dir/setup_test_env_isolated.sh

echo "## test deployement .... "

n_test=$(ctest -N | grep  "[\\/|#][0-9]" | wc -l)

echo "## found $n_test tests to execute..."


echo "## creat tests..."
mkdir -p $PATH_TESTS

for i in `seq $n_test`}
do
test_name_core=$(ctest -N -I ${i},${i} | grep  "[\\/|#][0-9]" | sed 's/[ \t]*$//g' | sed 's/.*\ \(.*\)$/\1/g')
test_name="gfal2-ctest-${test_name_core}-$i"
echo " generate test : $test_name"
test_path=$PATH_TESTS/$test_name
touch $test_path
echo "#!/bin/bash" >> $test_path
echo "## test $test_name" >> $test_path
echo " " >> $test_path
echo "## test credential ..."  >> $test_path
echo "voms-proxy-info -all" >> $test_path
echo "## setup credential path  ..."  >> $test_path
echo "export X509_USER_KEY=$KEY_PATH" >> $test_path
echo "export X509_USER_CERT=$CERT_PATH" >> $test_path

echo "## print env ..."  >> $test_path
echo "env" >> $test_path
echo "## start test ..."  >> $test_path
echo "cd $gfal_test_dir/build " >> $test_path
echo "set -e" >> $test_path
echo "source $gfal_dir/setup_test_env_isolated.sh" >> $test_path
echo "ctest -V -I ${i},${i}" >> $test_path
chmod a+x $test_path
done
