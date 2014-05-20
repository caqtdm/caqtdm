#!/bin/bash

# Location where package is assembled
PACKAGE_BUILD_BASE=~/caqtdm_package
# Location where package will be finally installed
PACKAGE_INST_BASE=/usr/caqtdm
# Location where build binaries are located
BUILD_HOME=~/caqtdm_project/caQtDM_Binaries

ARCHITECTURE=`uname -m`

# Clean and create package build base
rm -rf $PACKAGE_BUILD_BASE
mkdir -p $PACKAGE_BUILD_BASE


mkdir -p ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}

# Copy required qwt 6.0.1 into place

cd /tmp
TAR_NAME=qwt-6.0.1_${ARCHITECTURE}.tar.gz
wget http://yoke.psi.ch/artifactory/releases/${TAR_NAME}
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

mkdir -p ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/bin
cd ${PACKAGE_BUILD_BASE}${PACKAGE_INST_BASE}/bin
mv ../lib/adl2ui .

# Create wrapper for qt designer
cat > caqtdm_designer << "EOF"
#!/bin/bash

CAQTDM_HOME=/usr/caqtdm

export QT_PLUGIN_PATH=$CAQTDM_HOME/lib
designer-qt4 $@

EOF

chmod 755 caqtdm_designer


# Create wrapper for caqtdm binary
cat > caqtdm << "EOF"
#!/bin/bash

CAQTDM_HOME=/usr/caqtdm

export QT_PLUGIN_PATH=$CAQTDM_HOME/lib
$CAQTDM_HOME/lib/caQtDM $@

EOF

chmod 755 caqtdm
