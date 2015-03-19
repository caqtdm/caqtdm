include (../caQtDM.pri)

TARGET = caQtDM
TEMPLATE = app
DESTDIR = $(CAQTDM_COLLECT)
android {
DESTDIR =/Users/mezger/Documents/Entwicklung/qt/caqtdm_project/caQtDM_Binaries_SIM_ANDROID
ANDROID_PACKAGE_SOURCE_DIR = /Users/mezger/Documents/Entwicklung/qt/caqtdm_project/caQtDM_Viewer/caQtDM1/caQtDM_Viewer/caQtDM1/android
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
   APP_ICON.files = $$PWD/../src/caQtDM.png
   APP_ICON.files += $$PWD/../src/caQtDM-60@2x.png
   APP_ICON.files += $$PWD/../src/caQtDM-72.png
   APP_ICON.files += $$PWD/../src/caQtDM-76.png
   APP_ICON.files += $$PWD/../src/caQtDM-76@2x.png

   APP1_ICON.files = $$PWD/../src/caQtDM.icns
   APP_XML_FILES.files = $$PWD/../caQtDM_IOS_Config.xml
   StartScreen.files += $$PWD/../src/StartScreen-Landscape.png
   StartScreen.files += $$PWD/../src/StartScreen-568h@2x.png
   APP-FONTS.files = $$PWD/../lucida-sans-typewriter.ttf
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

   ICON = ../src/caQtDM.icns$$PWD/../src/caQtDM.png
   APP_ICON.files += $$PWD/../src/caqtdm762x.png
   APP_ICON.path = /res/drawable

   APP1_ICON.files = $$PWD/../src/caQtDM.icns
   APP_XML_FILES.files = $$PWD/../caQtDM_IOS_Config.xml
   StartScreen.files += $$PWD/../src/startscreenlandscape.png
   StartScreen.path = /res/drawable
   #StartScreen.files += $$PWD/../src/StartScreen-568h@2x.png
   APP-FONTS.files = $$PWD/../lucida-sans-typewriter.ttf
   APP-FONTS.path = /fonts
   QMAKE_BUNDLE_DATA += APP_XML_FILES APP_ICON APP1_ICON StartScreen APP-FONTS

   deployment.files += $$PWD/../caQtDM_IOS_Config.xml
   deployment.path = /assets
   INSTALLS += deployment APP-FONTS APP_ICON StartScreen
}

!ios {
!android {
  LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib
  LIBS += -L$(CAQTDM_COLLECT) -L$(CAQTDM_COLLECT)/designer

unix:!macx {

  }
  macx: {
   QMAKE_INFO_PLIST = ../src/Mac/Info.plist
   APP-FONTS.files = ../../caQtDM_Viewer/lucida-sans-typewriter.ttf
   APP-FONTS.path = fonts
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

OBJECTS_DIR = ../obj1
DEFINES += SUPPORT=\\\"EPICS\\\"

MOC_DIR = ../moc
VPATH += ../src
INCLUDEPATH += ../src
UI_DIR += ../src

INCLUDEPATH += $(QWTINCLUDE)
INCLUDEPATH += ../../caQtDM_QtControls/src
INCLUDEPATH += ../../caQtDM_Lib/src

RESOURCES += ../src/caQtDM.qrc
RC_FILE = ../src/caQtDM.rc
android {
DISTFILES += /Users/mezger/Documents/Entwicklung/qt/caqtdm_project/caQtDM_Viewer/caQtDM1/caQtDM_Viewer/caQtDM1/android/AndroidManifest.xml \
    caQtDM_Viewer/caQtDM1/android/gradle/wrapper/gradle-wrapper.jar \
    caQtDM_Viewer/caQtDM1/android/res/values/libs.xml \
    caQtDM_Viewer/caQtDM1/android/build.gradle \
    caQtDM_Viewer/caQtDM1/android/gradle/wrapper/gradle-wrapper.properties \
    caQtDM_Viewer/caQtDM1/android/gradlew \
    caQtDM_Viewer/caQtDM1/android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/caQtDM_Viewer/caQtDM1/android
}

