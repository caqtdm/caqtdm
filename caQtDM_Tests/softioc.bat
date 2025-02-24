@echo on
set PATH=D:\epics\Package\base\bin\windows-x64;%PATH%
cd D:\qt\caqtdm_project\caQtDM_Tests
softioc.exe -D D:\epics\Package\base\dbd\softioc.dbd st.cmd
cd ..\..

pause