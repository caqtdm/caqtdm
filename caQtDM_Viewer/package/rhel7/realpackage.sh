#!/bin/bash
echo "" 
echo "     caQtDM BuildScript2RPM  "
echo "" 
if [ "$1" == "--help" ]; then
  echo "" 
  echo "" 
  echo "Usage: realpackage.sh [OPTION...]"
  echo "Buildscript for caQtDM on Redhat Linux 7/8 to RPM"
  echo "" 
  echo "Examples:" 
  echo "./realpackage.sh              # Normal git checkout + using spec file from git " 
  echo "./realpackage.sh --rpmdev     # use the current caqtdm.spec in this directory  " 
  echo "" 
  echo "" 
  echo "" 
  exit 0
fi

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
if [ "$1" != "--rpmdev" ]; then
    mv ./caqtdm.spec ./caqtdm.spec_old
    cp ./caqtdm/caQtDM_Viewer/package/rhel7/caqtdm.spec ./
fi

mv caqtdm caqtdm-${PACKAGE_VERSION}
tar -czf caqtdm-${PACKAGE_VERSION}.tar.gz ./caqtdm-${PACKAGE_VERSION}
rm -rf caqtdm-${PACKAGE_VERSION}

export   EPICS_BASE_TARGET=/usr/local/epics/base-7.0.6;


mv caqtdm-${PACKAGE_VERSION}.tar.gz  ../rpmbuild/SOURCES/
 
rpmbuild -ba caqtdm.spec
