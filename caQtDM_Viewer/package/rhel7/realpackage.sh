#!/bin/bash

# If you want to compile latest release candidate uncomment this line
REPOSITORY_NAME=caqtdm
PACKAGE_VERSION=4.4.1
REPOSITORY=https://github.com/caqtdm/$REPOSITORY_NAME.git
# BRANCH_OR_TAG=V${PACKAGE_VERSION}
BRANCH_OR_TAG=Release

#### Clone and build caqtdm sources
git clone $REPOSITORY
cd $REPOSITORY_NAME
# TODO Enable this again for serious builds
git checkout $BRANCH_OR_TAG
rm -rf .git
cd ..
mv caqtdm caqtdm-${PACKAGE_VERSION}
tar -czf caqtdm-${PACKAGE_VERSION}.tar.gz ./caqtdm-${PACKAGE_VERSION}
rm -rf caqtdm-${PACKAGE_VERSION}

export   EPICS_BASE_TARGET=/usr/local/epics/base-7.0.6;


mv caqtdm-${PACKAGE_VERSION}.tar.gz  ../rpmbuild/SOURCES/
 
rpmbuild -ba caqtdm.spec
