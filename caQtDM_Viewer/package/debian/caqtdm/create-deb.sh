#!/bin/bash -e
echo "" 
echo "     caQtDM BuildScript2DEB  "
echo "" 
if [ "$1" == "--help" ]; then
  echo "" 
  echo "" 
  echo "Usage: create-deb.sh [OPTION...]"
  echo "Buildscript for caQtDM on Debian "
  echo "" 
  echo "Examples:" 
  echo "./create-deb.sh              # Normal git checkout + using spec file from git " 
  echo "./create-deb.sh --debdev     # use the current caqtdm.spec in this directory  " 
  echo "" 
  echo "" 
  echo "" 
  exit 0
fi

# If you want to compile latest release candidate uncomment this line
REPOSITORY_NAME=caqtdm
PACKAGE_VERSION=4.6.0
REPOSITORY=https://github.com/caqtdm/$REPOSITORY_NAME.git
# BRANCH_OR_TAG=V${PACKAGE_VERSION}
BRANCH_OR_TAG=Development

rm -rf caqtdm-${PACKAGE_VERSION}  || true

if [ "$1" != "--debdev" ]; then
  #### Clone and build caqtdm sources
  git clone $REPOSITORY
  cd $REPOSITORY_NAME
  # TODO Enable this again for serious builds
  git checkout $BRANCH_OR_TAG
  rm -rf .git
  cd ..

  mv caqtdm caqtdm_${PACKAGE_VERSION}
  cd caqtdm_${PACKAGE_VERSION}

  tar -czf ../caqtdm_${PACKAGE_VERSION}.orig.tar.gz  --exclude=.git . 
  cd ..
fi


mkdir -p caqtdm-${PACKAGE_VERSION}/
cd caqtdm-${PACKAGE_VERSION}
pwd
dh_make --createorig -p caqtdm_${CAQTDM_VERSION} --packagename caqtdm -s -y || true
pwd
cd ..
pwd
cp -r ./debian/* caqtdm-${PACKAGE_VERSION}/debian/
pwd
cd caqtdm-${PACKAGE_VERSION}
pwd

tar -xf ../caqtdm_${PACKAGE_VERSION}.orig.tar.gz
pwd
cp -rf ../debian/* debian/
rm -rf debian/*.ex debian/README.Debian debian/README.source
pwd

# Build the package
dpkg-buildpackage -us -uc
