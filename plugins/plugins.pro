include(../../caQtDM/qtdefs.pri)

DEFINES += QT_NO_DEBUG_OUTPUT
CONFIG += release

contains(QT_VER_MAJ, 4) {
   CONFIG += designer plugin qt thread warn_on
}

contains(QT_VER_MAJ, 5) {
   CONFIG += plugin qt thread warn_on
   QT += designer widgets
}

TEMPLATE = lib

win32 {
     INCLUDEPATH += $$(QWTHOME)/src
     
     win32-g++ {
	     LIBS += $$(QWTLIB)/lib/libqwt.a
	     LIBS += $$(QTCONTROLS_LIBS)/release/libqtcontrols.a
     }
     win32-msvc* {
	     DebugBuild {
	     	     INCLUDEPATH += $$(QWTHOME)/include
		     LIBS += $$(QWTHOME)/lib/qwtd.lib
		     LIBS += $$(QTCONTROLS_LIBS)/debug/qtcontrols.lib
	     }

	     ReleaseBuild {
	             INCLUDEPATH += $$(QWTHOME)/include
		     LIBS += $$(QWTHOME)/lib/qwt.lib
		     LIBS += $$(QTCONTROLS_LIBS)/release/qtcontrols.lib
	     }
     }

     
     
}
unix {

  INCLUDEPATH += $(QWTINCLUDE)
  LIBS += -L$(QTBASE) -Wl,-rpath,$(QTDM_RPATH) -lqtcontrols
  LIBS += -L$(QWTLIB) -Wl,-rpath,$(QWTLIB) -lqwt

  MOC_DIR = moc
  OBJECTS_DIR = obj
}

INCLUDEPATH += ../src
INCLUDEPATH += .

configA {
SOURCES	+= qtcontrols_monitors_plugin.cpp
HEADERS	+= qtcontrols_monitors_plugin.h
RESOURCES += qtcontrolsplugin.qrc
TARGET = qtcontrols_monitors_plugin
QMAKE_CLEAN += libqtcontrols_monitors_plugin*
unix {
  QMAKE_POST_LINK = cp libqtcontrols_monitors_plugin.so $(QTBASE)/designer
}
  win32 {
   ReleaseBuild {
    QMAKE_POST_LINK = copy /Y .\release\qtcontrols_monitors_plugin.dll ..\..\caQtDM_Binaries\designer
   }
  }
}

configB {
SOURCES	+= qtcontrols_controllers_plugin.cpp
HEADERS	+= qtcontrols_controllers_plugin.h
RESOURCES += qtcontrolsplugin.qrc
TARGET = qtcontrols_controllers_plugin
QMAKE_CLEAN += libqtcontrols_controllers_plugin*
unix {
  QMAKE_POST_LINK = cp libqtcontrols_controllers_plugin.so $(QTBASE)/designer
}
  win32 {
   ReleaseBuild {
    QMAKE_POST_LINK = copy /Y .\release\qtcontrols_controllers_plugin.dll ..\..\caQtDM_Binaries\designer
   }
  }
}

configC {
SOURCES	+= qtcontrols_graphics_plugin.cpp
HEADERS	+= qtcontrols_graphics_plugin.h
RESOURCES += qtcontrolsplugin.qrc
TARGET = qtcontrols_graphics_plugin
QMAKE_CLEAN += libqtcontrols_graphics_plugin*
unix {
  QMAKE_POST_LINK = cp libqtcontrols_graphics_plugin.so $(QTBASE)/designer
}
  win32 {
   ReleaseBuild {
    QMAKE_POST_LINK = copy /Y .\release\qtcontrols_graphics_plugin.dll ..\..\caQtDM_Binaries\designer
   }
  }
}
