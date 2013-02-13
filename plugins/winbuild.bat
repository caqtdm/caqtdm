echo on
echo Setting up environment for Qt usage...
set QTDIR=C:\QtSDK\Desktop\Qt\4.8.1\mingw
set PATH=%QTDIR%\bin;%PATH%
set PATH=C:\QtSDK\mingw\bin;%PATH%

qmake plugins.pro CONFIG+=configA
c:\QtSDK\mingw\bin\mingw32-make.exe -f Makefile.release clean
qmake plugins.pro CONFIG+=configA
c:\QtSDK\mingw\bin\mingw32-make.exe -f Makefile.release

qmake plugins.pro CONFIG+=configB
c:\QtSDK\mingw\bin\mingw32-make.exe -f Makefile.release clean
qmake plugins.pro CONFIG+=configB
c:\QtSDK\mingw\bin\mingw32-make.exe -f Makefile.release

qmake plugins.pro CONFIG+=configC
c:\QtSDK\mingw\bin\mingw32-make.exe -f Makefile.release clean
qmake plugins.pro CONFIG+=configC
c:\QtSDK\mingw\bin\mingw32-make.exe -f Makefile.release
