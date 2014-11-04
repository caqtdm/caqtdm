include(./plugins.pri)

SOURCES	+= qtcontrols_controllers_plugin.cpp
HEADERS	+= qtcontrols_controllers_plugin.h
RESOURCES += qtcontrolsplugin.qrc
TARGET = qtcontrols_controllers_plugin
QMAKE_CLEAN += libqtcontrols_controllers_plugin*

!ios {
  unix:!macx  {
   QMAKE_POST_LINK = cp libqtcontrols_controllers_plugin.so $(QTBASE)/designer
  }
  macx: {
     QMAKE_POST_LINK = cp libqtcontrols_controllers_plugin.dylib $(QTBASE)/designer
  }
}

ios {
  QMAKE_POST_LINK = cp libqtcontrols_controllers_plugin.a ../../caQtDM_Binaries/
}

win32 {
ReleaseBuild {
   QMAKE_POST_LINK = copy /Y .\release\qtcontrols_controllers_plugin.dll ..\..\caQtDM_Binaries\designer
 }
}



