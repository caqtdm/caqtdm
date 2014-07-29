# Packaging
To package caqtdm for SL5 and SL6 following steps are needed

## QWT
 * Compile qwt libraries
  * Download and extract qwt sources - http://yoke.psi.ch/artifactory/releases/qwt-6.0.1_source.tar.gz
  * Build and install sources

```
wget http://yoke.psi.ch/artifactory/releases/qwt-6.0.1_source.tar.gz
tar xfvz qwt-6.0.1_source.tar.gz
cd qwt-6.0.1

export INSTALL_ROOT=~
export PATH=/usr/lib64/qt4/bin:$PATH
qmake
make
make install
cd ..
```

 * Remove doc directory in qwt release
 * Package qwt and upload to artifactory

## CAQTDM

 * Create build directory
 
```
mkdir build
cd build
export BUILD_ROOT=$(pwd)
cd $BUILD_ROOT
```
 
 * Download qwt
 
```
wget http://yoke.psi.ch/artifactory/releases/qwt-6.0.1_${uname -m}.tar.gz
mkdir -p usr/local
cd usr/local
tar xfvz ../../qwt-6.0.1_*.tar.gz
cd  $BUILD_ROOT
rm -rf qwt-6.0.1_*.tar.gz
```

 * Clone caqtdm sources

```
git clone https://github.psi.ch/scm/qtdm/caqtdm_project.git
cd caqtdm_project
git checkout <tag>
```

 * Ensure that you are on base `work`

```
cb work
```

 * Build caqtdm binaries for architecture

```
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

./caQtDM_BuildAll
```

To build the package qwt-6.0.1 and epics base is required (adapt the paths above if required).

 * After caqtdm is build run the packaging script
  * the packaging script requires the `chrpath` command. Make sure that this command is available on the build machine!

```
./makePackage.sh
```

This will create a ~/caqtdm_package folder in which the content of the package is located. This content
can further on be packaged via rpm or some other package format.

## Info
The caqtdm package (created by the package script) is supposed to be installed in `/usr/caqtdm`

To build the package for different linux architectures use following servers:

 * SL6 64Bit gfalcd.psi.ch
 * SL6 32Bit gfalc6032.psi.ch
 * SL5 32Bit gfalc5732.psi.ch

# Usage
Once the caqtdm binaries are installed in `/usr/caqtdm` you can use caqtdm as follows:

```
export CAQTDM_DISPLAY_PATH=/work/sls/config/qt
export PATH=/usr/caqtdm/bin:$PATH

caqtdm -macro P=MTEST-HW3,M=:MOT1 /work/sls/config/qt/motor_x_all.ui
```

