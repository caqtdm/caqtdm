#!/bin/bash -e

CAQTDM_TAG=V4.6.0
CAQTDM_REVISION=1
export CAQTDM_VERSION=$(echo ${CAQTDM_TAG} | sed 's/V//;s/-/~/g')
export CAQTDM_VERSION_DEB=${CAQTDM_VERSION}-${CAQTDM_REVISION}

mkdir -p caqtdm/debian/
cp -r ./debian/* caqtdm/debian/
cd caqtdm



CAQTDM_TAG=Development
# Get source code (equivalent to PKGBUILD's source field)
git clone -c advice.detachedHead=false --branch ${CAQTDM_TAG} https://github.com/caqtdm/caqtdm.git temp
# Remove the .git directory
rm -rf temp/.git

cd temp
# Compress to caqtdm-${CAQTDM_VERSION}.orig.tar.gz
tar -czf ../caqtdm_${CAQTDM_VERSION}.orig.tar.gz --exclude=.git . 
cd ..

rsync -a --remove-source-files temp/ .
rm -rf temp/

mv ./caqtdm_${CAQTDM_VERSION}.orig.tar.gz ../

# Make rules executable
chmod +x debian/rules

cd debian/

# Replace change log %DATE% with the current date
sed -i "s/%DATE%/$(LC_TIME=C date +'%a, %d %b %Y %H:%M:%S %z')/" changelog

# Replace control and changelog with the current version
sed -i "s/%VERSION%/${CAQTDM_VERSION_DEB}/" control
sed -i "s/%VERSION%/${CAQTDM_VERSION_DEB}/" changelog

# Build the package
debuild -us -uc
