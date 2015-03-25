include(qtdefs.pri)

android {
include(../../qwt-6.1.1_android/qwt.prf)
}

!ios {
  include($$(QWTHOME)/qwt.prf)
}

unix {
  TEMPLATE = subdirs
  SUBDIRS += caQtDM1
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
	DESTDIR = ../$(CAQTDM_COLLECT)
	OTHER_FILES += ./src/caQtDM.ico
	DEFINES +=_CRT_SECURE_NO_WARNINGS
	RESOURCES += ./src/caQtDM.qrc
	RC_FILE = ./src/caQtDM.rc
	
	win32-msvc* {
		DebugBuild {
			CONFIG += console
                        DESTDIR = $$(CAQTDM_COLLECT)/debug
			EPICS_LIBS=$${EPICS_BASE}/lib/$$(EPICS_HOST_ARCH)
			OBJECTS_DIR = debug/obj
			LIBS += $$(CAQTDM_COLLECT)/debug/caQtDM_Lib.lib
			LIBS += $$(QWTLIB)/qwtd.lib
			LIBS += $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/ca.lib
			LIBS += $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/COM.lib
			LIBS += $$(CAQTDM_COLLECT)/debug/qtcontrols.lib


		}
		ReleaseBuild {
                        DESTDIR = $$(CAQTDM_COLLECT)
			EPICS_LIBS=$$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)
			OBJECTS_DIR = release/obj
			LIBS += $$(CAQTDM_COLLECT)/caQtDM_Lib.lib
			LIBS += $$(QWTLIB)/qwt.lib
			LIBS += $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/ca.lib
			LIBS += $$(EPICS_BASE)/lib/$$(EPICS_HOST_ARCH)/COM.lib
			LIBS += $$(CAQTDM_COLLECT)/qtcontrols.lib
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
