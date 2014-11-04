include(./plugins.pri)

SOURCES	+= qtcontrols_graphics_plugin.cpp
HEADERS	+= qtcontrols_graphics_plugin.h
RESOURCES += qtcontrolsplugin.qrc
TARGET = qtcontrols_graphics_plugin
QMAKE_CLEAN += libqtcontrols_graphics_plugin*

!ios {
   unix:!macx {
      QMAKE_POST_LINK = cp libqtcontrols_graphics_plugin.so $(QTBASE)/designer
   }
   macx: {
      QMAKE_POST_LINK = cp libqtcontrols_graphics_plugin.dylib $(QTBASE)/designer
   }
}

ios {
    QMAKE_POST_LINK = cp libqtcontrols_graphics_plugin.a ../../caQtDM_Binaries/
}

win32 {
   ReleaseBuild {
     QMAKE_POST_LINK = copy /Y .\release\qtcontrols_graphics_plugin.dll ..\..\caQtDM_Binaries\designer
   }
}

