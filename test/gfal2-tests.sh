#!/usr/bin/bash
set -ex

export HOME=/
export X509_USER_PROXY=/tmp/x509up_u`id -u`

VOMS=${VOMS:=dteam}
PASSWD=$1

if [[ -z "$PASSWD" ]]; then
    echo "Missing password"
    exit 1
fi

cat > /etc/gfal2.d/sftp_plugin.conf <<EOF
[SFTP PLUGIN]
PASSPHRASE=${PASSWD}
EOF

echo "${PASSWD}" | voms-proxy-init --rfc --voms "${VOMS}" --pwstdin
cd /usr/share/gfal2/tests
ctest -T test

