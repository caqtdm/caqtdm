#!/bin/bash

VERSION=4.1.3
VERSION_TAG=V${VERSION}

GIT_CHECKOUT=$VERSION_TAG
# If you want to compile latest release candidate uncomment this line
# GIT_CHECKOUT=Release

REPOSITORY=https://github.com/caqtdm/caqtdm.git
REPOSITORY_NAME=caqtdm
URL_ARTIFACT_REPO=http://artifacts.psi.ch/artifactory/releases

BUILD_ROOT=$(pwd)/build

# Location where package is assembled
PACKAGE_BUILD_BASE=$BUILD_ROOT/package
# Location where package will be finally installed
PACKAGE_INST_BASE=/usr/caqtdm
# Location where build binaries are located
BUILD_HOME=$BUILD_ROOT/$REPOSITORY_NAME/caQtDM_Binaries

ARCHITECTURE=`uname -m`

# Remove build directory if it already exists
rm -rf $BUILD_ROOT

# Create build directory
mkdir -p $BUILD_ROOT
cd $BUILD_ROOT

# Download QWT
wget $URL_ARTIFACT_REPO/qwt-6.0.1_$(uname -m).tar.gz
mkdir -p usr/local
cd usr/local
tar xfvz ../../qwt-6.0.1_*.tar.gz
cd  $BUILD_ROOT
rm -rf qwt-6.0.1_*.tar.gz

# Clone caqtdm sources
#git clone https://github.psi.ch/scm/qtdm/caqtdm_project.git
git clone $REPOSITORY
cd $REPOSITORY_NAME
git checkout $GIT_CHECKOUT

# Ensure that you are on base work
cb work

# Build caqtdm binaries for architecture
# INFO: To build the package qwt-6.0.1 and epics base is required (adapt the paths above if required)
if [ $(uname -m) == "i686" ];then
    # 32Bit machine
    export PATH=/usr/lib/qt4/bin:$PATH
else
    # 64Bit machine
    export PATH=/usr/lib64/qt4/bin:$PATH
fi
export QWTHOME=${BUILD_ROOT}/usr/local/qwt-6.0.1/features
export QWTINCLUDE=${BUILD_ROOT}/usr/local/qwt-6.0.1/include
export QWTLIB=${BUILD_ROOT}/usr/local/qwt-6.0.1/lib

# Compile CAQTDM
# echo y is required to automatically start the compilation
echo y | ./caQtDM_BuildAll


# Build package


# Clean and create package build base
rm -rf $PACKAGE_BUILD_BASE
mkdir -p $PACKAGE_BUILD_BASE


mkdir -p ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}

# Copy required qwt 6.0.1 into place

cd /tmp
TAR_NAME=qwt-6.0.1_${ARCHITECTURE}.tar.gz
wget $URL_ARTIFACT_REPO/${TAR_NAME}
cd ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}
tar xfvz /tmp/${TAR_NAME}
mv qwt-6.0.1 qwt
rm /tmp/${TAR_NAME}

mkdir -p ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib
cd ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib

# Copy build binaries in place
cp -r $BUILD_HOME/* .

# Adapt rpath for binaries
# This requires chrpath to be installed on the system (yum install chrpath)

chrpath -r /usr/local/epics/extensions/lib/${EPICS_HOST_ARCH}:$PACKAGE_INST_BASE/lib caQtDM
chrpath -r /usr/local/epics/base/lib/${EPICS_HOST_ARCH}:/usr/local/epics/extensions/lib/${EPICS_HOST_ARCH}:$PACKAGE_INST_BASE/lib libcaQtDM_Lib.so
chrpath -r $PACKAGE_INST_BASE/qwt/lib libqtcontrols.so

chrpath -r $PACKAGE_INST_BASE/qwt/lib:/usr/local/epics/extensions/lib/${EPICS_HOST_ARCH}:$PACKAGE_INST_BASE/lib designer/libqtcontrols_controllers_plugin.so
chrpath -r $PACKAGE_INST_BASE/qwt/lib:/usr/local/epics/extensions/lib/${EPICS_HOST_ARCH}:$PACKAGE_INST_BASE/lib designer/libqtcontrols_graphics_plugin.so
chrpath -r $PACKAGE_INST_BASE/qwt/lib:/usr/local/epics/extensions/lib/${EPICS_HOST_ARCH}:$PACKAGE_INST_BASE/lib designer/libqtcontrols_monitors_plugin.so

chrpath -r /usr/local/epics/extensions/lib/${EPICS_HOST_ARCH}:$PACKAGE_INST_BASE/lib controlsystems/libdemo_plugin.so
chrpath -r /usr/local/epics/base/lib/${EPICS_HOST_ARCH}:$PACKAGE_INST_BASE/lib controlsystems/libepics3_plugin.so

mkdir -p ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/bin
cd ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/bin
mv ../lib/adl2ui .

# Create wrapper for qt designer
cat > caqtdm_designer << "EOF"
#!/bin/bash

CAQTDM_HOME=/usr/caqtdm

# Register help
assistant-qt4 -register $CAQTDM_HOME/doc/caQtDM.qch

export QT_PLUGIN_PATH=$CAQTDM_HOME/lib
designer-qt4 $@

EOF

chmod 755 caqtdm_designer


# Create wrapper for caqtdm binary
cat > caqtdm << "EOF"
#!/bin/bash

CAQTDM_HOME=/usr/caqtdm

export QT_PLUGIN_PATH=$CAQTDM_HOME/lib
$CAQTDM_HOME/lib/caQtDM -style plastique $@

EOF

chmod 755 caqtdm

# Copy docs and help files (this includes the widget help and documentation)
cd ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}
cp -r $BUILD_ROOT/$REPOSITORY_NAME/caQtDM_QtControls/doc .

cd $PACKAGE_BUILD_BASE
tar cfvz caqtdm-${VERSION}_$ARCHITECTURE.tar.gz usr

echo ... packaging done.
