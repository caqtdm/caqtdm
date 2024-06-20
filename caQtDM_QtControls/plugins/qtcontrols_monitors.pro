include(./plugins.pri)

SOURCES	+= qtcontrols_monitors_plugin.cpp
HEADERS	+= qtcontrols_monitors_plugin.h designerPluginTexts.h
!MOBILE{
RESOURCES += qtcontrolsplugin.qrc
}
TARGET = qtcontrols_monitors_plugin

android {
   INCLUDEPATH += $(ANDROIDFUNCTIONSINCLUDE)
}
