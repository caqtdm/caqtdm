# Overview
This is a packaging script for caqtdm for SL6 systems at PSI. The script will produce a tar file that directly can be used to deploy the package or can be used as entry point to create an RPM package.

# Packaging
To package caqtdm for SL5 and SL6 following steps are needed.

 * Clone this repository
 * Update and commit/push the package version number inside the package script
 * Ensure that your on instbase `/work` - `cb /work`
 * Execute packaging script

```
./package.sh
```

For colored output use

```
# dye ./package.sh
```

The will create a `package` folder in which the content of the package is located (extracted and as tar file). This content
can further on be packaged via rpm or some other package format.

## Info
The caqtdm package (created by the package script) is supposed to be installed in `/usr/caqtdm`

To build the package for different linux architectures use following servers:

 * SL6 64Bit gfa-lc6d-64.psi.ch
 * SL6 32Bit sls-lc6-32.psi.ch

## QWT
If you are required to (re)compile qwt for your platform, these are the steps:

 * Compile qwt libraries
  * Download and extract qwt sources - http://artifacts.psi.ch/artifactory/releases/qwt-6.0.1_source.tar.gz
  * Build and install sources

```
wget http://artifacts.psi.ch/artifactory/releases/qwt-6.0.1_source.tar.gz
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


# Usage
Once the caqtdm binaries are installed in `/usr/caqtdm` you can use caqtdm as follows:

```
# These variables do not need to be set on GFA SL6 machines
export CAQTDM_DISPLAY_PATH=/work/sls/config/qt
export PATH=/usr/caqtdm/bin:$PATH

caqtdm -macro P=BLA,M=HELLO my.ui
```

Following options are available:

  * __-macro__ - Specify macros, example: _-macro P=MTEST-HW3,M=:MOT1_
  * __-noMsg__ - Minimizes the message window
  * __-attach__  Attaches the panel to an existing caQtDM task. Therefore, the message window is opened only once.

_Note:_ .ui files are searched inside the folder specified via the `CAQTDM_DISPLAY_PATH` variable. If you want to open a .ui
file not inside this folder specify the absolute path to the file.


To start the designer use:

```
caqtdm_designer
```

## Options
For remote connetions via e.g. ssh use following option:

```
-graphicssystem native
```

## Known Problems

While starting up the `caqtdm_designer` sometimes the program crashes.

```
[gfa-lc6-64 ~]
[-bash INSTBASE=/prod]$ caqtdm_designer
Connecting to deprecated signal QDBusConnectionInterface::serviceOwnerChanged(QString,QString,QString)
kbuildsycoca4 running...
Connecting to deprecated signal QDBusConnectionInterface::serviceOwnerChanged(QString,QString,QString)
**
ERROR:pulsesink.c:2314:gst_pulsesink_change_state: assertion failed: (pulsesink->mainloop == NULL)
/usr/caqtdm/bin/caqtdm_designer: line 6: 19395 Aborted                 designer-qt4 $@
[gfa-lc6-64 ~]
```

__Workaround:__ Execute the command again right after the crash.
