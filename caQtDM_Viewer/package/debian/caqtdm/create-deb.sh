#!/bin/bash -e

CAQTDM_TAG=v4.5.0-rc2
CAQTDM_REVISION=1
export CAQTDM_VERSION=$(echo ${CAQTDM_TAG} | sed 's/v//;s/-/~/g')
export CAQTDM_VERSION_DEB=${CAQTDM_VERSION}-${CAQTDM_REVISION}


rm -rf caqtdm-${CAQTDM_VERSION} || true
rm -f caqtdm* || true

# Download the source code
wget https://github.com/caqtdm/caqtdm/archive/refs/tags/${CAQTDM_TAG}.tar.gz -O caqtdm_${CAQTDM_VERSION}.orig.tar.gz

tar xf caqtdm_${CAQTDM_VERSION}.orig.tar.gz

mv caqtdm-$(echo ${CAQTDM_TAG} | sed 's/v//') caqtdm-${CAQTDM_VERSION}
cd caqtdm-${CAQTDM_VERSION}

dh_make --createorig -p caqtdm_${CAQTDM_VERSION} --packagename caqtdm -s -y
cp -rf ../debian/* debian/
rm -f debian/*.ex debian/README.Debian debian/README.source
pwd

# Build the package
dpkg-buildpackage -us -uc