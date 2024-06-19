@echo off
echo =============================================================================================
cd
echo =============================================================================================
if "%CAQTDM_GENERAL_COMPILATION%"=="1" GOTO :clean
call caQtDM_Env.bat

set PATH=%PATH%;%QTHOME%\bin

echo .\caQtDM_Binaries(_64Bit) will be removed and all directories will be cleaned up

echo Press [Enter] key to start cleanup
:clean
echo ========== remove binaries from directories ============
qmake all.pro
where /q jom.exe 
IF %ERRORLEVEL% NEQ 0 (nmake clean) ELSE (jom clean)
rmdir /S /Q .\caQtDM_Binaries
rmdir /S /Q .\caQtDM_Binaries_64Bit


echo ========== remove makefiles from directories ============
del Makefile
del .\caQtDM_Viewer\Makefile*

del .\caQtDM_Viewer\parser\Makefile*
del .\caQtDM_Viewer\parserEDM\Makefile*
del .\caQtDM_Lib\Makefile*

del .\caQtDM_Lib\caQtDM_Plugins\Makefile*
del .\caQtDM_Lib\caQtDM_Plugins\demo\Makefile*
del .\caQtDM_Lib\caQtDM_Plugins\bsread\Makefile*
del .\caQtDM_Lib\caQtDM_Plugins\epics3\Makefile*
del .\caQtDM_Lib\caQtDM_Plugins\epics4\Makefile*
del .\caQtDM_Lib\caQtDM_Plugins\modbus\Makefile*
del .\caQtDM_Lib\caQtDM_Plugins\gps\Makefile*
del .\caQtDM_Lib\caQtDM_Plugins\environment\Makefile*



del .\caQtDM_Lib\caQtDM_Plugins\archive\Makefile*
del .\caQtDM_Lib\caQtDM_Plugins\archive\archiveSF\Makefile*
del .\caQtDM_Lib\caQtDM_Plugins\archive\archiveHIPA\Makefile*
del .\caQtDM_Lib\caQtDM_Plugins\archive\archiveHTTP\Makefile*
del .\caQtDM_Lib\caQtDM_Plugins\archive\archivePro\Makefile*

del .\caQtDM_Parsers\Makefile*
del .\caQtDM_Parsers\adlParserSharedLib\Makefile*
del .\caQtDM_Parsers\adlParserStaticLib\Makefile*


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

rmdir /S /Q .\caQtDM_Lib\caQtDM_Plugins\demo\release
rmdir /S /Q .\caQtDM_Lib\caQtDM_Plugins\demo\debug
rmdir /S /Q .\caQtDM_Lib\caQtDM_Plugins\demo\moc

rmdir /S /Q .\caQtDM_Lib\caQtDM_Plugins\epics3\release
rmdir /S /Q .\caQtDM_Lib\caQtDM_Plugins\epics3\debug
rmdir /S /Q .\caQtDM_Lib\caQtDM_Plugins\epics3\moc

rmdir /S /Q .\caQtDM_Lib\caQtDM_Plugins\epics4\release
rmdir /S /Q .\caQtDM_Lib\caQtDM_Plugins\epics4\debug
rmdir /S /Q .\caQtDM_Lib\caQtDM_Plugins\epics4\moc

rmdir /S /Q .\caQtDM_Lib\caQtDM_Plugins\archive\archiveHTTP\release
rmdir /S /Q .\caQtDM_Lib\caQtDM_Plugins\archive\archiveHTTP\debug
rmdir /S /Q .\caQtDM_Lib\caQtDM_Plugins\archive\archiveHTTP\moc

rmdir /S /Q .\caQtDM_Parsers\adlParserSharedLib\release
rmdir /S /Q .\caQtDM_Parsers\adlParserSharedLib\debug
rmdir /S /Q .\caQtDM_Parsers\adlParserSharedLib\moc

rmdir /S /Q .\caQtDM_Parsers\adlParserStaticLib\release
rmdir /S /Q .\caQtDM_Parsers\adlParserStaticLib\debug
rmdir /S /Q .\caQtDM_Parsers\adlParserStaticLib\moc

rmdir /S /Q .\caQtDM_QtControls\release
rmdir /S /Q .\caQtDM_QtControls\debug
rmdir /S /Q .\caQtDM_QtControls\moc

rmdir /S /Q .\caQtDM_QtControls\plugins\release
rmdir /S /Q .\caQtDM_QtControls\plugins\debug

echo =========== remove package files ==================

rmdir /S /Q .\caQtDM_Viewer\package\windows\project
rmdir /S /Q  .\caQtDM_Viewer\package\windows\project_x64
rmdir /S /Q  .\caQtDM_Viewer\package\windows\project_x86

echo =========== clean all ==================

if "%CAQTDM_GENERAL_COMPILATION%"=="1" GOTO :eof
pause