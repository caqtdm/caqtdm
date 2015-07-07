@echo off
call caQtDM_Env.bat
echo =============================================================================================
echo.

echo binaries will be installed in %QTDM_BININSTALL%
echo libraries will be installed in %QTDM_LIBINSTALL%
echo plugins will be installed in %QTDM_LIBINSTALL%/designer
echo.

set PATH=%PATH%;%WIXHOME%
set WINCAQTDM_COLLECT=%CAQTDM_COLLECT:/=\%

cd .\caQtDM_Viewer\package\windows

IF %SELCTION%==1 GOTO BUILDQT4_32
IF %SELCTION%==2 GOTO BUILDQT5_64
IF %SELCTION%==3 GOTO BUILDQT5_32


:BUILDQT4_32
IF EXIST .\project (cd project && del /q /f /s *.*) ELSE (mkdir project && cd project)
candle ../caQtDM.wxs -ext WixUIExtension -ext WixUtilExtension
light caQtDM.wixobj -ext WixUIExtension -ext WixUtilExtension
copy caQtDM.msi %WINCAQTDM_COLLECT% 
GOTO FINISHED

:BUILDQT5_64
IF EXIST .\project_x64 (cd project_x64 && del /q /f /s *.*) ELSE (mkdir project_x64 && cd project_x64)
candle ../caQtDM_x64.wxs -ext WixUIExtension -ext WixUtilExtension
light caQtDM_x64.wixobj -ext WixUIExtension -ext WixUtilExtension
copy caQtDM_x64.msi %WINCAQTDM_COLLECT% 
GOTO FINISHED

:BUILDQT5_32
IF EXIST .\project_x86 (cd project_x86 && del /q /f /s *.*) ELSE (mkdir project_x86 && cd project_x86)
candle ../caQtDM_x86.wxs -ext WixUIExtension -ext WixUtilExtension
light caQtDM_x86.wixobj -ext WixUIExtension -ext WixUtilExtension
copy caQtDM_x86.msi %WINCAQTDM_COLLECT% 
GOTO FINISHED



:FINISHED
cd ..\..\..\..


echo .
echo "and execute 'designer' for building GUI's, 'adl2ui' for converting medm files, 'startDM' to execute caQtDM"
echo "you may try the test files located in caQtDM_Tests (set CAQTDM_DISPLAY_PATH accordingly)"
echo "you may try also to execute the command 'softioc.bat' in caQtDM_Tests (will setup a softioc) to get the channels"
echo "used by the test."
echo .
pause
