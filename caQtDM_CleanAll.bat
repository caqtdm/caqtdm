@echo off
echo =============================================================================================
cd
echo =============================================================================================

call caQtDM_Env.bat

call "C:\Program files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
set QMAKESPEC=%QTHOME%\mkspecs\win32-msvc2013
set PATH=%PATH%;%QTHOME%\bin

echo package will be removed from .\caQtDM_Binaries and all directories will be cleaned up

echo Press [Enter] key to start cleanup

echo ========== remove binaries from directories ============
qmake all.pro
where /q jom.exe 
IF %ERRORLEVEL% NEQ 0 (nmake clean) ELSE (jom clean)


echo ========== remove makefiles from directories ============
del Makefile
del .\caQtDM_Viewer\Makefile*

del .\caQtDM_Viewer\parser\Makefile*
del .\caQtDM_Viewer\parserEDM\Makefile*
del .\caQtDM_Lib\Makefile*

del .\caQtDM_QtControls\Makefile*
del .\caQtDM_QtControls\plugins\Makefile*

rmdir /S /Q .\caQtDM_Viewer\release
rmdir /S /Q .\caQtDM_Viewer\debug
rmdir /S /Q .\caQtDM_Viewer\moc

rmdir /S /Q .\caQtDM_Viewer\parser\release
rmdir /S /Q .\caQtDM_Viewer\parser\debug
rmdir /S /Q .\caQtDM_Viewer\parser\moc

rmdir /S /Q .\caQtDM_Viewer\parserEDM\release
rmdir /S /Q .\caQtDM_Viewer\parserEDM\debug
rmdir /S /Q .\caQtDM_Viewer\parserEDM\moc

rmdir /S /Q .\caQtDM_Lib\release
rmdir /S /Q .\caQtDM_Lib\debug
rmdir /S /Q .\caQtDM_Lib\moc


rmdir /S /Q .\caQtDM_QtControls\release
rmdir /S /Q .\caQtDM_QtControls\debug
rmdir /S /Q .\caQtDM_QtControls\moc

rmdir /S /Q .\caQtDM_QtControls\plugins\release
rmdir /S /Q .\caQtDM_QtControls\plugins\debug

rem rmdir /S /Q .\caQtDM_Binaries

echo =========== remove package files ==================

rmdir /S /Q .\caQtDM_Viewer\package\windows\project
rmdir /S /Q  .\caQtDM_Viewer\package\windows\project_x64
rmdir /S /Q  .\caQtDM_Viewer\package\windows\project_x86

echo =========== clean all ==================


pause