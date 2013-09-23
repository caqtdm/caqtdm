include(qtdefs.pri)

unix {
  TEMPLATE = subdirs
  SUBDIRS = parser caQtDM1 
  exists("/home/ACS/Control/Lib/libDEV.so") {
         SUBDIRS += caQtDM2
  }
}

win32 {
	TARGET = caQtDM
	TEMPLATE = app
	include (./caQtDM.pri)
	MOC_DIR = moc
	VPATH += ./src
	UI_DIR += ./
	INCLUDEPATH += .
	INCLUDEPATH += $(QWTHOME)/src
	INCLUDEPATH += $$(EPICS_BASE)/include
	INCLUDEPATH += $$(EPICS_BASE)/include/os/win32
	INCLUDEPATH += ../caQtDM_Lib/src
	INCLUDEPATH += ../caQtDM_QtControls/src
	INCLUDEPATH += $(QWTINCLUDE)
	INCLUDEPATH += $(EPICSINCLUDE)
	
	OTHER_FILES += ./src/caQtDM.ico
	DEFINES +=_CRT_SECURE_NO_WARNINGS
	RESOURCES += ./src/caQtDM.qrc
	RC_FILE = ./src/caQtDM.rc
	
	win32-msvc* {
		DebugBuild {
			CONFIG += console
			DESTDIR = debug
			EPICS_LIBS=$${EPICS_BASE}/lib/win32-x86
			OBJECTS_DIR = debug/obj
			LIBS += ../caQtDM_Lib/debug/caQtDM_Lib.lib
			LIBS += $$(QWTHOME)/lib/qwtd.lib
			LIBS += $$(EPICS_BASE)/lib/win32-x86-debug/ca.lib
			LIBS += $$(EPICS_BASE)/lib/win32-x86-debug/COM.lib
			LIBS += $$(QTCONTROLS_LIBS)/debug/qtcontrols.lib


		}
		ReleaseBuild {
			DESTDIR = release
			EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-debug
			OBJECTS_DIR = release/obj
			LIBS += ../caQtDM_Lib/release/caQtDM_Lib.lib
			LIBS += $$(QWTHOME)/lib/qwt.lib
			LIBS += $$(EPICS_BASE)/lib/win32-x86/ca.lib
			LIBS += $$(EPICS_BASE)/lib/win32-x86/COM.lib
			LIBS += $$(QTCONTROLS_LIBS)/release/qtcontrols.lib
			QMAKE_POST_LINK = $${QMAKE_COPY} .\\release\\caQtDM.exe ..\caQtDM_Binaries


		}
	}

	win32-g++ {
	        EPICS_LIBS=$$(EPICS_BASE)/lib/win32-x86-mingw
		LIBS += $$(QWTLIB)/libqwt.a
		LIBS += $$(QTCONTROLS_LIBS)/release/libqtcontrols.a
		LIBS += $${EPICS_LIBS}/ca.lib
		LIBS += $${EPICS_LIBS}/COM.lib
		LIBS += ../caQtDM_Lib/release/libcaQtDM_Lib.a
		QMAKE_POST_LINK = $${QMAKE_COPY} .\\release\\caQtDM.exe ..\caQtDM_Binaries
	}


}
