#!/bin/bash
export GFAL_CONFIG_DIR=`pwd`/test/conf_test/


## export etics needed configs 
if [ -f "/etc/grid-security/test-user/test_user_501_cert.pem" ]; then
export USER_CERT="/etc/grid-security/test-user/test_user_501_cert.pem"
export USER_KEY="/etc/grid-security/test-user/test_user_501_key.pem"
else
export USER_CERT="/root/user_certificates/test_user_501_cert.pem"
export USER_KEY="/root/user_certificates/test_user_501_key.pem"
fi

echo "## print environment"
env
echo "## end print environment"

$@
