CAQTDM_PLUGINNAME = monitors
CAQTDM_INSTALL_LIB = qtcontrols_monitors_plugin
include(../Common/plugins.pri)

SOURCES	+= qtcontrols_monitors_plugin.cpp
HEADERS	+= qtcontrols_monitors_plugin.h ../Common/designerPluginTexts.h
RESOURCES += ../Common/qtcontrolsplugin.qrc
TARGET = qtcontrols_monitors_plugin

android {
   INCLUDEPATH += $(ANDROIDFUNCTIONSINCLUDE)
}
