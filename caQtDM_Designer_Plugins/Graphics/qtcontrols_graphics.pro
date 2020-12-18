CAQTDM_PLUGINNAME = graphics
CAQTDM_INSTALL_LIB = qtcontrols_graphics_plugin
include(../Common/plugins.pri)

SOURCES	+= qtcontrols_graphics_plugin.cpp
HEADERS	+= qtcontrols_graphics_plugin.h ../Common/designerPluginTexts.h
RESOURCES += ../Common/qtcontrolsplugin.qrc
TARGET = qtcontrols_graphics_plugin

android {
   INCLUDEPATH += $(ANDROIDFUNCTIONSINCLUDE)
}

