CAQTDM_PLUGINNAME = monitors
include(../Common/plugins.pri)

SOURCES	+= qtcontrols_monitors_plugin.cpp
HEADERS	+= qtcontrols_monitors_plugin.h designerPluginTexts.h
RESOURCES += ../Common/qtcontrolsplugin.qrc
TARGET = qtcontrols_monitors_plugin

android {
   INCLUDEPATH += $(ANDROIDFUNCTIONSINCLUDE)
}
