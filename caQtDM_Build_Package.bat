@echo off
call caQtDM_Env.bat
echo =============================================================================================
echo.

echo binaries will be installed in %QTDM_BININSTALL%
echo libraries will be installed in %QTDM_LIBINSTALL%
echo plugins will be installed in %QTDM_LIBINSTALL%/designer

echo.
echo "Press [Enter] key to start build "
echo.
pause

set PATH=%PATH%;%WIXHOME%

cd .\caQtDM\package\windows

candle caQtDM.wxs -ext WixUIExtension

light caQtDM.wixobj -ext WixUIExtension

copy caQtDM.msi ..\..\..\caQtDM_Binaries 

cd ..\..\..

echo .
echo "Put these in your .profile"
echo "export QT_PLUGIN_PATH=%QTDM_LIBINSTALL%"
echo .
echo "and execute 'designer' for building GUI's, 'adl2ui' for converting medm files, 'startDM' to execute caQtDM"
echo "you may try the test files located in caQtDM_Tests (set CAQTDM_DISPLAY_PATH accordingly)"
echo "you may try also to execute the command 'run-epics' in caQtDM_Tests (will setup a softioc) to get the channels"
echo "used by the test."
echo .
pause
