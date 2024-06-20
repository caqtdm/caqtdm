include(./plugins.pri)

SOURCES	+= qtcontrols_utilities_plugin.cpp
HEADERS	+= qtcontrols_utilities_plugin.h  designerPluginTexts.h
!MOBILE{
RESOURCES += qtcontrolsplugin.qrc
}
TARGET = qtcontrols_utilities_plugin

android {
   INCLUDEPATH += $(ANDROIDFUNCTIONSINCLUDE)
}
