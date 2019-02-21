export ZMQLIB=/lib64
export ZMQINC=//usr/include
#export ZMQINC=/afs/psi.ch/user/e/ebner/package/zmq/zeromq-4.2.3/include

VERSION=4.2.4
VERSION_TAG=V${VERSION}
GIT_CHECKOUT=$VERSION_TAG

# If you want to compile latest release candidate uncomment this line
# GIT_CHECKOUT=Release
REPOSITORY_NAME=caqtdm
REPOSITORY=https://github.com/caqtdm/$REPOSITORY_NAME.git


BUILD_ROOT=$(pwd)/build

# Location where package is assembled
PACKAGE_BUILD_BASE=$BUILD_ROOT/package
# Location where package will be finally installed
PACKAGE_INST_BASE=/opt/caqtdm
# Location where build binaries are located
BUILD_HOME=$BUILD_ROOT/$REPOSITORY_NAME/caQtDM_Binaries

ARCHITECTURE=`uname -m`

# Remove build directory if it already exists
rm -rf $BUILD_ROOT

# Create build directory
mkdir -p $BUILD_ROOT
cd $BUILD_ROOT

# Clone caqtdm sources
git clone $REPOSITORY
cd $REPOSITORY_NAME
# TODO Enable this again for serious builds
git checkout $GIT_CHECKOUT


#cd caQtDM_Lib/caQtDM_Plugins/
patch -R < $BUILD_ROOT/../patch
#cd ../..

# Build sources
export PATH=/usr/lib64/qt4/bin/:$PATH
export PYTHONVERSION=2.7

#cd $REPOSITORY_NAME
echo y | ./caQtDM_BuildAll

# Clean and create package build base
rm -rf $PACKAGE_BUILD_BASE

mkdir -p $PACKAGE_BUILD_BASE
mkdir -p ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}

mkdir -p ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib
cd ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib
# Copy build binaries in place
cp -r $BUILD_HOME/* .

mkdir -p ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/bin
cd ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/bin

ln -s ../lib/adl2ui
ln -s ../lib/edl2ui

# Create wrapper for qt designer
cat > caqtdm_designer << "EOF"
#!/bin/bash
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
CAQTDM_HOME=$DIR/..
# Register help
assistant-qt4 -register $CAQTDM_HOME/doc/caQtDM.qch
export QT_PLUGIN_PATH=$CAQTDM_HOME/lib
designer-qt4 $@
EOF
chmod 755 caqtdm_designer
# Create wrapper for caqtdm binary
cat > caqtdm << "EOF"
#!/bin/bash
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
CAQTDM_HOME=$DIR/..
export QT_PLUGIN_PATH=$CAQTDM_HOME/lib
$CAQTDM_HOME/lib/caQtDM -style plastique $@
EOF
chmod 755 caqtdm
# Copy docs and help files (this includes the widget help and documentation)
cd ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}
cp -r $BUILD_ROOT/$REPOSITORY_NAME/caQtDM_QtControls/doc .
cd $PACKAGE_BUILD_BASE
# Modify RPATH of the binaries to match install location:
FILES="${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib/caQtDM
${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib/controlsystems/libepics3_plugin.so
${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib/controlsystems/libdemo_plugin.so
${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib/controlsystems/libarchiveSF_plugin.so
${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib/controlsystems/libbsread_Plugin.so
${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib/designer/libqtcontrols_utilities_plugin.so
${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib/designer/libqtcontrols_controllers_plugin.so
${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib/designer/libqtcontrols_graphics_plugin.so
${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib/designer/libqtcontrols_monitors_plugin.so
${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/lib/libcaQtDM_Lib.so
"
for F in $FILES
do
    RPATH_OLD=$(chrpath -l $F | sed -e 's/.*RPATH=//')
    RPATH_NEW=$(echo $RPATH_OLD | sed -e 's%'${BUILD_HOME}'%'$PACKAGE_INST_BASE'/lib%')
    chrpath -r $RPATH_NEW $F
done
# ATTENTION using hardcoded opt !!!!
tar cfvz caqtdm-${VERSION}_$ARCHITECTURE.tar.gz opt
echo ... packaging done.
# Packages this package is dependent on:
# qt
# qt-devel
# qt-assistant
# qwt
# qwt-devel
# epics-base
