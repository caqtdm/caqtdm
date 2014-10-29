include (../caQtDM.pri)

TARGET = caQtDM
TEMPLATE = app

# static build
ios {
   CONFIG += staticlib
   LIBS += ../../caQtDM_Binaries/libcaQtDM_Lib.a
   LIBS += ../../caQtDM_Binaries/libqtcontrols.a
   LIBS += $$(QWTHOME)/lib/libqwt.a
   LIBS += ../../caQtDM_Binaries/libqtcontrols_controllers_plugin.a
   LIBS += ../../caQtDM_Binaries/libqtcontrols_monitors_plugin.a
   LIBS += ../../caQtDM_Binaries/libqtcontrols_graphics_plugin.a
   LIBS += $$(EPICSLIB)/libca.a
   LIBS += $$(EPICSLIB)/libCom.a
   QMAKE_INFO_PLIST = ../src/IOS/Info.plist
   ICON = ../src/caQtDM.icns
   APP_ICON.files = ../../caQtDM_Viewer/src/caQtDM.png
   APP_ICON.files += ../../caQtDM_Viewer/src/caQtDM-60@2x.png
   APP_ICON.files += ../../caQtDM_Viewer/src/caQtDM-76.png
   APP_ICON.files += ../../caQtDM_Viewer/src/caQtDM-76@2x.png
   APP1_ICON.files = ../../caQtDM_Viewer/src/caQtDM.icns
   APP_XML_FILES.files = ../../caQtDM_Viewer/caQtDM_IOS_Config.xml
   StartScreen.files += ../../caQtDM_Viewer/src/StartScreen-Landscape.png
   APP-FONTS.files = ../../caQtDM_Viewer/lucida-sans-typewriter.ttf
   APP-FONTS.path = fonts
   QMAKE_BUNDLE_DATA += APP_XML_FILES APP_ICON APP1_ICON StartScreen APP-FONTS
}

!ios {

  LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lcaQtDM_Lib

  unix:!macx {
    QMAKE_POST_LINK = cp ../caQtDM $(QTBASE)
  }
  macx: {
   QMAKE_INFO_PLIST = ../src/Mac/Info.plist
   APP-FONTS.files = ../../caQtDM_Viewer/lucida-sans-typewriter.ttf
   APP-FONTS.path = fonts
   QMAKE_BUNDLE_DATA += APP-FONTS
   QMAKE_POST_LINK += cp -R ../caQtDM.app ../../caQtDM_Binaries/ &&
   QMAKE_POST_LINK += cp ../../caQtDM_Binaries/libcaQtDM_Lib.dylib ../../caQtDM_Binaries/caQtDM.app/Contents/Frameworks &&
   QMAKE_POST_LINK += cp ../../caQtDM_Binaries/libqtcontrols.dylib ../../caQtDM_Binaries/caQtDM.app/Contents/Frameworks
  }
}



OBJECTS_DIR = ../obj1
DEFINES += SUPPORT=\\\"EPICS\\\"

MOC_DIR = ../moc
VPATH += ../src
INCLUDEPATH += ../src
DESTDIR = ../
UI_DIR += ../src

INCLUDEPATH += $(QWTINCLUDE)
INCLUDEPATH += ../../caQtDM_QtControls/src
INCLUDEPATH += ../../caQtDM_Lib/src

RESOURCES += ../src/caQtDM.qrc
RC_FILE = ../src/caQtDM.rc

