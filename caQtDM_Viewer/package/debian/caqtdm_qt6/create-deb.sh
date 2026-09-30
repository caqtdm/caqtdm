#!/bin/bash -e
echo "" 
echo "     caQtDM BuildScript2DEB  "
echo "" 
NO_CHECKOUT=0
DEBDEV=0
NO_PARENT_ORIG=0
SHOW_HELP=0
for option in "$@"; do
  case "$option" in
    --no-checkout) NO_CHECKOUT=1 ;;
    --debdev) DEBDEV=1 ;;
    --no-parent-orig) NO_PARENT_ORIG=1 ;;
    --help) SHOW_HELP=1 ;;
    *) echo "Unknown option: $option"; exit 1 ;;
  esac
done

if [ "$SHOW_HELP" -eq 1 ]; then
  echo "" 
  echo "" 
  echo "Usage: create-deb.sh [OPTION...]"
  echo "Buildscript for caQtDM on Debian "
  echo "" 
  echo "Examples:" 
  echo "./create-deb.sh              # Normal git checkout + using spec file from git " 
  echo "./create-deb.sh --debdev     # use the current caqtdm.spec in this directory  " 
  echo "./create-deb.sh --no-checkout # use the current directory as source, do not checkout from git "
  echo "./create-deb.sh --no-parent-orig # keep the orig archive in this directory "
  echo "" 
  echo "" 
  echo "" 
  exit 0
fi

# If you want to compile latest release candidate uncomment this line
REPOSITORY_NAME=caqtdm
REPOSITORY=https://github.com/caqtdm/$REPOSITORY_NAME.git
# BRANCH_OR_TAG=V${CAQTDM_VERSION}
BRANCH_OR_TAG=Development

# Package version: env CAQTDM_VERSION wins, else qtdefs.pri, else 1.0.0
if [ -z "${CAQTDM_VERSION}" ]; then
  CAQTDM_VERSION=$(sed -n 's/^[[:space:]]*CAQTDM_VERSION[[:space:]]*=[[:space:]]*[Vv]\{0,1\}\([0-9][0-9.]*\).*/\1/p' ../../../qtdefs.pri 2>/dev/null | head -n 1)
fi
if [ -z "${CAQTDM_VERSION}" ]; then
  echo "WARNING: could not determine caQtDM version, falling back to 1.0.0"
  CAQTDM_VERSION=1.0.0
fi
PACKAGE_VERSION=${CAQTDM_VERSION}
echo "PACKAGE_VERSION=${PACKAGE_VERSION}"

rm -rf caqtdm-${PACKAGE_VERSION} || true

if [ "$NO_CHECKOUT" -eq 0 ] && [ "$DEBDEV" -eq 0 ]; then
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
elif [ "$NO_CHECKOUT" -eq 1 ]; then
  echo "Using current directory as source, not checking out from git"
  currentdir=$(pwd)
  cd ../../../../
  if [ "$NO_PARENT_ORIG" -eq 1 ]; then
    tar -czf "${currentdir}/caqtdm_${PACKAGE_VERSION}.orig.tar.gz" \
      --exclude=.git \
      --exclude="./caQtDM_Viewer/package/debian/caqtdm_qt6/caqtdm_${PACKAGE_VERSION}.orig.tar.gz" \
      .
  else
    tar -czf ../caqtdm_${PACKAGE_VERSION}.orig.tar.gz --exclude=.git .
    mv ../caqtdm_${PACKAGE_VERSION}.orig.tar.gz "${currentdir}/caqtdm_${PACKAGE_VERSION}.orig.tar.gz"
  fi
  cd "${currentdir}"
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

# Keep debian/changelog in sync: prepend an entry if the top version differs
CHANGELOG_VERSION=$(sed -n '1s/^caqtdm (\([0-9][0-9.]*\).*/\1/p' debian/changelog)
if [ "${CHANGELOG_VERSION}" != "${PACKAGE_VERSION}" ]; then
  {
    echo "caqtdm (${PACKAGE_VERSION}-1) unstable; urgency=medium"
    echo ""
    echo "  * Automated build (create-deb.sh)"
    echo ""
    echo " -- Helge Brands <helge.brands@psi.ch>  $(date -R)"
    echo ""
    cat debian/changelog
  } > debian/changelog.new
  mv debian/changelog.new debian/changelog
fi

# Build the package
dpkg-buildpackage -us -uc
