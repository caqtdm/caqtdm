echo on
echo Setting up environment for Qt usage...
set QTDIR=C:\QtSDK\Desktop\Qt\4.8.1\mingw
set PATH=%QTDIR%\bin;%PATH%
set PATH=C:\QtSDK\mingw\bin;%PATH%

qmake caQtDM.pro
c:\QtSDK\mingw\bin\mingw32-make.exe -f Makefile.release clean
c:\QtSDK\mingw\bin\mingw32-make.exe -f Makefile.release
