include(./plugins.pri)

SOURCES	+= qtcontrols_monitors_plugin.cpp
HEADERS	+= qtcontrols_monitors_plugin.h
RESOURCES += qtcontrolsplugin.qrc
TARGET = qtcontrols_monitors_plugin
QMAKE_CLEAN += libqtcontrols_monitors_plugin*

!ios {
  unix:!macx {
     QMAKE_POST_LINK = cp libqtcontrols_monitors_plugin.so $(QTBASE)/designer
  }
  macx: {
     QMAKE_POST_LINK = cp libqtcontrols_monitors_plugin.dylib $(QTBASE)/designer
  }
}

ios {
  QMAKE_POST_LINK = cp libqtcontrols_monitors_plugin.a ../../caQtDM_Binaries/
}

win32 {
  ReleaseBuild {
    QMAKE_POST_LINK = copy /Y .\release\qtcontrols_monitors_plugin.dll ..\..\caQtDM_Binaries\designer
  }
}

