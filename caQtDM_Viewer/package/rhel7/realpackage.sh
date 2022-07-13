#!/bin/bash

# If you want to compile latest release candidate uncomment this line
REPOSITORY_NAME=caqtdm
PACKAGE_VERSION=4.4.1
REPOSITORY=https://github.com/caqtdm/$REPOSITORY_NAME.git
# BRANCH_OR_TAG=V${PACKAGE_VERSION}
BRANCH_OR_TAG=feature/Qt6

#### Clone and build caqtdm sources
git clone $REPOSITORY
cd $REPOSITORY_NAME
# TODO Enable this again for serious builds
git checkout $BRANCH_OR_TAG
rm -rf .git
cd ..
mv caqtdm caQtDM-${PACKAGE_VERSION}
tar -czf caQtDM-${PACKAGE_VERSION}.tar.gz ./caQtDM-${PACKAGE_VERSION}
rm -rf caQtDM-${PACKAGE_VERSION}

export   EPICS_BASE_TARGET=/usr/local/epics/base-7.0.6;

DIR="../rpmbuild/SOURCES/"
[ ! -d "$DIR" ] && mkdir -p "$DIR"
mv caQtDM-${PACKAGE_VERSION}.tar.gz  "$DIR"
 
rpmbuild -ba caqtdm.spec
