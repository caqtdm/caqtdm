@echo off
echo =============================================================================================
cd
echo =============================================================================================

call caQtDM_Env.bat

call "C:\Program files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
set QMAKESPEC=%QTHOME%\mkspecs\win32-msvc2010
set PATH=%PATH%;%QTDM_BININSTALL%

echo package will be removed from .\caQtDM_Binaries and all directories will be cleaned up

echo Press [Enter] key to start cleanup

echo ========== remove binaries from directories ============
qmake all.pro
nmake clean


echo ========== remove makefiles from directories ============
del Makefile
del .\caQtDM_Viewer\Makefile*

del .\caQtDM_Viewer\parser\Makefile*
del .\caQtDM_Lib\Makefile*

del .\caQtDM_QtControls\Makefile*
del .\caQtDM_QtControls\plugins\Makefile*

rmdir /S /Q .\caQtDM_Viewer\release
rmdir /S /Q .\caQtDM_Viewer\debug
rmdir /S /Q .\caQtDM_Viewer\moc

rmdir /S /Q .\caQtDM_Viewer\parser\release
rmdir /S /Q .\caQtDM_Viewer\parser\debug
rmdir /S /Q .\caQtDM_Viewer\parser\moc


rmdir /S /Q .\caQtDM_Lib\release
rmdir /S /Q .\caQtDM_Lib\debug
rmdir /S /Q .\caQtDM_Lib\moc


rmdir /S /Q .\caQtDM_QtControls\release
rmdir /S /Q .\caQtDM_QtControls\debug
rmdir /S /Q .\caQtDM_QtControls\moc

rmdir /S /Q .\caQtDM_QtControls\plugins\release
rmdir /S /Q .\caQtDM_QtControls\plugins\debug

rmdir /S /Q .\caQtDM_Binaries

echo =========== remove package files ==================

del .\caQtDM_Viewer\package\windows\caQtDM.msi
del .\caQtDM_Viewer\package\windows\caQtDM.wixobj
del .\caQtDM_Viewer\package\windows\caQtDM.wixpdb

echo =========== clean all ==================


pause