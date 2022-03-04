#!/bin/bash
set -xe

# Defaults
MOCK_CONFIG_DIR="/etc/mock"
MOCK_ROOT="dev-epel-6-x86_64"
SKIP_INIT=0
UCERT="/afs/cern.ch/user/a/aalvarez/public/saketag-cert.pem"
UKEY="/afs/cern.ch/user/a/aalvarez/public/saketag-key.pem"
SSHKEY="/afs/cern.ch/user/a/aalvarez/public/tests_id_rsa"
PASSWD=""
VOMS="dteam"
OUTPUT_DIR="/tmp"

# Get parameters
while [ -n "$1" ]; do
    case "$1" in
        "--configdir")
            shift
            MOCK_CONFIG_DIR=$1
        ;;
        "--root")
            shift
            MOCK_ROOT=$1
        ;;
        "--skip-init")
            SKIP_INIT=1
        ;;
        "--ucert")
            shift
            UCERT=$1
        ;;
        "--ukey")
            shift
            UKEY=$1
        ;;
        "--passwd")
            shift
            PASSWD=$1
        ;;
        "--sshkey")
            shift
            SSHKEY=$1
        ;;
        "--voms")
            shift
            VOMS=$1
        ;;
        "--output")
            shift
            OUTPUT_DIR=$1
        ;;
        *)
            echo "Unknown parameter $1"
            exit 1
        ;;
    esac
    shift
done

# Feedback
echo "# Mock config dir:  ${MOCK_CONFIG_DIR}"
echo "# Mock root:        ${MOCK_ROOT}"
echo "# User certificate: ${UCERT}"
echo "# User key:         ${UKEY}"

# Run inside mock
function mock_cmd()
{
    mock -v --configdir "${MOCK_CONFIG_DIR}" -r "${MOCK_ROOT}" --disable-plugin=tmpfs --no-clean "$@"
}


# Prepare environment
if [ $SKIP_INIT == 0 ]; then
    mock_cmd clean
    mock_cmd init
else
    echo "Skipping initialization"
fi

# Install tests
mock_cmd install cmake gfal2-tests ca-policy-egi-core voms-clients voms-config-vo-dteam

# Push certificate and key
MOCK_HOME="/builddir"
mock_cmd chroot "mkdir -p ${MOCK_HOME}/.globus"
mock_cmd chroot "mkdir -p ${MOCK_HOME}/.ssh"
mock_cmd --copyin "${UCERT}" "${MOCK_HOME}/.globus/usercert.pem"
mock_cmd --copyin "${UKEY}" "${MOCK_HOME}/.globus/userkey.pem"
mock_cmd --copyin "${SSHKEY}" "${MOCK_HOME}/.ssh/id_rsa"
mock_cmd chroot "chmod 0400 ${MOCK_HOME}/.globus/*.pem ${MOCK_HOME}/.ssh/id_rsa"
mock_cmd chroot "chown root.root ${MOCK_HOME}/.globus/*.pem ${MOCK_HOME}/.ssh/id_rsa"
mock_cmd chroot "echo -e \"[SFTP PLUGIN]\nPASSPHRASE=${PASSWD}\" > /etc/gfal2.d/sftp_plugin.conf"

# Create proxy
mock_cmd chroot "echo ${PASSWD} | voms-proxy-init --rfc --cert ${MOCK_HOME}/.globus/usercert.pem --key ${MOCK_HOME}/.globus/userkey.pem --voms ${VOMS} --pwstdin"

# Run tests
mock_cmd chroot "cd /usr/share/gfal2/tests/; ctest -T test"

# Recover logs
rm -rf "${OUTPUT_DIR}/Testing"
mock_cmd --copyout "/usr/share/gfal2/tests/Testing" "${OUTPUT_DIR}/Testing"

