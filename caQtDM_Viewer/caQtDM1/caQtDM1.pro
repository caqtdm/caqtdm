include (../caQtDM.pri)

TARGET = caQtDM
TEMPLATE = app
DESTDIR = $(CAQTDM_COLLECT)
# static build
ios {
   CONFIG += staticlib
   LIBS += ../$(CAQTDM_COLLECT)/libcaQtDM_Lib.a
   LIBS += ../$(CAQTDM_COLLECT)/libqtcontrols.a
   LIBS += $$(QWTHOME)/lib/libqwt.a
   LIBS += ../$(CAQTDM_COLLECT)/designer/libqtcontrols_controllers_plugin.a
   LIBS += ../$(CAQTDM_COLLECT)/designer/libqtcontrols_monitors_plugin.a
   LIBS += ../$(CAQTDM_COLLECT)/designer/libqtcontrols_graphics_plugin.a
   LIBS += $$(EPICSLIB)/libca.a
   LIBS += $$(EPICSLIB)/libCom.a
   QMAKE_INFO_PLIST = ../src/IOS/Info.plist
   ICON = ../src/caQtDM.icns
   APP_ICON.files = ../../caQtDM_Viewer/src/caQtDM.png
   APP_ICON.files += ../../caQtDM_Viewer/src/caQtDM-60@2x.png
   APP_ICON.files += ../../caQtDM_Viewer/src/caQtDM-72.png
   APP_ICON.files += ../../caQtDM_Viewer/src/caQtDM-76.png
   APP_ICON.files += ../../caQtDM_Viewer/src/caQtDM-76@2x.png
   APP1_ICON.files = ../../caQtDM_Viewer/src/caQtDM.icns
   APP_XML_FILES.files = ../../caQtDM_Viewer/caQtDM_IOS_Config.xml
   StartScreen.files += ../../caQtDM_Viewer/src/StartScreen-Landscape.png
   APP-FONTS.files = ../../caQtDM_Viewer/lucida-sans-typewriter.ttf
   APP-FONTS.path = fonts
   QMAKE_BUNDLE_DATA += APP_XML_FILES APP_ICON APP1_ICON StartScreen APP-FONTS
   QMAKE_CFLAGS += -gdwarf-2
   QMAKE_CXXFLAGS += -gdwarf-2
}

!ios {

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
   cplugins.path = Contents/PlugIns/designer
   cplugins.files = $(CAQTDM_COLLECT)/designer/libqtcontrols_controllers_plugin.dylib
   gplugins.path = Contents/Plugins/designer
   gplugins.files = $(CAQTDM_COLLECT)/designer/libqtcontrols_graphics_plugin.dylib
   mplugins.path = Contents/Plugins/designer
   mplugins.files = $(CAQTDM_COLLECT)/designer/libqtcontrols_monitors_plugin.dylib
   QMAKE_BUNDLE_DATA += cplugins gplugins mplugins
   calib.path = Contents/Frameworks
   calib.files = $$(EPICS_BASE)/lib/darwin-x86/libca.3.14.12.dylib
   comlib.path = Contents/Frameworks
   comlib.files = $$(EPICS_BASE)/lib/darwin-x86/libCom.3.14.12.dylib
   QMAKE_BUNDLE_DATA += calib comlib
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

