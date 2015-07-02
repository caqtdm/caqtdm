REM =============================================================================================
REM For general compiling
REM =============================================================================================

IF "%1"=="1" GOTO SELECT1
IF "%1"=="2" GOTO SELECT2
IF "%1"=="3" GOTO SELECT3

echo =============================================================================================
echo Select Build Environment
echo 1) QT 4.8.5 QWT6.0.1  32 Bit VS2010  
echo 2) QT 5.4.1 QWT6.1.2  64 Bit VS2013  
echo 3) QT 5.4.1 QWT6.1.2  32 Bit VS2013  
set /P SELCTION=Select: 
echo =============================================================================================
 
IF %SELCTION%==1 GOTO :SELECT1
IF %SELCTION%==2 GOTO :SELECT2
IF %SELCTION%==3 GOTO :SELECT3
IF %SELCTION%==A GOTO :SELECTA

REM =============================================================================================
REM SELECT1
REM =============================================================================================

:SELECT1 
 
  call "C:\Program files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
 
  set QTHOME=X:/qt/4.8.5
  set QWTHOME=X:/qt/qwt-6.0.1
  set QWTINCLUDE=%QWTHOME%/src
  set QWTLIB=%QWTHOME%/lib
  
  
  set EPICS_BASE=X:/epics/base-3.14.12.4
  set EPICS_HOST_ARCH=win32-x86
  
  set EPICSINCLUDE=%EPICS_BASE%/include
  set QTCONTROLS_LIBS=X:/Qt/caqtdm_project/caQtDM_QtControls 
  set CAQTDM_COLLECT=X:/Qt/caqtdm_project/caQtDM_Binaries
  
  set JOM=X:\qt\jom
  set QTBASE=%QTCONTROLS_LIBS%
  
  set QTDM_LIBINSTALL=X:\Qt\4.8.5\lib
  set QTDM_BININSTALL=X:\qt\4.8.5\bin
  set WIXHOME=C:\Program Files (x86)\WiX Toolset v3.8\bin
  set QMAKESPEC=%QTHOME%\mkspecs\win32-msvc2010
  
GOTO PRINTOUT
REM =============================================================================================
REM SELECT2
REM =============================================================================================

:SELECT2 
 
  call "C:\Program files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
 
  set QTHOME=X:/qt/5.4.1_64bit/qtbase
  
  set QWTHOME=X:/qt/qwt-6.1.2_64bit
  set QWTINCLUDE=%QWTHOME%/src
  set QWTLIB=%QWTHOME%/lib
  
  
  set EPICS_BASE=X:/epics/Package/base
  set EPICS_HOST_ARCH=windows-x64

  set EPICSINCLUDE=%EPICS_BASE%/include
  set QTCONTROLS_LIBS=X:/Qt/caqtdm_project/caQtDM_QtControls_64Bit
  set CAQTDM_COLLECT=X:/Qt/caqtdm_project/caQtDM_Binaries_64Bit
  set JOM=X:\qt\jom
 
  set QTBASE=%QTCONTROLS_LIBS%
  
  set WIXHOME=C:\Program Files (x86)\WiX Toolset v3.8\bin
  set QMAKESPEC=%QTHOME%\mkspecs\win32-msvc2013
GOTO PRINTOUT

REM =============================================================================================
REM SELECT3
REM =============================================================================================

:SELECT3 
 
  call "C:\Program files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
 
  set QTHOME=X:/qt/5.4.1_32bit/qtbase
  
  set QWTHOME=X:/qt/qwt-6.1.2_32bit
  set QWTINCLUDE=%QWTHOME%/src
  set QWTLIB=%QWTHOME%/lib
  
  
  set EPICS_BASE=X:/epics/Package/base
  set EPICS_HOST_ARCH=win32-x86

  set EPICSINCLUDE=%EPICS_BASE%/include
  set QTCONTROLS_LIBS=X:/Qt/caqtdm_project/caQtDM_QtControls_32Bit
  set CAQTDM_COLLECT=X:/Qt/caqtdm_project/caQtDM_Binaries_32Bit
  set JOM=X:\qt\jom
 
  set QTBASE=%QTCONTROLS_LIBS%
  
  set WIXHOME=C:\Program Files (x86)\WiX Toolset v3.8\bin
  set QMAKESPEC=%QTHOME%\mkspecs\win32-msvc2013
GOTO PRINTOUT


:SELECTA
 set CAQTDM_GENERAL_COMPILATION=1
GOTO :eof


:PRINTOUT


echo =============================================================================================
echo in order to build this package you will eventually have to redefine following variables in 
echo this file, they are taken from your environment if they exist, otherwise define them yourself:
echo.
echo for building:
echo.
echo QTHOME               now defined as %QTHOME%		for locating Qt
echo QWTHOME              now defined as %QWTHOME%		for locating qwt
echo QWTINCLUDE           now defined as %QWTINCLUDE%		for locating the include files of qwt
echo QWTLIB               now defined as %QWTLIB%		for locating the libraries of qwt
echo EPICS_BASE           now defined as %EPICS_BASE%		for locating epics 
echo EPICSINCLUDE         now defined as %EPICSINCLUDE%		for locating epics include files
echo EPICSLIB             now defined as %EPICSLIB%		for locating epics libraries
echo QTBASE               now defined as %QTBASE%		for building the package locally, pointing to caQtDM_Binaries
echo QTDM_RPATH           now defined as %QTDM_RPATH%		for runtime search path 
echo CAQTDM_COLLECT       now defined as %CAQTDM_COLLECT%	for target path compile 
echo.
echo for install:
echo.
echo EPICSEXTENSIONS      	now defined as %EPICSEXTENSIONS%	for locating epics extensions
echo QTDM_LIBINSTALL      	now defined as %QTDM_LIBINSTALL%	for libraries install 
echo QTDM_BININSTALL      	now defined as %QTDM_BININSTALL%	for binaries install
echo WIXHOME      		now defined as %WIXHOME%		for package generation 
echo ============================================================================================
 




