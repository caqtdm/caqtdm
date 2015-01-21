echo =============================================================================================
echo Select Build Environment
echo 1) QT 4.8.5 QWT6.0.1  32 Bit VS2010  
echo 2) QT 5.4.0 QWT6.1.1  64 Bit VS2013  
echo 3) QT 5.4.0 QWT6.1.1  32 Bit VS2013  
set /P SELCTION=Select: 
echo =============================================================================================
 
IF %SELCTION%==1 GOTO SELECT1
IF %SELCTION%==2 GOTO SELECT2
IF %SELCTION%==3 GOTO SELECT3


REM =============================================================================================
REM SELECT1
REM =============================================================================================

:SELECT1 
 
  call "C:\Program files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
 
  IF "%QTHOME%"==""          set QTHOME=X:/qt/4.8.5
  IF "%QWTHOME%"==""         set QWTHOME=X:/qt/qwt-6.0.1
  IF "%QWTINCLUDE%"==""      set QWTINCLUDE=%QWTHOME%/src
  IF "%QWTLIB%"==""          set QWTLIB=%QWTHOME%/lib
  
  
  IF "%EPICS_BASE%"==""      set EPICS_BASE=X:/epics/base-3.14.12.4
  IF "%EPICS_HOST_ARCH%"=="" set EPICS_HOST_ARCH=win32-x86
  
  IF "%EPICSINCLUDE%"==""    set EPICSINCLUDE=%EPICS_BASE%/include
  IF "%QTCONTROLS_LIBS%"=="" set QTCONTROLS_LIBS=X:/Qt/caqtdm_project/caQtDM_QtControls
  IF "%CAQTDM_COLLECT%"==""  set CAQTDM_COLLECT=X:/Qt/caqtdm_project/caQtDM_Binaries
  IF "%JOM%"==""             set JOM=X:\qt\jom
  
  set QTBASE=%QTCONTROLS_LIBS%
  
  IF "%QTDM_LIBINSTALL%"=="" 		set QTDM_LIBINSTALL=X:\Qt\4.8.5\lib
  IF "%QTDM_BININSTALL%"=="" 		set QTDM_BININSTALL=X:\qt\4.8.5\bin
  IF "%WIXHOME%"=="" 			set WIXHOME=C:\Program Files (x86)\WiX Toolset v3.8\bin
  set QMAKESPEC=%QTHOME%\mkspecs\win32-msvc2010
  
GOTO PRINTOUT
REM =============================================================================================
REM SELECT2
REM =============================================================================================

:SELECT2 
 
  call "C:\Program files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
 
  IF "%QTHOME%"==""          set QTHOME=X:/qt/5.4.0_64bit/qtbase
  
  IF "%QWTHOME%"==""         set QWTHOME=X:/qt/qwt-6.1.1_5_64bit
  IF "%QWTINCLUDE%"==""      set QWTINCLUDE=%QWTHOME%/src
  IF "%QWTLIB%"==""          set QWTLIB=%QWTHOME%/lib
  
  
  IF "%EPICS_BASE%"==""      set EPICS_BASE=X:/epics/base-3.14.12.4
  IF "%EPICS_HOST_ARCH%"=="" set EPICS_HOST_ARCH=windows-x64

  IF "%EPICSINCLUDE%"==""    set EPICSINCLUDE=%EPICS_BASE%/include
  IF "%QTCONTROLS_LIBS%"=="" set QTCONTROLS_LIBS=X:/Qt/caqtdm_project/caQtDM_QtControls
  IF "%CAQTDM_COLLECT%"==""  set CAQTDM_COLLECT=X:/Qt/caqtdm_project/caQtDM_Binaries_64Bit
  IF "%JOM%"==""             set JOM=X:\qt\jom
 
  set QTBASE=%QTCONTROLS_LIBS%
  
  IF "%WIXHOME%"=="" 			set WIXHOME=C:\Program Files (x86)\WiX Toolset v3.8\bin
  set QMAKESPEC=%QTHOME%\mkspecs\win32-msvc2013
GOTO PRINTOUT

REM =============================================================================================
REM SELECT3
REM =============================================================================================

:SELECT3 
 
  call "C:\Program files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
 
  IF "%QTHOME%"==""          set QTHOME=X:/qt/5.4.0_32bit/qtbase
  
  IF "%QWTHOME%"==""         set QWTHOME=X:/qt/qwt-6.1.1_5_32bit
  IF "%QWTINCLUDE%"==""      set QWTINCLUDE=%QWTHOME%/src
  IF "%QWTLIB%"==""          set QWTLIB=%QWTHOME%/lib
  
  
  IF "%EPICS_BASE%"==""      set EPICS_BASE=X:/epics/base-3.14.12.4
  IF "%EPICS_HOST_ARCH%"=="" set EPICS_HOST_ARCH=win32-x86

  IF "%EPICSINCLUDE%"==""    set EPICSINCLUDE=%EPICS_BASE%/include
  IF "%QTCONTROLS_LIBS%"=="" set QTCONTROLS_LIBS=X:/Qt/caqtdm_project/caQtDM_QtControls
  IF "%CAQTDM_COLLECT%"==""  set CAQTDM_COLLECT=X:/Qt/caqtdm_project/caQtDM_Binaries_32Bit
  IF "%JOM%"==""             set JOM=X:\qt\jom
 
  set QTBASE=%QTCONTROLS_LIBS%
  
  IF "%WIXHOME%"=="" 			set WIXHOME=C:\Program Files (x86)\WiX Toolset v3.8\bin
  set QMAKESPEC=%QTHOME%\mkspecs\win32-msvc2013
GOTO PRINTOUT







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
 




