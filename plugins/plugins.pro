include(../../caQtDM/qtdefs.pri)

DEFINES += QT_NO_DEBUG_OUTPUT
CONFIG += release

CONFIG += designer plugin 
CONFIG	+= qt thread warn_on

TEMPLATE = lib

win32 {
     INCLUDEPATH += $${QWTHOME}/src
     
     win32-g++ {
	     LIBS += $${QWTHOME}/lib/libqwt.a
	     LIBS += $${QTCONTROLS_LIBS}/libqtcontrols.a
     }
     win32-msvc* {
	     DebugBuild {
	     	     INCLUDEPATH += $${QWTHOME}/include
		     LIBS += $${QWTHOME}/lib/qwtd.lib
		     LIBS += $${QTCONTROLS}/debug/qtcontrols.lib
	     }

	     ReleaseBuild {
	             INCLUDEPATH += $${QWTHOME}/include
		     LIBS += $${QWTHOME}/lib/qwt.lib
		     LIBS += $${QTCONTROLS}/release/qtcontrols.lib
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
QMAKE_POST_LINK = cp libqtcontrols_monitors_plugin.so $(QTBASE)/designer
  win32 {
    QMAKE_POST_LINK = copy /Y .\release\qtcontrols_monitors_plugin.dll $${PLUGINS_LOCATION}
  }
}

configB {
SOURCES	+= qtcontrols_controllers_plugin.cpp
HEADERS	+= qtcontrols_controllers_plugin.h
RESOURCES += qtcontrolsplugin.qrc
TARGET = qtcontrols_controllers_plugin
QMAKE_CLEAN += libqtcontrols_controllers_plugin*
QMAKE_POST_LINK = cp libqtcontrols_controllers_plugin.so $(QTBASE)/designer
  win32 {
    QMAKE_POST_LINK = copy /Y .\release\qtcontrols_controllers_plugin.dll $${PLUGINS_LOCATION}
  }
}

configC {
SOURCES	+= qtcontrols_graphics_plugin.cpp
HEADERS	+= qtcontrols_graphics_plugin.h
RESOURCES += qtcontrolsplugin.qrc
TARGET = qtcontrols_graphics_plugin
QMAKE_CLEAN += libqtcontrols_graphics_plugin*
QMAKE_POST_LINK = cp libqtcontrols_graphics_plugin.so $(QTBASE)/designer
  win32 {
    QMAKE_POST_LINK = copy /Y .\release\qtcontrols_graphics_plugin.dll $${PLUGINS_LOCATION}
  }
}
