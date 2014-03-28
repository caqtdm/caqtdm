echo on
echo Setting up environment for Qt usage...
set QTDIR=C:\QtSDK\Desktop\Qt\4.8.1\mingw
set PATH=%QTDIR%\bin;%PATH%
set PATH=C:\QtSDK\mingw\bin;%PATH%

set PATH=C:\work\QtControls\qtcontrols\release;%PATH%
set PATH=C:\Qwt-6.0.1\lib;%PATH%
cd \work\caQtDM\parser\proscanFiles

start  designer