echo on
echo Setting up environment for Qt usage...
set QTDIR=C:\QtSDK\Desktop\Qt\4.8.1\mingw

set PATH=C:\epics\base-3.14.12\bin\win32-x86-mingw;%PATH%
set PATH=%QTDIR%\bin;%PATH%
set PATH=C:\QtSDK\mingw\bin;%PATH%

set PATH=c:\work\caQtDM;%PATH%
set PATH=C:\work\caQtDM_Lib\release;%PATH%
set PATH=C:\work\caQtDM\release;%PATH%
set PATH=C:\work\QtControls\qtcontrols\release;%PATH%
set PATH=C:\Qwt-6.0.1\lib;%PATH%

cd \work\caQtDM\parser\proscanFiles

set EPICS_CA_MAX_ARRAY_BYTES=40000000
set EPICS_CA_ADDR_LIST=proscan-cagw01.psi.ch
set EPICS_CA_SERVER_PORT=5062
set EPICS_CA_AUTO_ADDR_LIST=NO
echo "EPICS Channel Access is to the proscan Network"

start /wait caQtDM tests.ui