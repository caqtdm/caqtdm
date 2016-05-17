@echo off
REM ****************************************
REM 
REM Startscript for test display
REM 
REM ****************************************
set AUTO_ADDR_LIST=NO
set EPICS_CA_ADDR_LIST=129.129.130.255

caQtDM -macro DEVICE=GM84 TST-CAQTDM-MAIN.ui
