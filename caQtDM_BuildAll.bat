@echo off

call caQtDM_Env.bat

echo.
echo "========== create destination directory if not exists============"
echo.

call "C:\Program files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86

set QMAKESPEC=%QTHOME%\mkspecs\win32-msvc2010
set PATH=%PATH%;%QTDM_BININSTALL%


if exist ".\caQtDM_Binaries" (
 echo "directory .\caQtDM_Binaries exists already"
) else (
 echo "create directory ./caQtDM_Binaries"
 md .\caQtDM_Binaries
)

if exist .\caQtDM_Binaries\designer ( 
echo "directory .\caQtDM_Binaries\designer exists already"
) else (
 echo "create directory ./caQtDM_Binaries/designer"
 md .\caQtDM_Binaries\designer
)

echo "package will be build in ./caQtDM_Binaries"
echo.
echo "Press [Enter] key to start build "
pause

echo ============ make all =================
qmake all.pro
nmake all

echo =========== build parser =============
cd .\caQtDM_Viewer\parser
qmake parser.pro
nmake all
cd ..\..

echo =========== build plugins =============
cd caQtDM_QtControls\plugins\
call .\winbuild_msc.bat


cd ..
cd ..

pause