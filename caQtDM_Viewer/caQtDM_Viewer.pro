include(qtdefs.pri)

contains(QT_VER_MAJ, 4) {
   QT     += core gui svg network
   CONFIG += qt warn_on thread uitools
}
contains(QT_VER_MAJ, 5) {
   QT     += core gui svg uitools  printsupport network
   CONFIG += qt warn_on thread widgets
   DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000 
} 

android {
include(../../qwt-6.1.1_android/qwt.prf)
}

!ios {
  include($$(QWTHOME)/qwt.prf)
}

TARGET = caQtDM
TEMPLATE = app
DESTDIR = $(CAQTDM_COLLECT)

OBJECTS_DIR = ./obj
DEFINES += SUPPORT=\\\"EPICS\\\"

MOC_DIR = ./moc
VPATH += ./src
INCLUDEPATH += ./src
UI_DIR += ./src

INCLUDEPATH += $(QWTINCLUDE)
INCLUDEPATH += ../caQtDM_QtControls/src
INCLUDEPATH += ../caQtDM_Lib/src
INCLUDEPATH += ../caQtDM_Lib/caQtDM_Plugins

RESOURCES += ./src/caQtDM.qrc
RC_FILE = ./src/caQtDM.rc

SOURCES +=\
    caQtDM.cpp \
    fileopenwindow.cpp \
    messagebox.cpp \
    configDialog.cpp

HEADERS  +=  \
    messagebox.h \
    fileopenwindow.h \
    configDialog.h

FORMS += main.ui


# building parameters
unix:{
	DEFINES += BUILDTIME=\\\"$$system(date '+%H:%M')\\\"
	DEFINES += BUILDDATE=\\\"$$system(date '+%d-%m-%Y')\\\"
}
win32{
	DEFINES += BUILDTIME=\\\"\\\"
	DEFINES += BUILDDATE=\\\"\\\"
	DEFINES += SUPPORT=\\\"\\\"
}
DEFINES += BUILDVERSION=\\\"$${CAQTDM_VERSION}\\\"
DEFINES += BUILDARCH=\\\"$$(QMAKESPEC)\\\"


unix: {
  CONFIG += x11
}

android {
DESTDIR =/Users/mezger/Documents/Entwicklung/qt/caqtdm_project/caQtDM_Binaries_SIM_ANDROID
DISTFILES += /Users/mezger/Documents/Entwicklung/qt/caqtdm_project/caQtDM_Viewer/caQtDM1/caQtDM_Viewer/caQtDM1/android/AndroidManifest.xml \
    caQtDM_Viewer/caQtDM1/android/gradle/wrapper/gradle-wrapper.jar \
    caQtDM_Viewer/caQtDM1/android/res/values/libs.xml \
    caQtDM_Viewer/caQtDM1/android/build.gradle \
    caQtDM_Viewer/caQtDM1/android/gradle/wrapper/gradle-wrapper.properties \
    caQtDM_Viewer/caQtDM1/android/gradlew \
    caQtDM_Viewer/caQtDM1/android/gradlew.bat
ANDROID_PACKAGE_SOURCE_DIR = /Users/mezger/Documents/Entwicklung/qt/caqtdm_project/caQtDM_Viewer/caQtDM1/caQtDM_Viewer/caQtDM1/android
#ANDROID_PACKAGE_SOURCE_DIR = $$PWD/caQtDM_Viewer/caQtDM1/android
}

# static build
ios {
   CONFIG += staticlib
   LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.a
   LIBS += $(CAQTDM_COLLECT)/libqtcontrols.a
   LIBS += $$(QWTHOME)/lib/libqwt.a
   LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_controllers_plugin.a
   LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_monitors_plugin.a
   LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_graphics_plugin.a
   LIBS += $$(EPICSLIB)/libca.a
   LIBS += $$(EPICSLIB)/libCom.a
   QMAKE_INFO_PLIST = ../src/IOS/Info.plist
   ICON = ../src/caQtDM.icns
   APP_ICON.files = $$PWD/src/caQtDM.png
   APP_ICON.files += $$PWD/src/caQtDM-57.png
   APP_ICON.files += $$PWD/src/caQtDM-60@2x.png
   APP_ICON.files += $$PWD/src/caQtDM-72.png
   APP_ICON.files += $$PWD/src/caQtDM-76.png
   APP_ICON.files += $$PWD/src/caQtDM-76@2x.png

   APP1_ICON.files = $$PWD/src/caQtDM.icns
   APP_XML_FILES.files = $$PWD/caQtDM_IOS_Config.xml
   StartScreen.files += $$PWD/src/StartScreen-Landscape.png
   StartScreen.files += $$PWD/src/StartScreen-568h@2x.png
   APP-FONTS.files = $$PWD/lucida-sans-typewriter.ttf
   APP-FONTS.path = fonts
   QMAKE_BUNDLE_DATA += APP_XML_FILES APP_ICON APP1_ICON StartScreen APP-FONTS
   QMAKE_CFLAGS += -gdwarf-2
   QMAKE_CXXFLAGS += -gdwarf-2
}

android {
   CONFIG += staticlib
   LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_controllers_plugin.a
   LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_monitors_plugin.a
   LIBS += $(CAQTDM_COLLECT)/designer/libqtcontrols_graphics_plugin.a
   LIBS += $(CAQTDM_COLLECT)/libcaQtDM_Lib.a
   LIBS += $(CAQTDM_COLLECT)/libqtcontrols.a
   LIBS += $$(QWTHOME)/lib/libqwt.a
   LIBS += $$(EPICSLIB)/libca.a
   LIBS += $$(EPICSLIB)/libCom.a
   LOCAL_LDLIBS += -llog

   ICON = ../src/caQtDM.icns$$PWD/src/caQtDM.png
   APP_ICON.files += $$PWD/src/caqtdm762x.png
   APP_ICON.path = /res/drawable

   APP1_ICON.files = $$PWD/src/caQtDM.icns
   APP_XML_FILES.files = $$PWD/caQtDM_IOS_Config.xml
   StartScreen.files += $$PWD/src/startscreenlandscape.png
   StartScreen.path = /res/drawable
   #StartScreen.files += $$PWD/src/StartScreen-568h@2x.png
   APP-FONTS.files = $$PWD/lucida-sans-typewriter.ttf
   APP-FONTS.path = /fonts
   QMAKE_BUNDLE_DATA += APP_XML_FILES APP_ICON APP1_ICON StartScreen APP-FONTS

   deployment.files += $$PWD/caQtDM_IOS_Config.xml
   deployment.path = /assets
   INSTALLS += deployment APP-FONTS APP_ICON StartScreen
}

!ios {
!android {
  LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
  LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lqtcontrols
  LIBS += -L$(CAQTDM_COLLECT) -L$(CAQTDM_COLLECT)/designer

unix:!macx {

  }
  macx: {

   QMAKE_INFO_PLIST = ../src/Mac/Info.plist
   APP-FONTS.files = ../caQtDM_Viewer/lucida-sans-typewriter.ttf
   APP-FONTS.path = Contents/Resources/fonts
   QMAKE_BUNDLE_DATA += APP-FONTS
   CONFIG += app_bundle
   ICON = ../src/caQtDM.icns
   plugins.path = Contents/PlugIns/designer
   plugins.files = $(CAQTDM_COLLECT)/designer/libqtcontrols_controllers_plugin.dylib
   plugins.files += $(CAQTDM_COLLECT)/designer/libqtcontrols_graphics_plugin.dylib
   plugins.files += $(CAQTDM_COLLECT)/designer/libqtcontrols_monitors_plugin.dylib
   caqtdmlibs.path = Contents/Frameworks/
   caqtdmlibs.files = $(CAQTDM_COLLECT)/libcaQtDM_Lib.dylib $(CAQTDM_COLLECT)/libqtcontrols.dylib
   caqtdmlibs.files += $(CAQTDM_COLLECT)/libqtcontrols.dylib
   QMAKE_BUNDLE_DATA += plugins caqtdmlibs

   calib.path = Contents/Frameworks
   calib.files = $$(EPICS_BASE)/lib/darwin-x86/libca.3.14.12.dylib
   comlib.path = Contents/Frameworks
   comlib.files = $$(EPICS_BASE)/lib/darwin-x86/libCom.3.14.12.dylib
   QMAKE_BUNDLE_DATA += calib comlib
  }
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
			QMAKE_CXXFLAGS += /Z7
			QMAKE_CFLAGS   += /Z7
			QMAKE_LFLAGS   += /DEBUG /OPT:REF /OPT:ICF
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
