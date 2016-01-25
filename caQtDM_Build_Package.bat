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
signtool sign /fd SHA256 /n %CAQTDM_SIGNER% /t %TIMESTAPER% %WINCAQTDM_COLLECT%\caQtDM.msi

GOTO FINISHED

:BUILDQT5_64
IF EXIST .\project_x64 (cd project_x64 && del /q /f /s *.*) ELSE (mkdir project_x64 && cd project_x64)
candle ../caQtDM_x64.wxs -ext WixUIExtension -ext WixUtilExtension
light caQtDM_x64.wixobj -ext WixUIExtension -ext WixUtilExtension
copy caQtDM_x64.msi %WINCAQTDM_COLLECT% 
signtool sign /fd SHA256 /n %CAQTDM_SIGNER% /t %TIMESTAPER% %WINCAQTDM_COLLECT%\caQtDM_x64.msi
GOTO FINISHED

:BUILDQT5_32
IF EXIST .\project_x86 (cd project_x86 && del /q /f /s *.*) ELSE (mkdir project_x86 && cd project_x86)
candle ../caQtDM_x86.wxs -ext WixUIExtension -ext WixUtilExtension
light caQtDM_x86.wixobj -ext WixUIExtension -ext WixUtilExtension
copy caQtDM_x86.msi %WINCAQTDM_COLLECT% 
signtool sign /fd SHA256 /n %CAQTDM_SIGNER% /t %TIMESTAPER% %WINCAQTDM_COLLECT%\caQtDM_x86.msi
GOTO FINISHED



:FINISHED
cd ..\..\..\..

